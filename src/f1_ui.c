#include "f1_ui.h"
#include "f1_ai.h"
#include "f1_art.h"

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

static int circuit_index_for_round(int round_index) {
    int i;
    const char *id = F1_ROUNDS[round_index].circuit_id;
    for (i = 0; i < F1_CIRCUIT_COUNT; ++i)
        if (strcmp(F1_CIRCUITS[i].id, id) == 0) return i;
    return 0;
}

static int setup_current_race(F1Ui *ui, int speed) {
    int circuit = circuit_index_for_round(ui->game.round_index);
    if (f1_start_weekend(&ui->game) != 0 || f1_run_qualifying(&ui->game) != 0 ||
        f1_race_setup(&ui->race, &ui->game, circuit, F1_CIRCUITS[circuit].laps) != 0) return 0;
    f1_race_set_team_performance(&ui->race, ui->game.player_team_id, f1_car_overall(&ui->car));
    f1_race_set_speed(&ui->race, speed);
    ui->race_ready = 1;
    ui->last_event_seen = 0;
    for (int i = 0; i < F1_DRIVER_COUNT; ++i)
        if (strcmp(F1_DRIVERS[ui->race.car[i].driver_index].team_id, ui->game.player_team_id) == 0) {
            ui->selected_car = i;
            break;
        }
    return 1;
}

static void complete_and_advance_race(F1Ui *ui) {
    short result[F1_MAX_DRIVERS];
    int speed = ui->race.speed;
    for (int i = 0; i < F1_DRIVER_COUNT; ++i)
        result[i] = ui->race.car[ui->race.order[i]].driver_index;
    if (f1_complete_weekend(&ui->game, result, F1_DRIVER_COUNT) == 0) {
        f1_car_advance_days(&ui->car, 14);
        f1_team_roll_event(&ui->team, ui->game.round_index * 14, 0);
        setup_current_race(ui, speed);
    }
}

static void panel(int x, int y, int w, int h, const char *title) {
    rect(x, y, w, h, C_PANEL);
    box(x, y, w, h, C_PANEL3);
    rect(x, y, w, 17, C_PANEL2);
    text(x + 7, y + 5, title, C_TEXT);
}

static void draw_driver_face(int x, int y, const F1DriverDef *driver) {
    f1_art_driver_portrait(x, y, driver);
}

static int team_slot(const char *id) {
    for (int i = 0; i < F1_TEAM_COUNT; ++i) if (strcmp(F1_TEAMS[i].id, id) == 0) return i;
    return -1;
}

static void sorted_team_order(const F1Ui *ui, int *order) {
    for (int i = 0; i < F1_TEAM_COUNT; ++i) order[i] = i;
    for (int i = 1; i < F1_TEAM_COUNT; ++i) {
        int key = order[i], j = i - 1;
        while (j >= 0 && ui->game.team_points[order[j]] < ui->game.team_points[key]) { order[j + 1] = order[j]; --j; }
        order[j + 1] = key;
    }
}

static int ui_year = 2026;

static void topbar(const char *section) {
    rect(0, 0, SW, 28, RGB(8,12,26));
    rect(0, 27, SW, 1, C_RED);
    text(12, 7, "F1 PIXEL MANAGER", C_WHT);
    text(130, 7, section, C_YEL);
    { char y[32]; snprintf(y, sizeof y, "TEMPORADA %d", ui_year); text_r(628, 7, y, C_DIM); }
}

static void nav(int active) {
    static const char *items[] = {"1  HUB", "2  COCHE", "3  DESARROLLO", "4  CARRERA"};
    for (int i = 0; i < 4; ++i) {
        Col c = i == active ? C_YEL : C_DIM;
        text(16 + i * 150, 342, items[i], c);
    }
}

static void draw_circuit_shape(const F1CircuitDef *c, int x, int y, int w, int h, Col col) {
    for (int i = 1; i < c->point_count; ++i) {
        int x0 = x + c->points[i-1].x * w / 1000, y0 = y + c->points[i-1].y * h / 1000;
        int x1 = x + c->points[i].x * w / 1000, y1 = y + c->points[i].y * h / 1000;
        line(x0, y0, x1, y1, col); line(x0 + 1, y0, x1 + 1, y1, col);
    }
}

static void draw_hub(F1Ui *ui) {
    const F1RoundDef *round = &F1_ROUNDS[ui->game.round_index];
    const F1CircuitDef *circuit = f1_circuit_by_id(round->circuit_id);
    const F1TeamDef *mine = f1_team_by_id(ui->game.player_team_id);
    char line_text[96];
    int order[F1_MAX_TEAMS], mine_pos = 0;
    ui_year = ui->game.year;
    topbar("CENTRO DE EQUIPO");
    panel(12, 38, 380, 116, "PROXIMO GRAN PREMIO");
    snprintf(line_text, sizeof line_text, "%s", round->name);
    text_fit(26, 63, 230, line_text, C_WHT);
    text_fit(26, 81, 230, circuit ? circuit->name : "-", C_DIM);
    textf(26, 99, C_DIM, "RONDA %d DE %d%s", ui->game.round_index + 1, F1_ROUND_COUNT, round->sprint ? "  SPRINT" : "");
    textf(26, 117, C_CYN, "%d VUELTAS   %.3f KM", circuit ? circuit->laps : 0, circuit ? circuit->length_km : 0.0f);
    text(26, 135, "[4] IR A LA CARRERA", C_YEL);
    rect(262, 58, 122, 90, RGB(24,58,48));
    if (circuit) draw_circuit_shape(circuit, 270, 62, 100, 80, C_GRN);

    panel(402, 38, 226, 116, "ESTADO DEL EQUIPO");
    if (mine) f1_art_team_logo(414, 60, mine);
    if (mine) text_fit(446, 64, 170, mine->name, C_WHT);
    text(416, 86, "PRESUPUESTO", C_DIM); textf(614 - 8 * 7, 86, C_GRN, "%.1f M", ui->team.cash / 1000000.0f);
    text(416, 104, "COCHE", C_DIM); textf(614 - 8 * 4, 104, C_TEXT, "%d", f1_car_overall(&ui->car));
    text(416, 122, "PILOTOS", C_DIM);
    {
        int shown = 0;
        for (int i = 0; i < F1_DRIVER_COUNT && shown < 2; ++i)
            if (strcmp(F1_DRIVERS[i].team_id, ui->game.player_team_id) == 0)
                draw_driver_face(530 + shown++ * 40, 114, &F1_DRIVERS[i]);
    }

    panel(12, 164, 302, 164, "MENSAJES");
    text(25, 190, "INGENIERIA", C_CYN);
    {
        int ready = 0, active = 0;
        for (int i = 0; i < F1_CAR_PROJECTS; ++i) { ready += ui->car.project[i].complete; active += ui->car.project[i].active; }
        textf(105, 190, C_TEXT, ready ? "%d pieza(s) lista(s)" : "%d proyecto(s) activo(s)", ready ? ready : active);
    }
    text(25, 216, "METEO", C_CYN);
    textf(105, 216, C_TEXT, "Riesgo de lluvia %d%%", circuit ? (int)(circuit->rain_probability * 100.0f + 0.5f) : 0);
    text(25, 242, "DIRECTIVA", C_YEL);
    text_fit(105, 242, 200, ui->team.last_event[0] ? ui->team.last_event : "Sin novedades", C_TEXT);
    text(25, 268, "AYUDA", C_ORG); text(105, 268, "F5 guardar   F9 cargar", C_TEXT);
    text(25, 304, "[ ] cambia de escuderia", C_DIM);

    panel(324, 164, 304, 164, "CAMPEONATO DE CONSTRUCTORES");
    sorted_team_order(ui, order);
    for (int i = 0; i < F1_TEAM_COUNT; ++i) if (order[i] == team_slot(ui->game.player_team_id)) mine_pos = i;
    for (int i = 0; i < 6; ++i) {
        int row = i, t = order[i];
        if (i == 5 && mine_pos > 4) { row = 5; t = order[mine_pos]; }
        else if (i == 5) t = order[5];
        {
            int mine_row = t == team_slot(ui->game.player_team_id);
            Col c = mine_row ? C_YEL : C_TEXT;
            textf(334, 188 + row * 22, c, "%2d", (i == 5 && mine_pos > 4) ? mine_pos + 1 : i + 1);
            f1_art_team_logo(358, 183 + row * 22, &F1_TEAMS[t]);
            text_fit(392, 188 + row * 22, 160, F1_TEAMS[t].name, c);
            textf(600 - 8 * 4, 188 + row * 22, c, "%4d", ui->game.team_points[t]);
        }
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
    {
        const F1TeamDef *tm = f1_team_by_id(ui->game.player_team_id);
        draw_pixel_car(105, 105, tm ? RGB((tm->color_primary>>16)&255,(tm->color_primary>>8)&255,tm->color_primary&255) : RGB(220,28,38), 6);
    }
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
    {
        int aero = (ui->car.component[F1_COMPONENT_FRONT_WING].performance + ui->car.component[F1_COMPONENT_REAR_WING].performance +
                    ui->car.component[F1_COMPONENT_FLOOR].performance + ui->car.component[F1_COMPONENT_DIFFUSER].performance) / 4;
        int power = (ui->car.component[F1_COMPONENT_ENGINE].performance + ui->car.component[F1_COMPONENT_ERS].performance) / 2;
        text(26, 272, "CARGA AERO", C_DIM); rect(126,272,150,7,C_PANEL3); rect(126,272,aero*3/2,7,C_CYN);
        text(26, 292, "POTENCIA", C_DIM); rect(126,292,150,7,C_PANEL3); rect(126,292,power*3/2,7,C_GRN);
    }
    panel(316, 246, 302, 82, "LIMITES DE DESARROLLO");
    textf(330, 272, C_DIM, "PRESUPUESTO"); textf(603-8*7,272,C_YEL,"%.1f M",ui->team.cash / 1000000.0f);
    text(330, 292, "TUNEL / CFD (ATR)", C_DIM); textf(603-8*5,292,C_CYN,"%d H",ui->car.atr_hours);
    text(330, 312, "PROYECTOS ACTIVOS", C_DIM); textf(603-8*3,312,C_TEXT,"%d/%d",ui->car.active_projects,F1_CAR_PROJECTS);
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
    panel(410, 38, 218, 140, "PROYECTOS");
    {
        static const char *names[] = {"ALERON DEL.","ALERON TRAS.","SUELO","DIFUSOR","CHASIS","SUSPENSION","REFRIGERACION","FRENOS","CAMBIO","MOTOR","ERS","FIABILIDAD"};
        int shown = 0;
        for (int i = 0; i < F1_CAR_PROJECTS; ++i) {
            const F1CarProject *p = &ui->car.project[i];
            if (!p->active) continue;
            text(424, 62 + shown * 38, names[p->component], p->complete ? C_GRN : C_YEL);
            rect(424, 78 + shown * 38, 186, 6, C_PANEL3);
            rect(424, 78 + shown * 38, 186 * p->days_done / (p->days_total ? p->days_total : 1), 6, p->complete ? C_GRN : C_CYN);
            if (p->complete) text_r(612, 62 + shown * 38, "LISTO (INTRO)", C_GRN);
            else textf(612 - 8 * 10, 62 + shown * 38, C_DIM, "+%d  %dD", p->gain, p->days_total - p->days_done);
            ++shown;
        }
        if (!shown) { text(424, 66, "SIN PROYECTOS", C_DIM); text(424, 88, "[N] INICIAR SUELO", C_YEL); }
    }
    panel(410, 188, 218, 140, "RESTRICCIONES");
    text(424,216,"PRESUPUESTO",C_DIM); textf(612 - 8 * 7,216,C_TEXT,"%.1f M",ui->team.cash / 1000000.0f);
    text(424,240,"ATR",C_DIM); textf(612 - 8 * 5,240,C_CYN,"%d H",ui->car.atr_hours);
    text(424,264,"PROYECTOS",C_DIM); textf(612 - 8 * 3,264,C_YEL,"%d/%d",ui->car.active_projects,F1_CAR_PROJECTS);
    text(424,294,"Rendimientos decrecientes",C_DIM);
    nav(2);
}

static void draw_track(const F1Race *race) {
    const F1CircuitDef *c = &F1_CIRCUITS[race->circuit_index];
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
    draw_track(&ui->race);
    for (int i = 0; i < F1_DRIVER_COUNT; ++i) {
        const F1RaceCar *rc = &ui->race.car[i];
        const F1CircuitDef *cc = &F1_CIRCUITS[ui->race.circuit_index];
        const F1DriverDef *dd = &F1_DRIVERS[rc->driver_index];
        const F1TeamDef *tt = f1_team_by_id(dd->team_id);
        int seg = (rc->sectors_completed % 3) * (cc->point_count - 1) / 3 + (cc->point_count - 1) / 6;
        int px_ = 282 + cc->points[seg].x * 238 / 1000, py_ = 54 + cc->points[seg].y * 176 / 1000;
        if (rc->retired) continue;
        disc(px_, py_, i == ui->selected_car ? 3 : 2, i == ui->selected_car ? C_WHT : (Col)tt->color_primary);
    }
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
    {
        const F1RaceCar *sel = &ui->race.car[ui->selected_car];
        text(282,294,"ERS",C_DIM); rect(322,295,90,6,C_PANEL3); rect(322,295,(int)(sel->ers_mj * 90.0f / 4.0f),6,C_GRN);
        if (sel->retired) text(430,294,"RETIRADO",C_RED);
        else if (sel->pit_requested) textf(430,294,C_CYN,"PARADA: %s",sel->requested_compound == F1_TYRE_SOFT ? "BLANDO" : sel->requested_compound == F1_TYRE_MEDIUM ? "MEDIO" : sel->requested_compound == F1_TYRE_HARD ? "DURO" : sel->requested_compound == F1_TYRE_INTERMEDIATE ? "INTER" : "LLUVIA");
        else text(430,294,"[P] PEDIR PARADA",C_CYN);
    }
    textf(282,312,C_DIM,"[ESPACIO] PAUSA   VELOCIDAD  x%d",ui->race.speed);
    nav(3);
    if (ui->replay_frames > 0) { draw_replay(ui); --ui->replay_frames; }
}

void f1_ui_init(F1Ui *ui) {
    if (!ui) return;
    f1_ui_new_career(ui, "ferrari");
}

int f1_ui_new_career(F1Ui *ui, const char *team_id) {
    if (!ui || !f1_team_by_id(team_id)) return 1;
    memset(ui, 0, sizeof *ui);
    f1_game_new(&ui->game, team_id, 2026u);
    f1_car_program_new(&ui->car, 86400000, 100);
    f1_team_management_new(&ui->team, 86400000, 2027u);
    setup_current_race(ui, 1);
    ui->hovered_component = -1;
    return 0;
}

void f1_ui_cycle_team(F1Ui *ui, int direction) {
    int i, current = 0;
    if (!ui || direction == 0) return;
    for (i = 0; i < F1_TEAM_COUNT; ++i)
        if (strcmp(F1_TEAMS[i].id, ui->game.player_team_id) == 0) { current = i; break; }
    current = (current + (direction > 0 ? 1 : F1_TEAM_COUNT - 1)) % F1_TEAM_COUNT;
    f1_ui_new_career(ui, F1_TEAMS[current].id);
}

int f1_ui_start_project(F1Ui *ui, F1ComponentType component) {
    int cost, slot;
    if (!ui || component < 0 || component >= F1_COMPONENT_COUNT) return F1_CAR_ERR_INVALID;
    cost = 900000 + component * 50000;
    if (ui->team.cash < cost) return F1_CAR_ERR_BUDGET;
    slot = f1_car_start_project(&ui->car, component, 30, cost, 10);
    if (slot >= 0) ui->team.cash -= cost;
    return slot;
}

F1SaveResult f1_ui_save(const F1Ui *ui, const char *path) {
    F1SaveState state;
    if (!ui) return F1_SAVE_ERR_STATE;
    memset(&state, 0, sizeof state);
    state.game = ui->game;
    state.car = ui->car;
    state.team = ui->team;
    state.race = ui->race;
    state.race_active = (unsigned char)(ui->race_ready ? 1 : 0);
    return f1_save_write(path, &state);
}

F1SaveResult f1_ui_load(F1Ui *ui, const char *path) {
    F1SaveState state;
    F1SaveResult result;
    if (!ui) return F1_SAVE_ERR_STATE;
    result = f1_save_read(path, &state);
    if (result != F1_SAVE_OK) return result;
    ui->game = state.game;
    ui->car = state.car;
    ui->team = state.team;
    ui->race = state.race;
    ui->race_ready = state.race_active;
    ui->race_accumulator = 0.0f;
    ui->last_event_seen = ui->race.event_count;
    ui->selected_car = 0;
    for (int i = 0; i < F1_DRIVER_COUNT; ++i)
        if (strcmp(F1_DRIVERS[ui->race.car[i].driver_index].team_id, ui->game.player_team_id) == 0) {
            ui->selected_car = i;
            break;
        }
    return F1_SAVE_OK;
}

void f1_ui_set_speed(F1Ui *ui, int speed) {
    if (ui && ui->race_ready) f1_race_set_speed(&ui->race, speed);
}

void f1_ui_select_other_car(F1Ui *ui) {
    if (!ui || !ui->race_ready) return;
    for (int offset = 1; offset <= F1_DRIVER_COUNT; ++offset) {
        int candidate = (ui->selected_car + offset) % F1_DRIVER_COUNT;
        const F1DriverDef *driver = &F1_DRIVERS[ui->race.car[candidate].driver_index];
        if (strcmp(driver->team_id, ui->game.player_team_id) == 0) {
            ui->selected_car = candidate;
            return;
        }
    }
}

void f1_ui_tick(F1Ui *ui, float seconds) {
    float interval;
    if (!ui || !ui->race_ready || ui->race.finished || ui->race.speed == 0 || seconds <= 0.0f) return;
    ui->race_accumulator += seconds;
    interval = 5.0f / ui->race.speed;
    while (ui->race_accumulator >= interval && !ui->race.finished) {
        ui->race_accumulator -= interval;
        for (int i = 0; i < F1_DRIVER_COUNT; ++i) {
            const F1DriverDef *driver = &F1_DRIVERS[ui->race.car[i].driver_index];
            if (strcmp(driver->team_id, ui->game.player_team_id) != 0 || i != ui->selected_car)
                f1_ai_update(&ui->race, i, 0.82f);
        }
        f1_race_step(&ui->race);
        if (ui->race.event_count > ui->last_event_seen) {
            const F1RaceEvent *event = &ui->race.events[ui->last_event_seen++];
            if (event->replay)
                f1_ui_set_replay(ui, event->type, event->text, 120);
        }
        if (ui->race.finished) complete_and_advance_race(ui);
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
