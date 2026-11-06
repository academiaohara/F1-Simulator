#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/f1_ui.h"
#include "../src/f1_art.h"

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

static void test_selected_driver_accepts_pace_and_ers_orders(void) {
    F1Ui ui;
    f1_ui_init(&ui);
    f1_ui_set_selected_strategy(&ui, F1_PACE_PUSH, F1_ERS_DEPLOY);
    assert(ui.race.car[ui.selected_car].pace == F1_PACE_PUSH);
    assert(ui.race.car[ui.selected_car].ers_mode == F1_ERS_DEPLOY);
    f1_ui_set_selected_strategy(&ui, F1_PACE_CONSERVE, F1_ERS_HARVEST);
    assert(ui.race.car[ui.selected_car].pace == F1_PACE_CONSERVE);
    assert(ui.race.car[ui.selected_car].ers_mode == F1_ERS_HARVEST);
}

static unsigned long long replay_hash(F1RaceEventType type) {
    F1Ui ui;
    unsigned long long h = 1469598103934665603ull;
    f1_ui_init(&ui);
    f1_ui_set_replay(&ui, type, "EVENTO", 2);
    f1_ui_render(&ui, F1_UI_RACE, -1, -1);
    for (int y = 82; y < 188; ++y)
        for (int x = 88; x < 552; ++x) { h ^= getpx(x, y); h *= 1099511628211ull; }
    return h;
}

static void test_replay_scene_depends_on_event_type(void) {
    unsigned long long overtake = replay_hash(F1_EVENT_OVERTAKE);
    unsigned long long pit = replay_hash(F1_EVENT_PIT);
    unsigned long long spin = replay_hash(F1_EVENT_SPIN);
    unsigned long long crash = replay_hash(F1_EVENT_CRASH);
    assert(overtake != pit);
    assert(overtake != spin);
    assert(overtake != crash);
    assert(pit != spin && pit != crash && spin != crash);
}

static void test_every_driver_and_team_has_art(void) {
    unsigned long long seen_faces[64];
    for (int i = 0; i < F1_DRIVER_COUNT; ++i) {
        unsigned long long h = 1469598103934665603ull;
        clear(C_BG);
        f1_art_driver_portrait(10, 10, &F1_DRIVERS[i]);
        for (int y = 0; y < F1_PORTRAIT_H; ++y)
            for (int x = 0; x < F1_PORTRAIT_W; ++x) { h ^= getpx(10 + x, 10 + y); h *= 1099511628211ull; }
        assert(getpx(10 + F1_PORTRAIT_W, 10) == C_BG);   /* no se sale de su caja */
        assert(i < 64);
        seen_faces[i] = h;
        for (int j = 0; j < i; ++j) assert(seen_faces[j] != h);   /* caras distintas */
    }
    for (int i = 0; i < F1_TEAM_COUNT; ++i) {
        clear(C_BG);
        f1_art_team_logo(10, 10, &F1_TEAMS[i]);
        assert(getpx(10 + F1_LOGO_W, 10) == C_BG);
        assert(getpx(10 + F1_LOGO_W / 2, 10 + 3) != C_BG);
    }
}

static void test_hub_follows_game_state(void) {
    F1Ui ui;
    f1_ui_init(&ui);
    f1_ui_render(&ui, F1_UI_HUB, -1, -1);
    ui.game.team_points[0] = 99;
    f1_ui_render(&ui, F1_UI_HUB, -1, -1);   /* no debe fallar con puntos y equipo fuera del top 6 */
    f1_ui_new_career(&ui, "cadillac");
    f1_ui_render(&ui, F1_UI_HUB, -1, -1);
}

int main(void) {
    test_every_driver_and_team_has_art();
    test_hub_follows_game_state();
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
    test_selected_driver_accepts_pace_and_ers_orders();
    test_replay_scene_depends_on_event_type();
    puts("f1_ui_test: OK");
    return 0;
}
