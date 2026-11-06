#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/f1_race.h"

static F1Race new_race(unsigned int seed) {
    F1Game game;
    F1Race race;
    assert(f1_game_new(&game, "ferrari", seed) == 0);
    assert(f1_start_weekend(&game) == 0);
    assert(f1_run_qualifying(&game) == 0);
    assert(f1_race_setup(&race, &game, 0, 12) == 0);
    return race;
}

static void test_sector_consumes_tyre_fuel_and_ers(void) {
    F1Race race = new_race(42u);
    float wear = race.car[0].tyre_wear;
    float fuel = race.car[0].fuel_kg;
    float ers = race.car[0].ers_mj;
    f1_race_set_order(&race, 0, F1_PACE_PUSH, F1_ERS_DEPLOY);
    assert(f1_race_step(&race) == 0);
    assert(race.car[0].tyre_wear < wear);
    assert(race.car[0].fuel_kg < fuel);
    assert(race.car[0].ers_mj < ers);
    assert(race.car[0].last_sector_ms > 10000);
}

static void test_pit_stop_changes_compound_and_costs_time(void) {
    F1Race base = new_race(7u);
    F1Race stop = base;
    int before;
    f1_race_request_pit(&stop, 0, F1_TYRE_HARD);
    while (stop.sector != 2) assert(f1_race_step(&stop) == 0);
    before = stop.car[0].total_time_ms;
    assert(f1_race_step(&stop) == 0);
    assert(stop.car[0].compound == F1_TYRE_HARD);
    assert(stop.car[0].tyre_age_laps == 0);
    assert(stop.car[0].total_time_ms - before > stop.car[0].last_sector_ms);
}

static void test_wet_track_penalizes_slicks(void) {
    F1Race slick = new_race(99u);
    F1Race inter = slick;
    slick.rain = 0.8f;
    inter.rain = 0.8f;
    slick.track_water = 0.8f;
    inter.track_water = 0.8f;
    inter.car[0].compound = F1_TYRE_INTERMEDIATE;
    assert(f1_race_step(&slick) == 0);
    assert(f1_race_step(&inter) == 0);
    assert(slick.car[0].last_sector_ms > inter.car[0].last_sector_ms);
}

static void test_race_finishes_with_unique_classification(void) {
    F1Race race = new_race(123u);
    int guard = 0, seen[F1_MAX_DRIVERS] = {0}, i;
    while (!race.finished && guard++ < 1000) assert(f1_race_step(&race) == 0);
    assert(race.finished);
    assert(guard <= 12 * 3 + 1);
    for (i = 0; i < F1_DRIVER_COUNT; ++i) {
        assert(race.order[i] >= 0 && race.order[i] < F1_DRIVER_COUNT);
        assert(!seen[race.order[i]]);
        seen[race.order[i]] = 1;
    }
}

static void test_speed_controls_are_bounded(void) {
    F1Race race = new_race(2u);
    assert(f1_race_set_speed(&race, 16) == 16);
    assert(f1_race_set_speed(&race, 100) == 16);
    assert(f1_race_set_speed(&race, 0) == 0);
    assert(f1_race_set_speed(&race, -1) == 0);
}

static void test_car_development_changes_sector_performance(void) {
    F1Race fast = new_race(55u), slow = fast;
    int ferrari = -1;
    for (int i = 0; i < F1_DRIVER_COUNT; ++i)
        if (strcmp(F1_DRIVERS[fast.car[i].driver_index].team_id, "ferrari") == 0) { ferrari = i; break; }
    assert(ferrari >= 0);
    f1_race_set_team_performance(&fast, "ferrari", 95);
    f1_race_set_team_performance(&slow, "ferrari", 45);
    assert(f1_race_step(&fast) == 0);
    assert(f1_race_step(&slow) == 0);
    assert(fast.car[ferrari].last_sector_ms < slow.car[ferrari].last_sector_ms);
}

static void test_incidents_are_present_but_rare(void) {
    int seed, incidents = 0, sectors = 0;
    for (seed = 1; seed <= 200; ++seed) {
        F1Race race = new_race((unsigned int)seed);
        if (seed % 5 == 0) { race.rain = 0.7f; race.track_water = 0.65f; }
        while (!race.finished) {
            assert(f1_race_step(&race) == 0);
            ++sectors;
        }
        for (int e = 0; e < race.event_count; ++e)
            if (race.events[e].type == F1_EVENT_SPIN || race.events[e].type == F1_EVENT_CRASH)
                ++incidents;
    }
    assert(incidents > 0);
    assert(incidents < sectors / 20);
}

static void test_position_change_creates_overtake_replay(void) {
    F1Race race = new_race(808u);
    race.car[0].total_time_ms = 5000;
    race.car[1].total_time_ms = 0;
    assert(race.order[0] == 0 && race.order[1] == 1);
    assert(f1_race_step(&race) == 0);
    assert(race.order[0] == 1);
    assert(race.event_count > 0);
    assert(race.events[0].type == F1_EVENT_OVERTAKE);
    assert(race.events[0].replay);
}

static void test_running_out_of_fuel_retires_car(void) {
    F1Race race = new_race(91u);
    race.car[2].fuel_kg = 0.1f;
    assert(f1_race_step(&race) == 0);
    assert(race.car[2].retired);
}

static void test_retirements_are_ranked_by_distance(void) {
    F1Race race = new_race(92u);
    int pos_early = -1, pos_late = -1;
    race.car[0].retired = 1;
    race.car[0].sectors_completed = 2;
    race.car[1].retired = 1;
    race.car[1].sectors_completed = 20;
    assert(f1_race_step(&race) == 0);
    for (int i = 0; i < F1_DRIVER_COUNT; ++i) {
        if (race.order[i] == 0) pos_early = i;
        if (race.order[i] == 1) pos_late = i;
    }
    assert(pos_late < pos_early);
}

int main(void) {
    test_sector_consumes_tyre_fuel_and_ers();
    test_pit_stop_changes_compound_and_costs_time();
    test_wet_track_penalizes_slicks();
    test_race_finishes_with_unique_classification();
    test_speed_controls_are_bounded();
    test_car_development_changes_sector_performance();
    test_incidents_are_present_but_rare();
    test_position_change_creates_overtake_replay();
    test_running_out_of_fuel_retires_car();
    test_retirements_are_ranked_by_distance();
    puts("f1_race_test: OK");
    return 0;
}
