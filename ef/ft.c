#include <ef/ft.h>
#include <ef/memory.h>
#include <ef/strong.h>
#include <ef/file.h>
#include <freetype2/ft2build.h>
#include <fontconfig/fontconfig.h>

#define FONT_GLYPH_MIN 10
#define FONT_GLYPH_MAX 512

__private FT_Library _ftl;

#undef FTERRORS_H_
#define FT_ERRORDEF( e, v, s )  [e] = s,
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       };

__private const char* ftStrError[] = 
#include FT_ERRORS_H

err_t ft_init(ftlib_h* ftl){
	if(ftl == NULL){
		ftl=&_ftl;
	}
	err_t err;
	if( (err=FT_Init_FreeType(ftl)) ){
		dbg_error("freetype2 error %d: %s", err, ftStrError[err]);
		return err;
	}
	return 0;
}

void ft_terminate(ftlib_h ftl){
	if( ftl == NULL ){
		ftl = _ftl;
	}
	FT_Done_FreeType(ftl);
}

void ft_fonts_init(ftlib_h ftl, ftFonts_s* fonts){
	fonts->lib = ftl ? ftl : _ftl;
	fonts->font = NULL;
}

void ft_fonts_free(ftFonts_s* fonts){
	while(fonts->font){
		ft_font_free(fonts, fonts->font);
	}
	fonts->lib = NULL;
}

ftFont_s* ft_fonts_search(ftFonts_s* fonts, const char* path){
	for(ftFont_s* font = fonts->font; font; font = font->next){
		if( !strcmp(path, font->fname) ){
			return font;
		}
	}
	return NULL;
}

ftFont_s* ft_fonts_search_index(ftFonts_s* fonts, unsigned index){
	ftFont_s* font = NULL; 
	for(font = fonts->font; font && index; font = font->next, --index);
	return font;
}

__private void ft_fonts_insert(ftFonts_s* fonts, ftFont_s* font){
	ftFont_s** next = &fonts->font;
	for(; *next; next = &((*next)->next) );
	*next = font;
}

__private void _ft_glyph_free(__unused uint32_t hash, __unused const char* name, void* a){
	ft_glyph_free(a);
}

char* ft_file_search(char const* path){
	if( file_exists((char*)path) ){
		dbg_info("%s => %s", path, path);
		return str_dup(path, 0);
	}
	FcConfig* config = FcInitLoadConfigAndFonts();
	FcPattern* pat = FcNameParse((const FcChar8*)path);
	FcConfigSubstitute(config, pat, FcMatchPattern);
	FcDefaultSubstitute(pat);
	FcResult result;
	FcPattern* font = FcFontMatch(config, pat, &result);
	if (font){
		FcChar8* file = NULL;
		if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch){
			char* ret = str_dup((char*)file, 0);
			dbg_info("%s => %s", path, (char*)file);
			FcPatternDestroy(font);
			FcPatternDestroy(pat);
			return ret;
		}
		FcPatternDestroy(font);
	}
	FcPatternDestroy(pat);
	return NULL;
}

ftFont_s* ft_fonts_load(ftFonts_s* fonts, const char* path){
	ftFont_s* font = NULL;
	if( !(path = ft_file_search(path)) ){
		dbg_error("invalid font file");
		return NULL;
	}

	if( (font=ft_fonts_search(fonts, path)) ){
		dbg_info("font %s already loaded", path);
		return font;
	}
	
	font = mem_new(ftFont_s);
	iassert(font);

	err_t err;
	if( (err=FT_New_Face(fonts->lib, path, 0, &font->face)) ){
		dbg_error("freetype2 newface error %d: %s", err, ftStrError[err]);
		free(font);
		return NULL;
	}
	if( (err=FT_Select_Charmap(font->face,ft_encoding_unicode)) ){
		dbg_error("freetype2 load charmap %d: %s", err, ftStrError[err]);
		FT_Done_Face(font->face);
		free(font);
		return NULL;
	}

	font->fname = path;
	font->next = NULL;
	chash_init(&font->charmap, FONT_GLYPH_MAX, FONT_GLYPH_MIN, hash_fasthash, _ft_glyph_free);
	ft_fonts_insert(fonts, font);
	return font;
}

__private void ft_fonts_remove(ftFonts_s* fonts, ftFont_s* font){
	ftFont_s** next = &fonts->font;
	for(; *next != font; next = &((*next)->next) );
	*next = font->next;
}

void ft_font_free(ftFonts_s* fonts, ftFont_s* font){
	ft_fonts_remove(fonts, font);
	chash_free(&font->charmap);
	FT_Done_Face(font->face);
	free((void*)font->fname);
	free(font);
}

__private void ft_font_metric_set(ftFont_s* font){
	font->width     = font->face->size->metrics.max_advance / 64;
	font->height    = font->face->size->metrics.height / 64;
	font->descender = font->face->size->metrics.descender / 64;
	font->ascender  = font->face->size->metrics.ascender / 64;
	font->advanceX  = font->face->max_advance_width / 64;
	font->advanceY  = font->face->max_advance_height /64;
}

err_t ft_font_size(ftFont_s* font, long w, long h){
	err_t err;	
	if( (err=FT_Set_Pixel_Sizes(font->face, w, h)) ){
		dbg_error("freetype2 set size error %d: %s", err, ftStrError[err]);
		return err;
	}
	ft_font_metric_set(font);
	return 0;
}

err_t ft_font_size_dpi(ftFont_s* font, long w, long h, long dpiw, long dpih){
	err_t err;
	if( (err=FT_Set_Char_Size(font->face, w*64, h*64, dpiw, dpih)) ){
		dbg_error("freetype2 set size error %d: %s", err, ftStrError[err]);
		return err;
	}
	ft_font_metric_set(font);
	return 0;
}

__private int _ft_ch_cmp(void* a, __unused uint32_t ht, void* data, __unused const char* b){
	ftRender_s* glyph = data;
	return (glyph->utf == *(utf_t*)a) ? 0 : 1;
}

ftRender_s* ft_glyph_get(ftFont_s* font, utf_t utf){
	void* out;
	if( chash_find_fromhash_custom(&out, &font->charmap, utf, &utf, _ft_ch_cmp) ){
		return NULL;
	}
	return out;
}

ftRender_s* ft_fallback_glyph_get(ftFonts_s* fonts, utf_t utf){
	ftRender_s* glyph = NULL;
	for(ftFont_s* font = fonts->font; font; font = font->next){
		if( (glyph=ft_glyph_get(font, utf)) ){
			return glyph;
		}
	}
	return glyph;	
}

__private void ft_glyph_render_hori_mono_byte(ftRender_s* glyph, unsigned char* buf, g2dMode_e mode){
	if( glyph->img.pixel ) return;

	unsigned const w = glyph->img.w;
	unsigned const h = glyph->img.h;
	g2d_init(&glyph->img, w, h, mode);

	g2dColor_t whyte = g2d_color_make(&glyph->img, 255, 255, 255, 255);
	g2dColor_t black = g2d_color_make(&glyph->img, 255, 0, 0, 0);

	g2dCoord_s pos = {.x = 0, .y = 0, .w = w, .h = h};
	g2d_clear(&glyph->img, whyte, &pos);
	
	unsigned const gw = glyph->width;
	unsigned const gh = (glyph->height > h) ? h : glyph->height;
	unsigned const p = glyph->pitch;
	unsigned y = ((h - (-glyph->descender)) - glyph->horiBearingY) < 0 ? 0 : ((h - (-glyph->descender)) - glyph->horiBearingY);

	for( unsigned gy = 0; gy < gh && gy < h; ++y, ++gy ){
		unsigned const row = g2d_row(&glyph->img, y);
		unsigned const gr = gy * p;
		for( 
				unsigned gx = 0, ix = glyph->horiBearingX; 
				gx < gw && ix < w;
			   	++gx, ++ix
		){
			if( (buf[gr+gx/8] << gx%8) & 0x80 ){
				g2dColor_t* pixel = g2d_color(&glyph->img, row, ix);
				*pixel = black;
			}
		}
	}
}

__private void ft_glyph_render_graphics(ftRender_s* glyph, unsigned char* buf, g2dMode_e mode){
	if( glyph->img.pixel ) return;

	unsigned const w = glyph->img.w;
	unsigned const h = glyph->img.h;
	g2d_init(&glyph->img, w, h, mode);

	g2dColor_t whyte = g2d_color_make(&glyph->img, 0, 255, 255, 255);

	g2dCoord_s pos = {.x = 0, .y = 0, .w = w, .h = h};
	g2d_clear(&glyph->img, whyte, &pos);
	
	unsigned const gw = glyph->width;
	unsigned const gh = (glyph->height > h) ? h : glyph->height;
	unsigned y = ((h - (-glyph->descender)) - glyph->horiBearingY) < 0 ? 0 : ((h - (-glyph->descender)) - glyph->horiBearingY);

	for( unsigned gy = 0; gy < gh && gy < h; ++y, ++gy ){
		unsigned const row = g2d_row(&glyph->img, y);
		unsigned const gr = gy * gw;
		for( 
				unsigned gx = 0, ix = glyph->horiBearingX; 
				gx < gw && ix < w;
			   	++gx, ++ix
		){
			unsigned char gc = buf[ gr + gx ];
			unsigned char bw = gc ? 0 : 255;
			g2dColor_t* pixel = g2d_color(&glyph->img, row, ix);
			*pixel = g2d_color_make(&glyph->img, gc, bw, bw, bw); 
		}
	}
	
}

/*
//bit position
//     0123456 
//8  0 0000000 1 0
//16 1 000#00  2 00
//24 2 00000   3 000
//32 3 0000    4 0000
//40 4 000     5 00000
//48 5 00      6 000000
//56 6 0       7 00#0000
//64 7 0000000 8
//72 8 000000  9 0

// y * w + x
//y7 w7 x3
//7*7+3=49+3=52
//y1 w7 x4
//1*7+4=7+4=11
//byte = (y*w+x)/8
//bits = (y*w+x)%8
__private void ft_glyph_render_hori_mono(ftRender_s* glyph, unsigned char* buf){
	if( glyph->pixel ) return;
	
	size_t nbyte = ((glyph->pixelWidth*glyph->pixelHeight)/8)+1;
	glyph->pixel  = mem_zero_many(unsigned char, nbyte);
	iassert(glyph->pixel);

	unsigned w = glyph->pixelWidth;
	unsigned h = glyph->pixelHeight;
	unsigned gw = glyph->width;
	unsigned gh = (glyph->height > h) ? h : glyph->height;
	unsigned p = glyph->pitch;
	unsigned y = (h + glyph->descender) - glyph->horiBearingY < 0 ? 0 : (h + glyph->descender) - glyph->horiBearingY;
	unsigned gy;

	for(gy = 0; gy < gh && y < h; ++y, ++gy){
		unsigned x = glyph->horiBearingX;
		unsigned r = y * w;
		unsigned gr = gy * p;
		for( unsigned gx = 0; (gx < gw) && (x < w) ; ++gx, ++x){
			if( (buf[gr+gx/8] << gx%8) & 0x80 ){
				unsigned byte = (r+x)/8;
				unsigned bit  = (r+x)%8; 
				//dbg_info("render r(%u) gx(%u/%u) gy(%u/%u) x(%u/%u) y(%u/%u) B(%u/%lu) b(%u)", r, gx, gw, gy, gh, x, w, y, h, byte, nbyte, bit);
				iassert( byte < nbyte );
				glyph->pixel[byte] |= 1 << (7-bit);
			}
		}
	}
}
*/

ftRender_s* ft_glyph_load(ftFont_s* font, utf_t utf, unsigned mode){
//	dbg_info("load glyph (%u)%c", utf,utf);
	iassert(font);
	ftRender_s* glyph = NULL;
	if( (glyph=ft_glyph_get(font, utf)) ){
		return glyph;
	}

	unsigned index = FT_Get_Char_Index(font->face, utf);
	if( (mode & FT_RENDER_VALID) ){
		if (index == 0) {
			return NULL;
		}
		mode &=~FT_RENDER_VALID;
	}

	err_t err;
	if( (err=FT_Load_Glyph(font->face, index, 0)) ){
		dbg_error("freetype2 load glyph error %d: %s", err, ftStrError[err]);
		free(glyph);
		return NULL;
	}

	if( font->face->glyph->format != FT_GLYPH_FORMAT_BITMAP ){
		if( (err=FT_Render_Glyph( font->face->glyph, mode & FT_RENDER_ANTIALIASED ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO )) ){
			dbg_error("freetype2 render glyph error %d: %s", err, ftStrError[err]);
			return NULL;
		}
	}

	glyph = mem_new(ftRender_s);
	iassert( glyph );
	glyph->pitch = font->face->glyph->bitmap.pitch;
	glyph->width = font->face->glyph->bitmap.width;
	glyph->height = font->face->glyph->bitmap.rows;
	glyph->horiBearingX = font->face->glyph->metrics.horiBearingX/64;
	glyph->horiBearingY = font->face->glyph->metrics.horiBearingY/64;
	glyph->horiAdvance =  font->face->glyph->metrics.horiAdvance/64;
	glyph->penAdvance = font->face->glyph->bitmap_left;
	glyph->vertBearingX = font->face->glyph->metrics.vertBearingX/64;
	glyph->vertBearingY = font->face->glyph->metrics.vertBearingY/64;
	glyph->vertAdvance = font->face->glyph->metrics.vertAdvance/64;
	glyph->linearHoriAdvance = font->face->glyph->linearHoriAdvance/65536;
	glyph->linearVertAdvance = font->face->glyph->linearVertAdvance/65536;
	glyph->img.pixel = NULL;
	glyph->img.h = font->height;
	glyph->img.w  = font->width;
	glyph->descender = font->descender;
	glyph->utf = utf;
	chash_add_fromhash(&font->charmap, utf, (char*)&glyph->utf, 6, 0, glyph);

	//dbg_error("%c fw:%lu w:%u ha:%u fa:%d",utf, font->width, glyph->width, glyph->horiAdvance, font->face->glyph->bitmap_left);

	if( mode & FT_RENDER_BYTE ){
		dbg_info("mono");
		ft_glyph_render_hori_mono_byte(glyph, font->face->glyph->bitmap.buffer, G2D_MODE_BGRA);
	}
	else if( mode & FT_RENDER_ANTIALIASED ){
		dbg_info("antialaised");
		ft_glyph_render_graphics(glyph, font->face->glyph->bitmap.buffer, G2D_MODE_ARGB);
	}
	else{
		dbg_fail("not implemented");
	}
	return glyph;
}

ftRender_s* ft_fallback_glyph_load(ftFonts_s* fonts, utf_t utf, unsigned mode){
	ftRender_s* glyph = NULL;
	for(ftFont_s* font = fonts->font; font; font = font->next){
		if( (glyph=ft_glyph_get(font, utf)) ){
			return glyph;
		}
		if( FT_Get_Char_Index(font->face, utf) ){
			return ft_glyph_load(font, utf, mode);
		}
	}
	return ft_glyph_load(fonts->font, utf, mode);
}

void ft_glyph_free(ftRender_s* glyph){
	g2d_unload(&glyph->img);
	free(glyph);
}

int ft_glyph_min_width(ftFont_s* font, utf_t utf){
	unsigned index = FT_Get_Char_Index(font->face, utf);

	err_t err;
	if( (err=FT_Load_Glyph(font->face, index, 0)) ){
		dbg_error("freetype2 load glyph error %d: %s", err, ftStrError[err]);
		return -1;
	}

	if( font->face->glyph->format != FT_GLYPH_FORMAT_BITMAP ){
		if( (err=FT_Render_Glyph( font->face->glyph, FT_RENDER_MODE_MONO )) ){
			dbg_error("freetype2 render glyph error %d: %s", err, ftStrError[err]);
			return -1;
		}
	}

	return font->face->glyph->metrics.horiAdvance/64;
}
void ft_font_render_size(ftFont_s* font, unsigned w, unsigned h){
	font->width = w;
	font->height = h;
}

void ft_font_print_info(ftFont_s* font){
	printf("[font]\n");
	printf("path: %s ", font->fname);
	printf("family: %s ", font->face->family_name);
	printf("style: %s\n", font->face->style_name);
	printf("nglyphs: %ld ", font->face->num_glyphs);
	printf("EM: %d ", font->face->units_per_EM);
	printf("max_advance: %ld ", font->face->size->metrics.max_advance/64);
	printf("height: %ld ", font->height);
	printf("descending: %ld ", font->descender);
	printf("count size: %d\n", font->face->num_fixed_sizes);
	printf("size: ");
	for(int i = 0; i < font->face->num_fixed_sizes; ++i){
		printf("%ld|%d*%d ", font->face->available_sizes[i].size, font->face->available_sizes[i].width, font->face->available_sizes[i].height);
	}
	printf("\n");
}

#ifdef DEBUG_ENABLE
/*
__private void draw_hmB(ftRender_s* render){
	for( unsigned i = 0; i < render->pixelWidth + 2; ++i)
		putchar('O');
	putchar('\n');
	for( unsigned y = 0; y < render->pixelHeight; ++y){
		putchar('O');
		unsigned p = y * render->pixelWidth;
		for( unsigned x = 0; x < render->pixelWidth; ++x){
			putchar( render->pixel[p+x] ? '#' : ' ' );
		}
		puts("O");
	}
	for( unsigned i = 0; i < render->pixelWidth + 2; ++i)
		putchar('O');
	putchar('\n');
}

__private void draw_hmb(ftRender_s* render){
	for( unsigned i = 0; i < render->pixelWidth + 2; ++i)
		putchar('O');
	putchar('\n');
	for( unsigned y = 0; y < render->pixelHeight; ++y){
		putchar('O');
		unsigned r = y * render->pixelWidth;
		for( unsigned x = 0; x < render->pixelWidth; ++x){
			unsigned byte = (r+x)/8;
			unsigned bit  = (r+x)%8;
			//dbg_info("render x(%u/%u) y(%u/%u) B(%u) b(%u)", x, render->pixelWidth, y, render->pixelHeight, byte, bit);
			putchar( render->pixel[byte] & (1<<(7-bit)) ? '#' : ' ' );
		}
		puts("O");
	}
	for( unsigned i = 0; i < render->pixelWidth + 2; ++i)
		putchar('O');
	putchar('\n');
}

void ft_font_test(int mode){
	ftlib_h lib;
	ftFonts_s fonts;
	ftFont_s* font;
	const char* fontName = "/usr/share/fonts/TTF/Unifont.ttf";
	const size_t size = 16;
	utf8_t* fontPatterns = (utf8_t*)"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789▀▁▂▃▄▅▆▇█▉▊▋▌▍▎▏▐░▒▓▔▕▖▗▘▙▚▛▜▝▞▟";

	if( ft_init(&lib) ) return;
	ft_fonts_init(lib, &fonts);
	if( !(font=ft_fonts_load(&fonts, fontName)) ){	
		ft_terminate(lib);
		return;
	}
	if( ft_font_size(font, 0, size) ){
		ft_fonts_free(&fonts);
		ft_terminate(lib);
		return;
	}
	
	//ft_font_render_size(font, 8, size);

	if( mode & FT_RENDER_BYTE ){
		puts("Render Byte");
	}
	else{
		puts("Render Bits");
	}

	utf8Iterator_s it = utf8_iterator(fontPatterns, 0);
	utf_t utf;
	while( (utf=utf8_iterator_next(&it)) ){
		ftRender_s* glyph;
		if( !(glyph=ft_glyph_load(font, utf, mode)) ){
			dbg_error("fail load glyph %u", utf);
			continue;
		}
		if( mode & FT_RENDER_BYTE ){
			draw_hmB(glyph);
		}
		else{
			draw_hmb(glyph);
		}
	}
	
	ft_font_print_info(font);
	ft_fonts_free(&fonts);
	ft_terminate(lib);
}
*/
#endif

unsigned ft_line_height(ftFonts_s* fonts){
	ftRender_s* rch = ft_fallback_glyph_load(fonts, ' ', FT_RENDER_ANTIALIASED);
	return rch->img.h;
}

unsigned ft_line_lenght(ftFonts_s* fonts, utf8_t* str){
	if( !str ){
		dbg_warning("no str");
		return 0;
	}
	unsigned lenght = 0;
	utf8Iterator_s it = utf8_iterator(str, 0);
	utf_t u;	
	while( (u = utf8_iterator_next(&it)) ){
		ftRender_s* rch = ft_fallback_glyph_load(fonts, u, FT_RENDER_ANTIALIASED);
		lenght += rch->horiAdvance;
	}
	return lenght;
}	

unsigned ft_autowrap_height(ftFonts_s* fonts, utf8_t* str, unsigned width){
	if( !str ){
		dbg_warning("no str");
		return 0;
	}

	unsigned monoh = ft_line_height(fonts);
	unsigned height = monoh;
	unsigned lenght = 0;

	utf8Iterator_s it = utf8_iterator(str, 0);
	utf_t utf;
	while( (utf = utf8_iterator_next(&it)) ){
		if( utf == '\n' ){
			height += monoh;
			lenght = 0;
			continue;
		}
		if( lenght >= width ){
			height += monoh;
			lenght = 0;
		}	
		ftRender_s* rch = ft_fallback_glyph_load(fonts, *str, FT_RENDER_ANTIALIASED);
		lenght += rch->horiAdvance;
	}

	return height;
}	

void g2d_putch(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf_t ch, g2dColor_t fore, g2dColor_t back, unsigned originX, int cls){
	if( ch == '\n' ){
		pos->y += ft_line_height(fonts);
		pos->x = originX;
		return;
	}
	
	ftRender_s* rch = ft_fallback_glyph_load(fonts, ch, FT_RENDER_ANTIALIASED);
	pos->w = rch->img.w;
	if( pos->x + pos->w > dst->w ) return;
	if( pos->y + rch->img.h > dst->h ) return;
	if( cls ) g2d_clear(dst, back, pos);
	unsigned const oh = pos->h;
	pos->h = rch->img.h;
	g2d_char(dst, pos, &rch->img, fore);
	pos->h = oh;
	pos->x += rch->horiAdvance;
}

void g2d_putch_autowrap(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf_t ch, g2dColor_t fore, g2dColor_t back, unsigned originX, int cls){
	if( ch == '\n' ){
		pos->y += ft_line_height(fonts);
		pos->x = originX;
		return;
	}
	
	ftRender_s* rch = ft_fallback_glyph_load(fonts, ch, FT_RENDER_ANTIALIASED);
	pos->w = rch->img.w;
	pos->h = rch->img.h;
	if( pos->x + pos->w > dst->w ){
	   	pos->y += ft_line_height(fonts);
		pos->x = originX;	
	}
	if( pos->y + pos->h > dst->h ) return;
	if( cls ) g2d_clear(dst, back, pos);
	g2d_char(dst, pos, &rch->img, fore);
	pos->x += rch->horiAdvance;
}

void g2d_string(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf8_t const* str, g2dColor_t col, unsigned originX){
	utf8Iterator_s it = utf8_iterator((utf8_t*)str, 0);
	utf_t utf;
	while( (utf = utf8_iterator_next(&it)) ){
		g2d_putch(dst, pos, fonts, utf, col, 0,  originX, 0);
	}
}

void g2d_string_autowrap(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf8_t const* str, g2dColor_t col, unsigned originX){
	utf8Iterator_s it = utf8_iterator((utf8_t*)str, 0);
	utf_t utf;
	while( (utf = utf8_iterator_next(&it)) ){
		g2d_putch_autowrap(dst, pos, fonts, utf, col, 0,  originX, 0);
	}
}

void g2d_string_replace(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf8_t const* str, utf8_t const* old, g2dColor_t f, g2dColor_t b, unsigned originX){
	if( !str || !old ) return;
	utf8Iterator_s it = utf8_iterator((utf8_t*)str, 0);
	utf8Iterator_s ot = utf8_iterator((utf8_t*)old, 0);
	utf_t utf, of;
	while( (utf = utf8_iterator_next(&it)) ){
		of = utf8_iterator_next(&ot);
		if( of == utf ){
			ftRender_s* rch = ft_fallback_glyph_load(fonts, utf, FT_RENDER_ANTIALIASED);
			pos->x += rch->horiAdvance;
		}
		else{
			g2d_putch(dst, pos, fonts, utf, f, b, originX, 1);
		}
		if( !of ) break;
	}

	while( (utf = utf8_iterator_next(&it)) ){
		g2d_putch(dst, pos, fonts, utf, f, b, originX, 1);
	}

	while( (of = utf8_iterator_next(&ot)) ){
		ftRender_s* rch = ft_fallback_glyph_load(fonts, utf, FT_RENDER_ANTIALIASED);
		pos->w = rch->horiAdvance;
		pos->h = rch->img.h;
		g2d_clear(dst, b, pos);
		pos->x += rch->horiAdvance;
	}
}











