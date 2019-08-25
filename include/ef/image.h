#ifndef __EF_IMAGE_H__
#define __EF_IMAGE_H__

#include <ef/type.h>
#include <ef/utf8.h>

typedef enum {G2D_MODE_BGRA, G2D_MODE_ABGR, G2D_MODE_ARGB, G2D_MODE_RGBA} g2dMode_e;

typedef struct g2dImage{
	unsigned char* pixel;
	unsigned w,h,p,sa,sr,sg,sb,ma,mr,mg,mb;
	g2dMode_e mode;
}g2dImage_s;

typedef unsigned int g2dColor_t;

typedef struct g2dCoord{
	unsigned x,y,w,h;
}g2dCoord_s;

typedef struct g2dPoint{
	unsigned x,y;
}g2dPoint_s;


#define img_clip(V) ((V) < 0 ? 0 : (V) > 255 ? 255 : (V))
#define img_clip_h(V) ((V>255)?255:V)

void img_rgb_to_yuv8(unsigned char* y, unsigned char* u, unsigned char* v, unsigned char r ,unsigned char g, unsigned char b);
void img_yuv8_to_rgb(unsigned char* r, unsigned char* g, unsigned char* b, unsigned char y ,unsigned char u, unsigned char v);
int img_color_h(int R, int G, int B);
unsigned char img_rgb_to_gray(unsigned char r, unsigned char g, unsigned char b);

void g2d_begin(void);
void g2d_zero(g2dImage_s* img);
void g2d_init(g2dImage_s* img, unsigned w, unsigned h, g2dMode_e mode);
g2dImage_s g2d_new(unsigned w, unsigned h, g2dMode_e mode);
void g2d_clone(g2dImage_s* img, unsigned w, unsigned h, g2dMode_e mode, uint8_t* pixels);
err_t g2d_load(g2dImage_s* img, char const* path);
#define g2d_unload(IMG) do{free((IMG)->pixel);(IMG)->pixel = NULL;}while(0)
#define __g2dImage_autounload __cleanup(_g2d_autounload)
void _g2d_autounload(g2dImage_s* img);
void g2d_ratio(int modeAWH, unsigned sw, unsigned sh, unsigned* w, unsigned* h);
void g2d_copy(g2dImage_s* dst, g2dImage_s* src);
void g2d_bitblt(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos);
void g2d_bitblt_xor(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos);
void g2d_bitblt_alpha(g2dImage_s* dst, g2dCoord_s* cod, g2dImage_s* src, g2dCoord_s* cos);
void g2d_clear(g2dImage_s* img, g2dColor_t color, g2dCoord_s* coord);
void g2d_channel_set(g2dImage_s* img, g2dColor_t color, g2dCoord_s* coord, unsigned mask);
#define g2d_row(IMG, Y) (((IMG)->p)*(Y))
#define g2d_pixel(IMG,ROW) (&(IMG)->pixel[ROW])
#define g2d_color(IMG,ROW,X) (unsigned*)(&(IMG)->pixel[(ROW)+((X)*4)])
#define g2d_color_alpha(IMG,ARGB) (((ARGB)>>(IMG)->sa)&0xFF)
#define g2d_color_red(IMG,ARGB)   (((ARGB)>>(IMG)->sr)&0xFF)
#define g2d_color_green(IMG,ARGB) (((ARGB)>>(IMG)->sg)&0xFF)
#define g2d_color_blue(IMG,ARGB)  (((ARGB)>>(IMG)->sb)&0xFF)
#define g2d_color_make(IMG,A,R,G,B) ( ((A)<<(IMG)->sa) | ((R)<<(IMG)->sr) | ((G)<<(IMG)->sg) | ((B)<<(IMG)->sb) )
#define g2d_color_alpha_set(IMG,ARGB,A) ( ((ARGB) & (~(IMG)->ma)) | ((A)<<(IMG)->sa) )
g2dColor_t g2d_color_gen(g2dMode_e mode, unsigned a, unsigned r, unsigned g, unsigned b);

#define g2d_alpha_part(ALPHA, PART, BACKGROUND) ( ((PART) * (ALPHA)) / 255  + (BACKGROUND) *  (255 - (ALPHA)) / 255 )

void g2d_luminance(g2dImage_s* img);

void g2d_black_white(g2dImage_s* gray, g2dCoord_s * coord);
void g2d_black_white_dominant(g2dColor_t* outAB, g2dImage_s* src, g2dImage_s* bw, g2dCoord_s* coord);
void g2d_black_white_set(g2dImage_s* bw, g2dCoord_s* coord, g2dColor_t* colorAB);
unsigned g2d_compare_similar(g2dImage_s* a, g2dCoord_s* ca, g2dImage_s* b, g2dCoord_s* cb);
unsigned g2d_bitcount(g2dImage_s* img, g2dCoord_s* coord);
void g2d_resize(g2dImage_s* dst, g2dImage_s* src, unsigned w, unsigned h, int ratio);
void g2d_rotate(g2dImage_s* dst, g2dImage_s* src, unsigned cx, unsigned cy, float grad);

void g2d_char(g2dImage_s* dst, g2dCoord_s* coord, g2dImage_s* ch, g2dColor_t col);

/*ascii*/
typedef struct asciiImageBlock{
	utf_t utf;
	g2dColor_t dominant[2];
}asciiImageBlock_s;

typedef struct asciiImage{
	asciiImageBlock_s* blk;
	unsigned w;
	unsigned h;
}asciiImage_s;

void img_ascii_init(asciiImage_s* img, unsigned w, unsigned h);
void img_ascii_unload(asciiImage_s* img);


/*primitive*/
void g2d_point_rotate(unsigned* y, unsigned* x, unsigned cy, unsigned cx, double grad);
void g2d_points(g2dImage_s* img, g2dPoint_s* points, g2dColor_t* colors, size_t count);
void g2d_hline(g2dImage_s* img, g2dPoint_s* st, unsigned x1, g2dColor_t col);
void g2d_vline(g2dImage_s* img, g2dPoint_s* st, unsigned y1, g2dColor_t col);
void g2d_bline(g2dImage_s* img, g2dPoint_s* st, g2dPoint_s* en, g2dColor_t col);
void g2d_bline_antialiased(g2dImage_s* img, g2dPoint_s* st, g2dPoint_s* en, g2dColor_t col);
void g2d_line(g2dImage_s* img, g2dPoint_s* st, g2dPoint_s* en, g2dColor_t col, int antialaised);
void g2d_cubezier(g2dImage_s* img, g2dPoint_s* p0, g2dPoint_s* p1, g2dPoint_s* p2, g2dPoint_s* p3, g2dColor_t col);
void g2d_triangle(g2dImage_s* img, g2dPoint_s* p0, g2dPoint_s* p1, g2dPoint_s* p2, g2dColor_t col, int antialaised);
void g2d_triangle_fill(g2dImage_s* img, g2dPoint_s* p0, g2dPoint_s* p1, g2dPoint_s* p2, g2dColor_t col);
void g2d_rect(g2dImage_s* img, g2dCoord_s* rc, g2dColor_t col);
void g2d_rect_fill(g2dImage_s* img, g2dCoord_s* rc, g2dColor_t col);
void g2d_circle(g2dImage_s* img, g2dPoint_s* cx, unsigned r, g2dColor_t col);
void g2d_circle_fill(g2dImage_s* img, g2dPoint_s* cx, unsigned r, g2dColor_t col);
void g2d_ellipse(g2dImage_s* img, g2dPoint_s* cx, unsigned rx, unsigned ry, g2dColor_t col);
void g2d_repfill(g2dImage_s* img, g2dPoint_s* st, g2dColor_t rep, g2dColor_t col);



#endif 
