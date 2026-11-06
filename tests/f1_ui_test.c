#include <assert.h>
#include <stdio.h>
#include <string.h>

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

static void test_other_player_driver_keeps_ai_strategy(void) {
    F1Ui ui;
    int other = -1;
    f1_ui_init(&ui);
    for (int i = 0; i < F1_DRIVER_COUNT; ++i) {
        const F1DriverDef *d = &F1_DRIVERS[ui.race.car[i].driver_index];
        if (strcmp(d->team_id, ui.game.player_team_id) == 0 && i != ui.selected_car) other = i;
    }
    assert(other >= 0);
    ui.race.track_water = 0.8f;
    ui.race.rain = 0.9f;
    ui.race.car[other].compound = F1_TYRE_MEDIUM;
    f1_ui_set_speed(&ui, 1);
    f1_ui_tick(&ui, 5.1f);
    assert(ui.race.car[other].pit_requested);
    assert(ui.race.car[other].requested_compound == F1_TYRE_WET);
}

static void test_finished_race_advances_to_next_round(void) {
    F1Ui ui;
    f1_ui_init(&ui);
    ui.race.laps = 1;
    f1_ui_set_speed(&ui, 16);
    f1_ui_tick(&ui, 1.1f);
    assert(ui.game.round_index == 1);
    assert(ui.race.circuit_index != 0);
    assert(!ui.race.finished);
}

static void test_player_can_switch_between_both_cars(void) {
    F1Ui ui;
    int first;
    f1_ui_init(&ui);
    first = ui.selected_car;
    f1_ui_select_other_car(&ui);
    assert(ui.selected_car != first);
    assert(strcmp(F1_DRIVERS[ui.race.car[ui.selected_car].driver_index].team_id,
                  ui.game.player_team_id) == 0);
    f1_ui_select_other_car(&ui);
    assert(ui.selected_car == first);
}

static void test_ui_save_restores_career_race_and_team(void) {
    F1Ui ui;
    f1_ui_init(&ui);
    ui.game.year = 2031;
    ui.race.lap = 17;
    ui.team.cash = 1234567;
    assert(f1_ui_save(&ui, "build/ui_save.sav") == F1_SAVE_OK);
    ui.game.year = 2026;
    ui.race.lap = 0;
    ui.team.cash = 0;
    assert(f1_ui_load(&ui, "build/ui_save.sav") == F1_SAVE_OK);
    assert(ui.game.year == 2031);
    assert(ui.race.lap == 17);
    assert(ui.team.cash == 1234567);
    remove("build/ui_save.sav");
}

static void test_hub_can_start_career_with_another_real_team(void) {
    F1Ui ui;
    f1_ui_init(&ui);
    assert(f1_ui_new_career(&ui, "mclaren") == 0);
    assert(strcmp(ui.game.player_team_id, "mclaren") == 0);
    for (int i = 0; i < F1_DRIVER_COUNT; ++i)
        if (strcmp(F1_DRIVERS[ui.race.car[i].driver_index].team_id, "mclaren") == 0) {
            assert(ui.race.car[i].car_performance == f1_car_overall(&ui.car));
            return;
        }
    assert(!"mclaren drivers missing");
}

static void test_development_action_uses_limits(void) {
    F1Ui ui;
    int slot;
    f1_ui_init(&ui);
    slot = f1_ui_start_project(&ui, F1_COMPONENT_FLOOR);
    assert(slot >= 0);
    assert(ui.car.budget < 86400000);
    assert(ui.car.atr_hours < 100);
}

int main(void) {
    gfx_init();
    test_all_primary_screens_render();
    test_car_hover_selects_component();
    test_replay_overlay_occupies_foreground();
    test_race_tick_respects_pause_and_speed();
    test_other_player_driver_keeps_ai_strategy();
    test_finished_race_advances_to_next_round();
    test_player_can_switch_between_both_cars();
    test_ui_save_restores_career_race_and_team();
    test_hub_can_start_career_with_another_real_team();
    test_development_action_uses_limits();
    puts("f1_ui_test: OK");
    return 0;
}
