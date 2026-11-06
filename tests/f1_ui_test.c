#include <assert.h>
#include <stdio.h>

#include "../src/f1_ui.h"

static int changed_pixels(void) {
    int n = 0;
    for (int i = 0; i < SW * SH; ++i) if (fb[i] != C_BG) ++n;
    return n;
}

static void test_all_primary_screens_render(void) {
    F1Ui ui;
    f1_ui_init(&ui);
    for (int screen = F1_UI_HUB; screen <= F1_UI_RACE; ++screen) {
        clear(C_BG);
        f1_ui_render(&ui, (F1UiScreen)screen, 320, 180);
        assert(changed_pixels() > 10000);
    }
}

static void test_car_hover_selects_component(void) {
    F1Ui ui;
    f1_ui_init(&ui);
    f1_ui_render(&ui, F1_UI_CAR, 270, 130);
    assert(ui.hovered_component >= 0);
    assert(ui.hovered_component < F1_COMPONENT_COUNT);
}

static void test_replay_overlay_occupies_foreground(void) {
    F1Ui ui;
    Col before;
    f1_ui_init(&ui);
    f1_ui_render(&ui, F1_UI_RACE, -1, -1);
    before = getpx(320, 110);
    f1_ui_set_replay(&ui, F1_EVENT_OVERTAKE, "LEC adelanta a NOR", 120);
    f1_ui_render(&ui, F1_UI_RACE, -1, -1);
    assert(ui.replay_frames == 119);
    assert(getpx(320, 110) != before);
}

static void test_race_tick_respects_pause_and_speed(void) {
    F1Ui ui;
    f1_ui_init(&ui);
    assert(ui.race_ready);
    assert(ui.race.sector == 0);
    f1_ui_set_speed(&ui, 0);
    f1_ui_tick(&ui, 20.0f);
    assert(ui.race.sector == 0);
    f1_ui_set_speed(&ui, 1);
    f1_ui_tick(&ui, 5.1f);
    assert(ui.race.sector == 1);
    f1_ui_set_speed(&ui, 16);
    f1_ui_tick(&ui, 1.0f);
    assert(ui.race.sector != 1 || ui.race.lap > 0);
}

int main(void) {
    gfx_init();
    test_all_primary_screens_render();
    test_car_hover_selects_component();
    test_replay_overlay_occupies_foreground();
    test_race_tick_respects_pause_and_speed();
    puts("f1_ui_test: OK");
    return 0;
}
