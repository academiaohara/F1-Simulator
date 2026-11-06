#ifndef GFX_H
#define GFX_H
#include <stdint.h>

#define SW 640
#define SH 360

typedef uint32_t Col;
#undef RGB
#define RGB(r,g,b) ((Col)(((r)<<16)|((g)<<8)|(b)))
#define COL_R(c) (((c)>>16)&255)
#define COL_G(c) (((c)>>8)&255)
#define COL_B(c) ((c)&255)

/* paleta retro */
#define C_BG     RGB(14,17,34)
#define C_PANEL  RGB(26,32,62)
#define C_PANEL2 RGB(38,47,88)
#define C_PANEL3 RGB(54,66,118)
#define C_LINE   RGB(120,138,200)
#define C_LINE_D RGB(18,22,46)
#define C_TEXT   RGB(232,236,250)
#define C_DIM    RGB(128,140,176)
#define C_YEL    RGB(255,214,80)
#define C_GRN    RGB(98,214,124)
#define C_RED    RGB(236,84,84)
#define C_CYN    RGB(98,206,236)
#define C_ORG    RGB(255,152,62)
#define C_PNK    RGB(240,112,174)
#define C_WHT    RGB(255,255,255)
#define C_BLK    RGB(8,8,16)

extern Col fb[SW * SH];

void gfx_init(void);
Col  col_mix(Col a, Col b, int t256);
Col  col_dark(Col c, int amount);   /* resta */
Col  col_light(Col c, int amount);
void px(int x, int y, Col c);
Col  getpx(int x, int y);
void clear(Col c);
void rect(int x, int y, int w, int h, Col c);
void rect_dither(int x, int y, int w, int h, Col a, Col b);
void box(int x, int y, int w, int h, Col c);              /* contorno 1 px */
void line(int x0, int y0, int x1, int y1, Col c);
void disc(int cx, int cy, int r, Col c);
void ring(int cx, int cy, int r, Col c);
void set_clip(int x, int y, int w, int h);
void reset_clip(void);

int  text_w(const char *s);
void text(int x, int y, const char *s, Col c);
void text_shadow(int x, int y, const char *s, Col c);
void text_c(int cx, int y, const char *s, Col c);         /* centrado */
void text_r(int rx, int y, const char *s, Col c);         /* alineado a la derecha */
void textf(int x, int y, Col c, const char *fmt, ...);
void text_fit(int x, int y, int maxw, const char *s, Col c);
int  text_wrap(int x, int y, int w, const char *s, Col c, int maxlines);  /* devuelve lineas usadas */

void png_save(const char *path, int scale);

#endif
