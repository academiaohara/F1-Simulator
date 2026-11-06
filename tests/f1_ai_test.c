#include <assert.h>
#include <stdio.h>

#include "../src/f1_ai.h"

static F1Race setup(void) {
    F1Game game;
    F1Race race;
    assert(f1_game_new(&game, "ferrari", 44u) == 0);
    assert(f1_start_weekend(&game) == 0);
    assert(f1_run_qualifying(&game) == 0);
    assert(f1_race_setup(&race, &game, 0, 18) == 0);
    return race;
}

static void test_ai_pits_worn_tyres(void) {
    F1Race race = setup();
    race.car[3].tyre_wear = 21.0f;
    f1_ai_update(&race, 3, 1.0f);
    assert(race.car[3].pit_requested);
    assert(race.car[3].requested_compound == F1_TYRE_HARD ||
           race.car[3].requested_compound == F1_TYRE_MEDIUM);
}

static void test_ai_reacts_to_wet_track(void) {
    F1Race race = setup();
    race.track_water = 0.62f;
    race.rain = 0.8f;
    f1_ai_update(&race, 4, 1.0f);
    assert(race.car[4].pit_requested);
    assert(race.car[4].requested_compound == F1_TYRE_INTERMEDIATE ||
           race.car[4].requested_compound == F1_TYRE_WET);
}

static void test_ai_does_not_double_stop_fresh_correct_tyre(void) {
    F1Race race = setup();
    race.track_water = 0.55f;
    race.car[5].compound = F1_TYRE_INTERMEDIATE;
    race.car[5].tyre_wear = 98.0f;
    f1_ai_update(&race, 5, 1.0f);
    assert(!race.car[5].pit_requested);
}

static void test_uncertainty_changes_threshold_not_observation(void) {
    F1Race certain = setup(), uncertain = certain;
    certain.car[6].tyre_wear = 32.0f;
    uncertain.car[6].tyre_wear = 32.0f;
    f1_ai_update(&certain, 6, 1.0f);
    f1_ai_update(&uncertain, 6, 0.0f);
    assert(certain.rng != uncertain.rng ||
           certain.car[6].pit_requested != uncertain.car[6].pit_requested);
}

int main(void) {
    test_ai_pits_worn_tyres();
    test_ai_reacts_to_wet_track();
    test_ai_does_not_double_stop_fresh_correct_tyre();
    test_uncertainty_changes_threshold_not_observation();
    puts("f1_ai_test: OK");
    return 0;
}
