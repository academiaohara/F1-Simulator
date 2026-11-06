#include "gfx.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

Col fb[SW * SH];
static int cx0 = 0, cy0 = 0, cx1 = SW, cy1 = SH;

void gfx_init(void) { reset_clip(); }
void set_clip(int x, int y, int w, int h) {
    cx0 = x < 0 ? 0 : x; cy0 = y < 0 ? 0 : y;
    cx1 = x + w > SW ? SW : x + w; cy1 = y + h > SH ? SH : y + h;
}
void reset_clip(void) { cx0 = 0; cy0 = 0; cx1 = SW; cy1 = SH; }

Col col_mix(Col a, Col b, int t) {
    if (t < 0) t = 0;
    if (t > 256) t = 256;
    int r = (COL_R(a) * (256 - t) + COL_R(b) * t) >> 8;
    int g = (COL_G(a) * (256 - t) + COL_G(b) * t) >> 8;
    int bl = (COL_B(a) * (256 - t) + COL_B(b) * t) >> 8;
    return RGB(r, g, bl);
}
Col col_dark(Col c, int a) {
    int r = COL_R(c) - a, g = COL_G(c) - a, b = COL_B(c) - a;
    return RGB(r < 0 ? 0 : r, g < 0 ? 0 : g, b < 0 ? 0 : b);
}
Col col_light(Col c, int a) {
    int r = COL_R(c) + a, g = COL_G(c) + a, b = COL_B(c) + a;
    return RGB(r > 255 ? 255 : r, g > 255 ? 255 : g, b > 255 ? 255 : b);
}

void px(int x, int y, Col c) {
    if (x < cx0 || y < cy0 || x >= cx1 || y >= cy1) return;
    fb[y * SW + x] = c;
}
Col getpx(int x, int y) { if (x < 0 || y < 0 || x >= SW || y >= SH) return 0; return fb[y * SW + x]; }
void clear(Col c) { for (int i = 0; i < SW * SH; i++) fb[i] = c; }

void rect(int x, int y, int w, int h, Col c) {
    int x1 = x + w, y1 = y + h;
    if (x < cx0) x = cx0;
    if (y < cy0) y = cy0;
    if (x1 > cx1) x1 = cx1;
    if (y1 > cy1) y1 = cy1;
    for (int j = y; j < y1; j++) { Col *p = fb + j * SW; for (int i = x; i < x1; i++) p[i] = c; }
}
void rect_dither(int x, int y, int w, int h, Col a, Col b) {
    for (int j = 0; j < h; j++) for (int i = 0; i < w; i++) px(x + i, y + j, ((i + j) & 1) ? b : a);
}
void box(int x, int y, int w, int h, Col c) {
    rect(x, y, w, 1, c); rect(x, y + h - 1, w, 1, c); rect(x, y, 1, h, c); rect(x + w - 1, y, 1, h, c);
}
void line(int x0, int y0, int x1, int y1, Col c) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    for (;;) {
        px(x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}
void disc(int cx, int cy, int r, Col c) {
    for (int j = -r; j <= r; j++) for (int i = -r; i <= r; i++) if (i * i + j * j <= r * r + r / 2) px(cx + i, cy + j, c);
}
void ring(int cx, int cy, int r, Col c) {
    for (int j = -r; j <= r; j++) for (int i = -r; i <= r; i++) {
        int d = i * i + j * j;
        if (d <= r * r + r / 2 && d >= (r - 1) * (r - 1) + (r - 1) / 2) px(cx + i, cy + j, c);
    }
}

/* ---------------------------------------------------------------- fuente 5x7 (columnas, bit0 = fila superior) */
static const unsigned char FONT[96][5] = {
    {0x00,0x00,0x00,0x00,0x00},{0x00,0x00,0x5F,0x00,0x00},{0x00,0x07,0x00,0x07,0x00},{0x14,0x7F,0x14,0x7F,0x14},
    {0x24,0x2A,0x7F,0x2A,0x12},{0x23,0x13,0x08,0x64,0x62},{0x36,0x49,0x56,0x20,0x50},{0x00,0x08,0x07,0x03,0x00},
    {0x00,0x1C,0x22,0x41,0x00},{0x00,0x41,0x22,0x1C,0x00},{0x2A,0x1C,0x7F,0x1C,0x2A},{0x08,0x08,0x3E,0x08,0x08},
    {0x00,0x80,0x70,0x30,0x00},{0x08,0x08,0x08,0x08,0x08},{0x00,0x00,0x60,0x60,0x00},{0x20,0x10,0x08,0x04,0x02},
    {0x3E,0x51,0x49,0x45,0x3E},{0x00,0x42,0x7F,0x40,0x00},{0x72,0x49,0x49,0x49,0x46},{0x21,0x41,0x49,0x4D,0x33},
    {0x18,0x14,0x12,0x7F,0x10},{0x27,0x45,0x45,0x45,0x39},{0x3C,0x4A,0x49,0x49,0x31},{0x41,0x21,0x11,0x09,0x07},
    {0x36,0x49,0x49,0x49,0x36},{0x46,0x49,0x49,0x29,0x1E},{0x00,0x00,0x14,0x00,0x00},{0x00,0x40,0x34,0x00,0x00},
    {0x00,0x08,0x14,0x22,0x41},{0x14,0x14,0x14,0x14,0x14},{0x00,0x41,0x22,0x14,0x08},{0x02,0x01,0x59,0x09,0x06},
    {0x3E,0x41,0x5D,0x59,0x4E},{0x7C,0x12,0x11,0x12,0x7C},{0x7F,0x49,0x49,0x49,0x36},{0x3E,0x41,0x41,0x41,0x22},
    {0x7F,0x41,0x41,0x41,0x3E},{0x7F,0x49,0x49,0x49,0x41},{0x7F,0x09,0x09,0x09,0x01},{0x3E,0x41,0x41,0x51,0x73},
    {0x7F,0x08,0x08,0x08,0x7F},{0x00,0x41,0x7F,0x41,0x00},{0x20,0x40,0x41,0x3F,0x01},{0x7F,0x08,0x14,0x22,0x41},
    {0x7F,0x40,0x40,0x40,0x40},{0x7F,0x02,0x1C,0x02,0x7F},{0x7F,0x04,0x08,0x10,0x7F},{0x3E,0x41,0x41,0x41,0x3E},
    {0x7F,0x09,0x09,0x09,0x06},{0x3E,0x41,0x51,0x21,0x5E},{0x7F,0x09,0x19,0x29,0x46},{0x26,0x49,0x49,0x49,0x32},
    {0x03,0x01,0x7F,0x01,0x03},{0x3F,0x40,0x40,0x40,0x3F},{0x1F,0x20,0x40,0x20,0x1F},{0x3F,0x40,0x38,0x40,0x3F},
    {0x63,0x14,0x08,0x14,0x63},{0x03,0x04,0x78,0x04,0x03},{0x61,0x59,0x49,0x4D,0x43},{0x00,0x7F,0x41,0x41,0x41},
    {0x02,0x04,0x08,0x10,0x20},{0x00,0x41,0x41,0x41,0x7F},{0x04,0x02,0x01,0x02,0x04},{0x40,0x40,0x40,0x40,0x40},
    {0x00,0x03,0x07,0x08,0x00},{0x20,0x54,0x54,0x78,0x40},{0x7F,0x28,0x44,0x44,0x38},{0x38,0x44,0x44,0x44,0x28},
    {0x38,0x44,0x44,0x28,0x7F},{0x38,0x54,0x54,0x54,0x18},{0x00,0x08,0x7E,0x09,0x02},{0x18,0xA4,0xA4,0x9C,0x78},
    {0x7F,0x08,0x04,0x04,0x78},{0x00,0x44,0x7D,0x40,0x00},{0x20,0x40,0x40,0x3D,0x00},{0x7F,0x10,0x28,0x44,0x00},
    {0x00,0x41,0x7F,0x40,0x00},{0x7C,0x04,0x78,0x04,0x78},{0x7C,0x08,0x04,0x04,0x78},{0x38,0x44,0x44,0x44,0x38},
    {0xFC,0x18,0x24,0x24,0x18},{0x18,0x24,0x24,0x18,0xFC},{0x7C,0x08,0x04,0x04,0x08},{0x48,0x54,0x54,0x54,0x24},
    {0x04,0x04,0x3F,0x44,0x24},{0x3C,0x40,0x40,0x20,0x7C},{0x1C,0x20,0x40,0x20,0x1C},{0x3C,0x40,0x30,0x40,0x3C},
    {0x44,0x28,0x10,0x28,0x44},{0x4C,0x90,0x90,0x90,0x7C},{0x44,0x64,0x54,0x4C,0x44},{0x00,0x08,0x36,0x41,0x00},
    {0x00,0x00,0x77,0x00,0x00},{0x00,0x41,0x36,0x08,0x00},{0x08,0x08,0x2A,0x1C,0x08},{0x08,0x1C,0x2A,0x08,0x08}
};
/* iconos: codigos 1..15 */
static const unsigned char ICON[16][5] = {
    {0},
    {0x20,0x30,0x38,0x30,0x20},   /* 1 triangulo arriba (se dibuja con filas 2-4) */
    {0x08,0x18,0x38,0x18,0x08},   /* 2 triangulo abajo */
    {0x3E,0x1C,0x08,0x00,0x00},   /* 3 derecha */
    {0x00,0x00,0x08,0x1C,0x3E},   /* 4 izquierda */
    {0x1C,0x3E,0x3E,0x3E,0x1C},   /* 5 punto */
    {0x10,0x20,0x10,0x08,0x04},   /* 6 check */
    {0x44,0x28,0x10,0x28,0x44},   /* 7 cruz */
    {0x08,0x1C,0x3E,0x1C,0x08},   /* 8 rombo/estrella */
    {0x14,0x7F,0x55,0x55,0x41},   /* 9 euro */
    {0},
    {0x0C,0x1E,0x3C,0x1E,0x0C},   /* 11 corazon */
    {0x06,0x09,0x09,0x06,0x00},   /* 12 o pequena (ordinal) */
    {0x00,0x00,0x18,0x18,0x00},   /* 13 punto medio */
    {0x3C,0x42,0x42,0x42,0x3C},   /* 14 circulo hueco */
    {0x7E,0x42,0x42,0x42,0x7E},   /* 15 cuadro hueco */
};
/* arreglo de triangulos: la tabla de arriba para 1/2 usa bits distintos, redefinimos en codigo */

static int decode(const unsigned char **ps) {
    const unsigned char *s = *ps;
    int c = *s++;
    if (c < 0x80) { *ps = s; return c; }
    if ((c & 0xE0) == 0xC0 && *s) { int c2 = *s++; *ps = s; return ((c & 0x1F) << 6) | (c2 & 0x3F); }
    if ((c & 0xF0) == 0xE0 && s[0] && s[1]) { int c2 = *s++, c3 = *s++; *ps = s; return ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F); }
    *ps = s; return '?';
}

/* devuelve glyph ascii base y tipo de acento: 0 nada, 1 agudo, 2 tilde, 3 dieresis */
static int map_cp(int cp, int *base, int *acc) {
    *acc = 0;
    if (cp < 128) { *base = cp; return 1; }
    switch (cp) {
        case 0xE1: *base = 'a'; *acc = 1; return 1; case 0xE9: *base = 'e'; *acc = 1; return 1;
        case 0xED: *base = 'i'; *acc = 1; return 1; case 0xF3: *base = 'o'; *acc = 1; return 1;
        case 0xFA: *base = 'u'; *acc = 1; return 1; case 0xF1: *base = 'n'; *acc = 2; return 1;
        case 0xFC: *base = 'u'; *acc = 3; return 1;
        case 0xC1: *base = 'A'; *acc = 1; return 1; case 0xC9: *base = 'E'; *acc = 1; return 1;
        case 0xCD: *base = 'I'; *acc = 1; return 1; case 0xD3: *base = 'O'; *acc = 1; return 1;
        case 0xDA: *base = 'U'; *acc = 1; return 1; case 0xD1: *base = 'N'; *acc = 2; return 1;
        case 0xBF: *base = '?'; *acc = 4; return 1; case 0xA1: *base = '!'; *acc = 4; return 1;
        case 0x20AC: *base = 9; return 1;
        case 0xB7: case 0x2022: *base = 13; return 1;
        case 0x2192: case 0x25B6: *base = 3; return 1; case 0x2190: *base = 4; return 1;
        case 0x2191: *base = 1; return 1; case 0x2193: *base = 2; return 1;
        case 0x2013: case 0x2014: *base = '-'; return 1;
        case 0xBA: *base = 12; return 1;
        case 0x25B2: *base = 1; return 1; case 0x25BC: *base = 2; return 1;
        case 0x25BA: *base = 3; return 1; case 0x25C4: *base = 4; return 1;
    }
    *base = '?'; return 1;
}

static int draw_glyph(int x, int y, int cp, Col c) {
    int base, acc;
    map_cp(cp, &base, &acc);
    const unsigned char *g;
    if (base >= 1 && base < 16) {
        g = ICON[base];
        for (int i = 0; i < 5; i++) for (int j = 0; j < 8; j++) if (g[i] & (1 << j)) px(x + i, y + j, c);
        return 6;
    }
    if (base < 32 || base > 127) base = '?';
    g = FONT[base - 32];
    for (int i = 0; i < 5; i++) {
        unsigned char col = g[i];
        for (int j = 0; j < 8; j++) if (col & (1 << j)) px(x + i, y + j, c);
    }
    if (acc) {
        int up = (base >= 'A' && base <= 'Z') || base == '?' || base == '!';
        int ay = up ? -1 : 0;
        if (acc == 1) { px(x + 3, y + ay, c); if (!up) px(x + 2, y + ay + 1, c); }
        else if (acc == 2) { px(x + 1, y + ay + (up ? 0 : 1), c); px(x + 2, y + ay, c); px(x + 3, y + ay + (up ? 0 : 1), c); }
        else if (acc == 3) { px(x + 1, y + ay + 1, c); px(x + 3, y + ay + 1, c); }
        else if (acc == 4) { /* signo invertido: se pinta punto abajo/arriba simple */
            if (base == '!') { px(x + 2, y + 8, c); } else { px(x + 2, y + 8, c); }
        }
    }
    return 6;
}

int text_w(const char *s) {
    int w = 0; const unsigned char *p = (const unsigned char *)s;
    while (*p) { decode(&p); w += 6; }
    return w > 0 ? w - 1 : 0;
}
void text(int x, int y, const char *s, Col c) {
    const unsigned char *p = (const unsigned char *)s;
    while (*p) { int cp = decode(&p); x += draw_glyph(x, y, cp, c); }
}
void text_shadow(int x, int y, const char *s, Col c) { text(x + 1, y + 1, s, C_BLK); text(x, y, s, c); }
void text_c(int cx, int y, const char *s, Col c) { text(cx - text_w(s) / 2, y, s, c); }
void text_r(int rx, int y, const char *s, Col c) { text(rx - text_w(s), y, s, c); }
void textf(int x, int y, Col c, const char *fmt, ...) {
    char buf[256]; va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
    text(x, y, buf, c);
}
void text_fit(int x, int y, int maxw, const char *s, Col c) {
    const unsigned char *p = (const unsigned char *)s;
    int w = 0;
    while (*p) {
        const unsigned char *q = p;
        int cp = decode(&p);
        if (w + 5 > maxw) break;
        (void)q;
        w += draw_glyph(x + w, y, cp, c);
    }
}
int text_wrap(int x, int y, int w, const char *s, Col c, int maxlines) {
    char line[200]; int lines = 0;
    const char *p = s;
    while (*p && lines < maxlines) {
        int n = 0, last_space = -1, width = 0;
        while (p[n] && p[n] != '\n') {
            if (p[n] == ' ') last_space = n;
            if ((unsigned char)p[n] >= 0xC0) { /* utf8 lead */ width += 6; }
            else if (((unsigned char)p[n] & 0xC0) != 0x80) width += 6;
            if (width > w) { if (last_space > 0) n = last_space; break; }
            n++;
            if (n >= (int)sizeof(line) - 2) break;
        }
        if (n == 0) { if (*p == '\n') { p++; lines++; continue; } n = 1; }
        memcpy(line, p, n); line[n] = 0;
        text(x, y + lines * 9, line, c);
        lines++;
        p += n;
        while (*p == ' ') p++;
        if (*p == '\n') p++;
    }
    return lines;
}

/* ---------------------------------------------------------------- PNG (sin compresion) para capturas de prueba */
static uint32_t crc_table[256]; static int crc_ready = 0;
static uint32_t crc_update(uint32_t crc, const unsigned char *buf, size_t n) {
    if (!crc_ready) { for (uint32_t i = 0; i < 256; i++) { uint32_t c = i; for (int k = 0; k < 8; k++) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1; crc_table[i] = c; } crc_ready = 1; }
    crc ^= 0xFFFFFFFFu; for (size_t i = 0; i < n; i++) crc = crc_table[(crc ^ buf[i]) & 255] ^ (crc >> 8); return crc ^ 0xFFFFFFFFu;
}
static void put32(FILE *f, uint32_t v) { unsigned char b[4] = { (unsigned char)(v >> 24), (unsigned char)(v >> 16), (unsigned char)(v >> 8), (unsigned char)v }; fwrite(b, 1, 4, f); }
static void chunk(FILE *f, const char *type, const unsigned char *data, uint32_t len) {
    put32(f, len); fwrite(type, 1, 4, f); if (len) fwrite(data, 1, len, f);
    unsigned char *tmp = (unsigned char *)malloc(len + 4); memcpy(tmp, type, 4); if (len) memcpy(tmp + 4, data, len);
    put32(f, crc_update(0, tmp, len + 4)); free(tmp);
}
void png_save(const char *path, int scale) {
    int W = SW * scale, H = SH * scale;
    size_t rowlen = (size_t)W * 3 + 1, rawlen = rowlen * H;
    unsigned char *raw = (unsigned char *)malloc(rawlen);
    for (int y = 0; y < H; y++) {
        unsigned char *r = raw + y * rowlen; *r++ = 0;
        for (int x = 0; x < W; x++) { Col c = fb[(y / scale) * SW + (x / scale)]; *r++ = COL_R(c); *r++ = COL_G(c); *r++ = COL_B(c); }
    }
    size_t nblocks = (rawlen + 65534) / 65535;
    size_t zlen = 2 + rawlen + nblocks * 5 + 4;
    unsigned char *z = (unsigned char *)malloc(zlen); size_t zp = 0;
    z[zp++] = 0x78; z[zp++] = 0x01;
    uint32_t a = 1, b = 0;
    for (size_t i = 0; i < rawlen; i++) { a = (a + raw[i]) % 65521; b = (b + a) % 65521; }
    size_t off = 0;
    while (off < rawlen) {
        size_t n = rawlen - off; if (n > 65535) n = 65535;
        z[zp++] = (off + n >= rawlen) ? 1 : 0;
        z[zp++] = (unsigned char)(n & 255); z[zp++] = (unsigned char)(n >> 8);
        z[zp++] = (unsigned char)(~n & 255); z[zp++] = (unsigned char)((~n >> 8) & 255);
        memcpy(z + zp, raw + off, n); zp += n; off += n;
    }
    uint32_t ad = (b << 16) | a;
    z[zp++] = (unsigned char)(ad >> 24); z[zp++] = (unsigned char)(ad >> 16); z[zp++] = (unsigned char)(ad >> 8); z[zp++] = (unsigned char)ad;
    FILE *f = fopen(path, "wb"); if (!f) { free(raw); free(z); return; }
    static const unsigned char sig[8] = { 0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A };
    fwrite(sig, 1, 8, f);
    unsigned char ihdr[13] = { (unsigned char)(W >> 24), (unsigned char)(W >> 16), (unsigned char)(W >> 8), (unsigned char)W,
        (unsigned char)(H >> 24), (unsigned char)(H >> 16), (unsigned char)(H >> 8), (unsigned char)H, 8, 2, 0, 0, 0 };
    chunk(f, "IHDR", ihdr, 13); chunk(f, "IDAT", z, (uint32_t)zp); chunk(f, "IEND", NULL, 0);
    fclose(f); free(raw); free(z);
}
