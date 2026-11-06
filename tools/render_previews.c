#include "../src/f1_ui.h"

int main(void) {
    static const char *paths[] = {
        "previews/hub.png", "previews/car.png",
        "previews/development.png", "previews/race_replay.png"
    };
    F1Ui ui;
    gfx_init();
    f1_ui_init(&ui);
    for (int screen = F1_UI_HUB; screen <= F1_UI_RACE; ++screen) {
        if (screen == F1_UI_RACE)
            f1_ui_set_replay(&ui, F1_EVENT_OVERTAKE, "LEC ADELANTA A NOR POR EL INTERIOR", 120);
        f1_ui_render(&ui, (F1UiScreen)screen,
                     screen == F1_UI_CAR ? 270 : -1,
                     screen == F1_UI_CAR ? 130 : -1);
        png_save(paths[screen], 2);
    }
    {
        static const F1RaceEventType types[] = {F1_EVENT_PIT, F1_EVENT_SPIN, F1_EVENT_CRASH};
        static const char *captions[] = {"PARADA EN BOXES", "TROMPO EN EL SECTOR 2", "ACCIDENTE: SAFETY CAR"};
        static const char *extra[] = {"previews/replay_pit.png", "previews/replay_spin.png", "previews/replay_crash.png"};
        for (int i = 0; i < 3; ++i) {
            f1_ui_set_replay(&ui, types[i], captions[i], 120);
            f1_ui_render(&ui, F1_UI_RACE, -1, -1);
            png_save(extra[i], 2);
        }
    }
    return 0;
}
