#include "f1_ui.h"
#include "f1_ai.h"

#include <stdio.h>
#include <string.h>

typedef struct { int x, y, w, h, component; const char *name; } CarZone;

static const CarZone CAR_ZONES[] = {
    {224,112,88,22,F1_COMPONENT_FRONT_WING,"ALERON DEL."},
    {316,112,104,28,F1_COMPONENT_CHASSIS,"CHASIS"},
    {422,108,70,25,F1_COMPONENT_REAR_WING,"ALERON TRAS."},
    {266,137,178,24,F1_COMPONENT_FLOOR,"SUELO"},
    {330,91,58,18,F1_COMPONENT_ENGINE,"MOTOR"},
    {295,164,52,28,F1_COMPONENT_SUSPENSION,"SUSPENSION"},
    {400,164,52,28,F1_COMPONENT_BRAKES,"FRENOS"}
};

static void panel(int x, int y, int w, int h, const char *title) {
    rect(x, y, w, h, C_PANEL);
    box(x, y, w, h, C_PANEL3);
    rect(x, y, w, 17, C_PANEL2);
    text(x + 7, y + 5, title, C_TEXT);
}

static void draw_driver_face(int x, int y, const F1DriverDef *driver) {
    const F1TeamDef *team = f1_team_by_id(driver->team_id);
    unsigned int hash = (unsigned int)driver->number * 2654435761u;
    Col skin = (hash & 1u) ? RGB(212,151,108) : RGB(239,186,137);
    Col hair = (hash & 2u) ? RGB(55,34,27) : RGB(118,76,42);
    Col shirt = team ? team->color_primary : C_PANEL3;
    rect(x+5,y+3,18,18,skin);
    rect(x+3,y+8,3,9,skin); rect(x+23,y+8,3,9,skin);
    rect(x+5,y+2,18,5,hair); rect(x+5,y+5,4,5,hair);
    if (hash & 4u) rect(x+19,y+5,4,3,hair);
    rect(x+9,y+10,3,2,C_BLK); rect(x+17,y+10,3,2,C_BLK);
    px(x+14,y+14,col_dark(skin,35));
    rect(x+11,y+18,8,1,RGB(115,50,45));
    rect(x+8,y+21,12,3,skin);
    rect(x+3,y+24,23,8,shirt);
    rect(x+11,y+24,7,8,C_TEXT);
    text_c(x+14,y+34,driver->code,C_DIM);
}

static void topbar(const char *section) {
    rect(0, 0, SW, 28, RGB(8,12,26));
    rect(0, 27, SW, 1, C_RED);
    text(12, 7, "F1 PIXEL MANAGER", C_WHT);
    text(130, 7, section, C_YEL);
    text_r(628, 7, "TEMPORADA 2026", C_DIM);
}

static void nav(int active) {
    static const char *items[] = {"1  HUB", "2  COCHE", "3  DESARROLLO", "4  CARRERA"};
    for (int i = 0; i < 4; ++i) {
        Col c = i == active ? C_YEL : C_DIM;
        text(16 + i * 150, 342, items[i], c);
    }
}

static void draw_hub(F1Ui *ui) {
    (void)ui;
    topbar("CENTRO DE EQUIPO");
    panel(12, 38, 380, 116, "PROXIMO GRAN PREMIO");
    text(26, 63, "AUSTRALIA  /  ALBERT PARK", C_WHT);
    text(26, 81, "RONDA 1 DE 24", C_DIM);
    text(26, 103, "VIERNES", C_CYN); text(99, 103, "LIBRES", C_TEXT);
    text(26, 121, "SABADO", C_CYN); text(99, 121, "CLASIFICACION", C_TEXT);
    rect(270, 60, 102, 66, RGB(24,58,48));
    for (int i = 0; i < 9; ++i) line(282 + i * 9, 114 - (i&1)*18, 290 + i * 9, 78 + (i&1)*20, C_GRN);

    panel(402, 38, 226, 116, "ESTADO DEL EQUIPO");
    text(416, 64, "PRESUPUESTO", C_DIM); text_r(614, 64, "86,4 M", C_GRN);
    text(416, 84, "CONFIANZA", C_DIM); text_r(614, 84, "72%", C_YEL);
    text(416, 104, "COCHE", C_DIM); text_r(614, 104, "6 DE 11", C_TEXT);
    text(416, 124, "PILOTOS", C_DIM);
    {
        int shown = 0;
        for (int i = 0; i < F1_DRIVER_COUNT && shown < 2; ++i)
            if (strcmp(F1_DRIVERS[i].team_id, "ferrari") == 0)
                draw_driver_face(544 + shown++ * 34, 103, &F1_DRIVERS[i]);
    }

    panel(12, 164, 302, 164, "MENSAJES");
    text(25, 190, "INGENIERIA", C_CYN); text(105, 190, "Nuevo suelo listo", C_TEXT);
    text(25, 216, "PILOTOS", C_ORG); text(105, 216, "Reunion previa", C_TEXT);
    text(25, 242, "METEO", C_CYN); text(105, 242, "Riesgo de lluvia 18%", C_TEXT);
    text(25, 268, "DIRECTIVA", C_YEL); text(105, 268, "Objetivos de temporada", C_TEXT);
    text(25, 304, "Los eventos aparecen con moderacion.", C_DIM);

    panel(324, 164, 304, 164, "CAMPEONATO");
    for (int i = 0; i < 6; ++i) {
        textf(340, 190 + i*20, i < 3 ? C_TEXT : C_DIM, "%d", i+1);
        text(366, 190 + i*20, F1_TEAMS[i].name, i == 0 ? C_YEL : C_TEXT);
        text_r(610, 190 + i*20, "0", C_DIM);
    }
    nav(0);
}

static void draw_pixel_car(int x, int y, Col body, int scale) {
    rect(x + 8*scale, y + 4*scale, 26*scale, 5*scale, body);
    rect(x + 15*scale, y, 11*scale, 7*scale, body);
    rect(x + 2*scale, y + 7*scale, 38*scale, 3*scale, col_dark(body, 30));
    rect(x, y + 5*scale, 9*scale, 2*scale, body);
    rect(x + 34*scale, y + 2*scale, 7*scale, 7*scale, body);
    disc(x + 10*scale, y + 10*scale, 4*scale, C_BLK);
    disc(x + 32*scale, y + 10*scale, 4*scale, C_BLK);
    rect(x + 18*scale, y + scale, 6*scale, 3*scale, C_CYN);
}

static void draw_car(F1Ui *ui, int mx, int my) {
    topbar("MONOPLAZA  /  VISTA TECNICA");
    panel(12, 38, 606, 198, "F1-26  ESPECIFICACION A");
    ui->hovered_component = -1;
    for (unsigned i = 0; i < sizeof CAR_ZONES / sizeof CAR_ZONES[0]; ++i) {
        const CarZone *z = &CAR_ZONES[i];
        if (mx >= z->x && mx < z->x + z->w && my >= z->y && my < z->y + z->h)
            ui->hovered_component = z->component;
    }
    rect_dither(40, 76, 548, 134, RGB(18,25,46), RGB(20,28,52));
    draw_pixel_car(105, 105, RGB(220,28,38), 6);
    for (unsigned i = 0; i < sizeof CAR_ZONES / sizeof CAR_ZONES[0]; ++i) {
        const CarZone *z = &CAR_ZONES[i];
        Col c = ui->hovered_component == z->component ? C_YEL : C_PANEL3;
        box(z->x, z->y, z->w, z->h, c);
    }
    if (ui->hovered_component >= 0) {
        const char *name = "COMPONENTE";
        for (unsigned i = 0; i < sizeof CAR_ZONES / sizeof CAR_ZONES[0]; ++i)
            if (CAR_ZONES[i].component == ui->hovered_component) name = CAR_ZONES[i].name;
        rect(28, 188, 220, 30, RGB(8,12,24)); box(28,188,220,30,C_YEL);
        text(38, 198, name, C_YEL);
    }
    panel(12, 246, 294, 82, "RENDIMIENTO");
    text(26, 272, "CARGA AERO", C_DIM); rect(126,272,150,7,C_PANEL3); rect(126,272,108,7,C_CYN);
    text(26, 292, "VELOCIDAD", C_DIM); rect(126,292,150,7,C_PANEL3); rect(126,292,96,7,C_GRN);
    panel(316, 246, 302, 82, "LIMITES DE DESARROLLO");
    text(330, 272, "TECHO PRESUPUESTARIO", C_DIM); text_r(603,272,"71%",C_YEL);
    text(330, 292, "TUNEL / CFD (ATR)", C_DIM); text_r(603,292,"68%",C_CYN);
    text(330, 312, "CAPACIDAD DE FABRICA", C_DIM); text_r(603,312,"2 / 3",C_TEXT);
    nav(1);
}

static void draw_development(F1Ui *ui) {
    topbar("INVESTIGACION Y DESARROLLO");
    panel(12, 38, 388, 290, "COMPONENTES");
    for (int i = 0; i < F1_COMPONENT_COUNT; ++i) {
        int col = i / 6, row = i % 6, x = 26 + col * 185, y = 65 + row * 39;
        static const char *names[] = {"ALERON DEL.","ALERON TRAS.","SUELO","DIFUSOR","CHASIS","SUSPENSION","REFRIGERACION","FRENOS","CAMBIO","MOTOR","ERS","FIABILIDAD"};
        text(x,y,names[i],C_TEXT);
        rect(x,y+13,150,6,C_PANEL3);
        rect(x,y+13,ui->car.component[i].performance*3/2,6,i<4?C_CYN:C_GRN);
        textf(x+155,y,C_DIM,"%d",ui->car.component[i].performance);
    }
    panel(410, 38, 218, 140, "PROYECTO ACTIVO");
    text(424, 66, "NUEVO SUELO", C_YEL);
    text(424, 88, "18 DIAS RESTANTES", C_TEXT);
    rect(424,108,186,8,C_PANEL3); rect(424,108,119,8,C_CYN);
    text(424,130,"COSTE  2,8 M",C_DIM);
    text(424,150,"GANANCIA EST.  +3",C_GRN);
    panel(410, 188, 218, 140, "RESTRICCIONES");
    text(424,216,"PRESUPUESTO",C_DIM); text_r(612,216,"29,6 M",C_TEXT);
    text(424,240,"ATR",C_DIM); text_r(612,240,"68 H",C_CYN);
    text(424,264,"PROYECTOS",C_DIM); text_r(612,264,"1 / 3",C_YEL);
    text(424,294,"Rendimientos decrecientes",C_DIM);
    nav(2);
}

static void draw_track(void) {
    const F1CircuitDef *c = &F1_CIRCUITS[0];
    for (int i = 1; i < c->point_count; ++i) {
        int x0 = 282 + c->points[i-1].x * 238 / 1000;
        int y0 = 54 + c->points[i-1].y * 176 / 1000;
        int x1 = 282 + c->points[i].x * 238 / 1000;
        int y1 = 54 + c->points[i].y * 176 / 1000;
        line(x0,y0,x1,y1,C_LINE); line(x0+1,y0,x1+1,y1,C_TEXT);
    }
}

static void draw_replay(F1Ui *ui) {
    rect(78, 55, 484, 166, RGB(4,7,15));
    box(78, 55, 484, 166, C_YEL);
    rect(79,56,482,18,C_PANEL2);
    text(90,62,"REPETICION",C_YEL); text_r(548,62,"MOMENTO CLAVE",C_DIM);
    rect_dither(88,82,464,105,RGB(35,43,61),RGB(39,48,68));
    rect(88,157,464,30,RGB(52,55,62));
    line(88,167,552,167,C_WHT); line(88,177,552,177,C_WHT);
    draw_pixel_car(216,139,RGB(232,32,42),2);
    draw_pixel_car(306,125,RGB(255,126,22),2);
    text_c(320,199,ui->replay_text,C_WHT);
}

static void draw_race(F1Ui *ui) {
    char heading[96];
    snprintf(heading, sizeof heading, "CARRERA  /  VUELTA %d DE %d", ui->race.lap + 1, ui->race.laps);
    topbar(heading);
    panel(8, 36, 250, 292, "CLASIFICACION");
    for (int i = 0; i < 11; ++i) {
        int car_index = ui->race.order[i];
        const F1DriverDef *d = &F1_DRIVERS[ui->race.car[car_index].driver_index];
        textf(18,60+i*22,i<2?C_YEL:C_TEXT,"%2d",i+1);
        rect(42,61+i*22,4,8,(Col)f1_team_by_id(d->team_id)->color_primary);
        text(52,60+i*22,d->code,C_TEXT);
        if (i == 0) text_r(244,60+i*22,"LIDER",C_GRN);
        else textf(184,60+i*22,C_DIM,"+%.3f",(ui->race.car[car_index].total_time_ms-ui->race.car[ui->race.order[0]].total_time_ms)/1000.0f);
    }
    panel(268, 36, 364, 202, "TRAZADO Y SECTORES");
    draw_track();
    text(282,216,"S1",C_CYN); text(322,216,"S2",C_YEL); text(362,216,"S3",C_ORG);
    panel(268, 246, 364, 82, "ESTRATEGIA");
    {
        F1RaceCar *selected = &ui->race.car[ui->selected_car];
        const F1DriverDef *driver = &F1_DRIVERS[selected->driver_index];
        static const char *tyres[] = {"BLANDO","MEDIO","DURO","INTER","LLUVIA"};
        static const char *paces[] = {"CONSERVAR","NORMAL","ATACAR"};
        text(282,272,driver->code,C_TEXT);
        textf(322,272,C_YEL,"%s  %dV",tyres[selected->compound],selected->tyre_age_laps);
        textf(430,272,C_TEXT,"RITMO %s",paces[selected->pace]);
    }
    text(282,294,"ERS",C_DIM); rect(322,295,90,6,C_PANEL3); rect(322,295,66,6,C_GRN);
    text(430,294,"PARADA  V18-22",C_CYN);
    textf(282,312,C_DIM,"[ESPACIO] PAUSA   VELOCIDAD  x%d",ui->race.speed);
    nav(3);
    if (ui->replay_frames > 0) { draw_replay(ui); --ui->replay_frames; }
}

void f1_ui_init(F1Ui *ui) {
    if (!ui) return;
    memset(ui, 0, sizeof *ui);
    f1_game_new(&ui->game, "ferrari", 2026u);
    f1_car_program_new(&ui->car, 86400000, 100);
    f1_start_weekend(&ui->game);
    f1_run_qualifying(&ui->game);
    if (f1_race_setup(&ui->race, &ui->game, 0, F1_CIRCUITS[0].laps) == 0) {
        f1_race_set_team_performance(&ui->race, ui->game.player_team_id, f1_car_overall(&ui->car));
        ui->race_ready = 1;
        for (int i = 0; i < F1_DRIVER_COUNT; ++i)
            if (strcmp(F1_DRIVERS[ui->race.car[i].driver_index].team_id, ui->game.player_team_id) == 0) {
                ui->selected_car = i;
                break;
            }
    }
    ui->hovered_component = -1;
}

void f1_ui_set_speed(F1Ui *ui, int speed) {
    if (ui && ui->race_ready) f1_race_set_speed(&ui->race, speed);
}

void f1_ui_tick(F1Ui *ui, float seconds) {
    float interval;
    if (!ui || !ui->race_ready || ui->race.finished || ui->race.speed == 0 || seconds <= 0.0f) return;
    ui->race_accumulator += seconds;
    interval = 5.0f / ui->race.speed;
    while (ui->race_accumulator >= interval && !ui->race.finished) {
        ui->race_accumulator -= interval;
        f1_ai_update_all(&ui->race, ui->game.player_team_id, 0.82f);
        f1_race_step(&ui->race);
        if (ui->race.event_count > ui->last_event_seen) {
            const F1RaceEvent *event = &ui->race.events[ui->last_event_seen++];
            if (event->replay)
                f1_ui_set_replay(ui, event->type, event->text, 120);
        }
    }
}

void f1_ui_set_replay(F1Ui *ui, F1RaceEventType type, const char *caption, int frames) {
    if (!ui || frames < 0) return;
    ui->replay_type = type;
    ui->replay_frames = frames;
    strncpy(ui->replay_text, caption ? caption : "", sizeof ui->replay_text - 1);
    ui->replay_text[sizeof ui->replay_text - 1] = 0;
}

void f1_ui_render(F1Ui *ui, F1UiScreen screen, int mouse_x, int mouse_y) {
    if (!ui) return;
    clear(C_BG);
    if (screen == F1_UI_HUB) draw_hub(ui);
    else if (screen == F1_UI_CAR) draw_car(ui, mouse_x, mouse_y);
    else if (screen == F1_UI_DEVELOPMENT) draw_development(ui);
    else draw_race(ui);
}
