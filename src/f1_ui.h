#ifndef F1_UI_H
#define F1_UI_H

#include "f1_car.h"
#include "f1_race.h"
#include "gfx.h"

typedef enum { F1_UI_HUB, F1_UI_CAR, F1_UI_DEVELOPMENT, F1_UI_RACE } F1UiScreen;

typedef struct {
    F1Game game;
    F1CarProgram car;
    F1Race race;
    int race_ready;
    float race_accumulator;
    int last_event_seen;
    int selected_car;
    int hovered_component;
    F1RaceEventType replay_type;
    int replay_frames;
    char replay_text[80];
} F1Ui;

void f1_ui_init(F1Ui *ui);
void f1_ui_render(F1Ui *ui, F1UiScreen screen, int mouse_x, int mouse_y);
void f1_ui_set_replay(F1Ui *ui, F1RaceEventType type, const char *caption, int frames);
void f1_ui_set_speed(F1Ui *ui, int speed);
void f1_ui_tick(F1Ui *ui, float seconds);

#endif
