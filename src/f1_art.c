#include "f1_art.h"
#include "gfx.h"

#include <string.h>

static unsigned int hash_id(const char *s) {
    unsigned int h = 2166136261u;
    while (*s) { h ^= (unsigned char)*s++; h *= 16777619u; }
    return h ^ (h >> 15);
}

static Col from_hex(unsigned int c) { return RGB((c >> 16) & 255, (c >> 8) & 255, c & 255); }

static int luminance(Col c) { return (COL_R(c) * 3 + COL_G(c) * 6 + COL_B(c)) / 10; }

void f1_art_driver_portrait(int x, int y, const F1DriverDef *d) {
    static const Col SKIN[] = {
        RGB(250,214,184), RGB(239,186,137), RGB(212,151,108),
        RGB(176,118,80),  RGB(130,86,58),   RGB(92,60,42)
    };
    static const Col HAIR[] = {
        RGB(24,20,20), RGB(55,34,27), RGB(118,76,42),
        RGB(176,128,64), RGB(204,170,102), RGB(120,120,124)
    };
    const F1TeamDef *team = f1_team_by_id(d->team_id);
    unsigned int h = hash_id(d->id);
    Col skin = SKIN[h % 6u], hair = HAIR[(h >> 4) % 6u];
    Col shirt = team ? from_hex(team->color_primary) : C_PANEL3;
    Col trim = team ? from_hex(team->color_secondary) : C_TEXT;
    int style = (int)((h >> 8) % 4u);   /* 0 corto, 1 largo, 2 rapado, 3 tupe */
    int beard = (int)((h >> 12) % 4u) == 0;
    int glasses = (int)((h >> 14) % 7u) == 0;
    Col shade = col_dark(skin, 35);

    rect(x, y, F1_PORTRAIT_W, F1_PORTRAIT_H, col_mix(C_PANEL2, shirt, 70));
    box(x, y, F1_PORTRAIT_W, F1_PORTRAIT_H, C_LINE_D);
    /* hombros y cuello */
    rect(x + 3, y + 24, 24, 8, shirt);
    rect(x + 3, y + 24, 24, 1, col_light(shirt, 40));
    rect(x + 12, y + 24, 6, 8, trim);
    rect(x + 11, y + 20, 8, 5, shade);
    /* cabeza y orejas */
    rect(x + 8, y + 6, 14, 15, skin);
    rect(x + 7, y + 11, 1, 5, shade); rect(x + 22, y + 11, 1, 5, shade);
    /* pelo */
    if (style == 2) { rect(x + 8, y + 6, 14, 2, col_mix(skin, hair, 90)); }
    else {
        rect(x + 8, y + 4, 14, 4, hair);
        rect(x + 7, y + 6, 2, 5, hair); rect(x + 21, y + 6, 2, 5, hair);
        if (style == 1) { rect(x + 6, y + 8, 3, 10, hair); rect(x + 21, y + 8, 3, 10, hair); }
        if (style == 3) rect(x + 10, y + 3, 11, 2, hair);
    }
    /* cara */
    if (glasses) {
        box(x + 9, y + 11, 5, 4, C_BLK); box(x + 16, y + 11, 5, 4, C_BLK);
        rect(x + 14, y + 12, 2, 1, C_BLK);
    }
    rect(x + 10, y + 12, 3, 2, C_WHT); rect(x + 17, y + 12, 3, 2, C_WHT);
    px(x + 11, y + 12, C_BLK); px(x + 11, y + 13, C_BLK);
    px(x + 18, y + 12, C_BLK); px(x + 18, y + 13, C_BLK);
    rect(x + 9, y + 10, 5, 1, col_mix(hair, skin, 60));
    rect(x + 16, y + 10, 5, 1, col_mix(hair, skin, 60));
    px(x + 15, y + 15, shade); px(x + 15, y + 16, shade);
    rect(x + 12, y + 18, 6, 1, RGB(138,64,58));
    if (beard) {
        Col b = col_mix(hair, skin, 40);
        rect(x + 9, y + 17, 12, 3, b); rect(x + 12, y + 18, 6, 1, RGB(138,64,58));
        rect(x + 8, y + 14, 2, 4, b); rect(x + 20, y + 14, 2, 4, b);
    }
    /* marca de color del equipo en la esquina del retrato */
    rect(x + 1, y + 1, 4, 3, from_hex(team ? team->color_primary : 0x888888u));
}

void f1_art_team_logo(int x, int y, const F1TeamDef *t) {
    Col a = from_hex(t->color_primary), b = from_hex(t->color_secondary);
    Col ink = luminance(a) > 150 ? C_BLK : C_WHT;
    int motif = (int)(hash_id(t->id) % 5u);
    int i;
    rect(x, y, F1_LOGO_W, F1_LOGO_H, a);
    switch (motif) {
    case 0: rect(x, y + F1_LOGO_H - 5, F1_LOGO_W, 5, b); break;                /* franja inferior */
    case 1: for (i = 0; i < F1_LOGO_H; ++i) rect(x + 20 + i / 3, y + i, 4, 1, b); break;   /* diagonal */
    case 2: rect(x, y, 4, F1_LOGO_H, b); rect(x + F1_LOGO_W - 4, y, 4, F1_LOGO_H, b); break;
    case 3: for (i = 0; i < F1_LOGO_W; i += 4) rect(x + i, y, 2, 3, b); break;      /* almenas */
    default: rect(x, y, F1_LOGO_W, 3, b); break;
    }
    box(x, y, F1_LOGO_W, F1_LOGO_H, C_LINE_D);
    if (text_w(t->code) <= F1_LOGO_W - 2)
        text_c(x + F1_LOGO_W / 2, y + 6, t->code, ink);
}
