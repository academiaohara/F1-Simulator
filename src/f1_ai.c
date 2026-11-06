#include "f1_ai.h"

#include <string.h>

static unsigned int ai_random(F1Race *race) {
    unsigned int x = race->rng ? race->rng : 1u;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return race->rng = x;
}

static float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }

void f1_ai_update(F1Race *race, int index, float accuracy) {
    F1RaceCar *car;
    float uncertainty, estimate, noise;
    int remaining;
    if (!race || index < 0 || index >= F1_DRIVER_COUNT) return;
    car = &race->car[index];
    if (car->retired || car->pit_requested) return;
    accuracy = clampf(accuracy, 0.0f, 1.0f);
    uncertainty = (1.0f - accuracy) * 12.0f;
    noise = ((int)(ai_random(race) % 2001u) - 1000) / 1000.0f * uncertainty;
    estimate = car->tyre_wear + noise;
    remaining = race->laps - race->lap;

    if (race->track_water > 0.72f && car->compound != F1_TYRE_WET) {
        f1_race_request_pit(race, index, F1_TYRE_WET);
        return;
    }
    if (race->track_water > 0.28f && race->track_water <= 0.72f &&
        car->compound != F1_TYRE_INTERMEDIATE && car->compound != F1_TYRE_WET) {
        f1_race_request_pit(race, index, F1_TYRE_INTERMEDIATE);
        return;
    }
    if (race->track_water < 0.12f &&
        (car->compound == F1_TYRE_INTERMEDIATE || car->compound == F1_TYRE_WET)) {
        f1_race_request_pit(race, index, remaining > 9 ? F1_TYRE_HARD : F1_TYRE_MEDIUM);
        return;
    }
    if (estimate < (race->track_state == F1_TRACK_SAFETY_CAR ? 48.0f : 30.0f)) {
        F1TyreCompound replacement = race->track_water > 0.72f ? F1_TYRE_WET :
                                      race->track_water > 0.28f ? F1_TYRE_INTERMEDIATE :
                                      remaining > 9 ? F1_TYRE_HARD : F1_TYRE_MEDIUM;
        f1_race_request_pit(race, index, replacement);
        return;
    }
    if (car->fuel_kg < remaining * 1.45f) car->pace = F1_PACE_CONSERVE;
    else if (car->tyre_wear > 55.0f && remaining <= 4) car->pace = F1_PACE_PUSH;
    else car->pace = F1_PACE_STANDARD;
    car->ers_mode = remaining <= 2 ? F1_ERS_DEPLOY :
                    car->ers_mj < 1.0f ? F1_ERS_HARVEST : F1_ERS_BALANCED;
}

void f1_ai_update_all(F1Race *race, const char *human_team_id, float accuracy) {
    int i;
    if (!race) return;
    for (i = 0; i < F1_DRIVER_COUNT; ++i) {
        const F1DriverDef *driver = &F1_DRIVERS[race->car[i].driver_index];
        if (!human_team_id || strcmp(driver->team_id, human_team_id) != 0)
            f1_ai_update(race, i, accuracy);
    }
}
