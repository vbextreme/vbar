#ifndef __EF_FREETYPE_H__
#define __EF_FREETYPE_H__

//TODO
//bug character _

#include <ef/type.h>
#include <ef/utf8.h>
#include <ef/chash.h>
#include <ef/image.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

typedef FT_Library ftlib_h;

typedef struct ftRender{
	utf_t utf;
	g2dImage_s img;

	unsigned width;
	unsigned height;
	unsigned pitch;
	unsigned horiAdvance;
	int horiBearingX;
	int horiBearingY;
	unsigned linearHoriAdvance;
	unsigned vertAdvance;
	unsigned vertBearingX;
	unsigned vertBearingY;
	unsigned linearVertAdvance;
	long descender;
	int penAdvance;
}ftRender_s;

typedef struct ftFont{
	struct ftFont* next;
	const char* fname;
	FT_Face face;
	unsigned long width;
	unsigned long height;
	long descender;
	long ascender;
	long advanceX;
	long advanceY;
	chash_s charmap;
}ftFont_s;

typedef struct ftFonts{
	ftlib_h lib;
	ftFont_s* font;
}ftFonts_s;

#define FT_RENDER_ANTIALIASED 0x1
#define FT_RENDER_VERT        0x2
#define FT_RENDER_VALID       0x4
#define FT_RENDER_BYTE		  0x8

//pitch number byte for line
#define ft_buf_line(Y,P) ((Y)*(P))
#define ft_buf_mono_get(buf,X,Y,P) ((buf[ft_buf_line(Y,P)] << (X)) & 0x80)
#define ft_pixel_byte_get(PX) ((PX)/8)
#define ft_pixel_bit_get(PX)  ((PX)%8)
#define ft_pixel_bitval_get(BUF, PX) (BUF[ft_pixel_byte_get(PX)] & (1<<(7-ft_pixel_bit_get(PX))));

err_t ft_init(ftlib_h* ftl);
void ft_terminate(ftlib_h ftl);
void ft_fonts_init(ftlib_h ftl, ftFonts_s* fonts);
void ft_fonts_free(ftFonts_s* fonts);
ftFont_s* ft_fonts_search(ftFonts_s* fonts, const char* path);
ftFont_s* ft_fonts_search_index(ftFonts_s* fonts, unsigned index);
char* ft_file_search(char const* path);
ftFont_s* ft_fonts_load(ftFonts_s* fonts, const char* path);
void ft_font_free(ftFonts_s* fonts, ftFont_s* font);
err_t ft_font_size(ftFont_s* font, long w, long h);
err_t ft_font_size_dpi(ftFont_s* font, long w, long h, long dpiw, long dpih);
ftRender_s* ft_glyph_get(ftFont_s* font, utf_t utf);
ftRender_s* ft_fallback_glyph_get(ftFonts_s* fonts, utf_t utf);
ftRender_s* ft_glyph_load(ftFont_s* font, utf_t utf, unsigned mode);
ftRender_s* ft_fallback_glyph_load(ftFonts_s* fonts, utf_t utf, unsigned mode);
void ft_glyph_free(ftRender_s* glyph);
void ft_font_render_size(ftFont_s* font, unsigned w, unsigned h);
int ft_glyph_min_width(ftFont_s* font, utf_t utf);
void ft_font_print_info(ftFont_s* font);

unsigned ft_line_height(ftFonts_s* fonts);
unsigned ft_line_lenght(ftFonts_s* fonts, utf8_t* str);
unsigned ft_autowrap_height(ftFonts_s* fonts, utf8_t* str, unsigned width);
void g2d_putch(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf_t ch, g2dColor_t fore, g2dColor_t back, unsigned originX, int cls);
void g2d_putch_autowrap(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf_t ch, g2dColor_t fore, g2dColor_t back, unsigned originX, int cls);
void g2d_string(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf8_t const* str, g2dColor_t col, unsigned originX);
void g2d_string_autowrap(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf8_t const* str, g2dColor_t col, unsigned originX);
void g2d_string_replace(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf8_t const* str, utf8_t const* old, g2dColor_t f, g2dColor_t b, unsigned originX);

#ifdef DEBUG_ENABLE
	void ft_font_test(int mode);
#else
	#define ft_font_test(Z)
#endif

#endif
