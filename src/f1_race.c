#include "f1_race.h"

#include <stdio.h>
#include <string.h>

static unsigned int rrnd(F1Race *r) {
    unsigned int x = r->rng ? r->rng : 1u;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return r->rng = x;
}

static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }
static float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }

static int team_strength(const F1DriverDef *d) {
    const F1TeamDef *t = f1_team_by_id(d->team_id);
    return t ? t->reputation : 50;
}

static float tyre_grip(F1TyreCompound c, float water) {
    static const float dry[5] = {1.035f, 1.015f, 1.0f, 0.88f, 0.78f};
    static const float wet[5] = {0.64f, 0.65f, 0.66f, 1.02f, 1.04f};
    return dry[c] * (1.0f - water) + wet[c] * water;
}

static float tyre_loss(F1TyreCompound c) {
    static const float loss[5] = {2.0f, 1.35f, 0.9f, 1.1f, 0.8f};
    return loss[c];
}

static F1RaceEvent *add_event(F1Race *r, F1RaceEventType type, int driver, const char *verb) {
    F1RaceEvent *e;
    const F1DriverDef *d;
    if (r->event_count >= F1_RACE_EVENTS) return 0;
    e = &r->events[r->event_count++];
    d = &F1_DRIVERS[r->car[driver].driver_index];
    memset(e, 0, sizeof *e);
    e->type = type;
    e->lap = (short)(r->lap + 1);
    e->sector = (short)(r->sector + 1);
    e->driver_a = (short)driver;
    e->driver_b = -1;
    e->replay = 1;
    snprintf(e->text, sizeof e->text, "%s %s", d->code, verb);
    return e;
}

static void sort_order(F1Race *r) {
    int i, j;
    for (i = 0; i < F1_DRIVER_COUNT; ++i) r->order[i] = (short)i;
    for (i = 1; i < F1_DRIVER_COUNT; ++i) {
        short key = r->order[i];
        j = i - 1;
        while (j >= 0) {
            const F1RaceCar *left = &r->car[r->order[j]], *right = &r->car[key];
            int move = left->retired > right->retired;
            if (left->retired == right->retired) {
                if (left->retired)
                    move = left->sectors_completed < right->sectors_completed ||
                           (left->sectors_completed == right->sectors_completed &&
                            left->total_time_ms > right->total_time_ms);
                else move = left->total_time_ms > right->total_time_ms;
            }
            if (!move) break;
            r->order[j + 1] = r->order[j]; --j;
        }
        r->order[j + 1] = key;
    }
}

int f1_race_setup(F1Race *r, F1Game *g, int circuit_index, int short_laps) {
    int i;
    if (!r || !g || g->phase != F1_PHASE_RACE_READY || circuit_index < 0 ||
        circuit_index >= F1_CIRCUIT_COUNT) return 1;
    memset(r, 0, sizeof *r);
    r->rng = g->rng;
    r->circuit_index = circuit_index;
    r->laps = short_laps > 0 ? short_laps : F1_CIRCUITS[circuit_index].laps;
    r->speed = 1;
    r->track_temperature = 31.0f;
    for (i = 0; i < F1_DRIVER_COUNT; ++i) {
        int di = g->grid[i];
        r->order[i] = (short)i;
        r->car[i].driver_index = (short)di;
        r->car[i].compound = i < 10 ? F1_TYRE_MEDIUM : F1_TYRE_SOFT;
        r->car[i].requested_compound = r->car[i].compound;
        r->car[i].pace = F1_PACE_STANDARD;
        r->car[i].ers_mode = F1_ERS_BALANCED;
        r->car[i].tyre_wear = 100.0f;
        r->car[i].fuel_kg = r->laps * 1.65f + 1.5f;
        r->car[i].ers_mj = 4.0f;
        r->car[i].car_performance = (unsigned char)team_strength(&F1_DRIVERS[di]);
        r->car[i].total_time_ms = i * 120;
    }
    return 0;
}

void f1_race_set_team_performance(F1Race *r, const char *team_id, int performance) {
    int i;
    if (!r || !team_id) return;
    performance = clampi(performance, 1, 100);
    for (i = 0; i < F1_DRIVER_COUNT; ++i) {
        const F1DriverDef *d = &F1_DRIVERS[r->car[i].driver_index];
        if (strcmp(d->team_id, team_id) == 0) r->car[i].car_performance = (unsigned char)performance;
    }
}

void f1_race_set_order(F1Race *r, int car, F1Pace pace, F1ErsMode ers) {
    if (!r || car < 0 || car >= F1_DRIVER_COUNT) return;
    r->car[car].pace = (F1Pace)clampi(pace, F1_PACE_CONSERVE, F1_PACE_PUSH);
    r->car[car].ers_mode = (F1ErsMode)clampi(ers, F1_ERS_HARVEST, F1_ERS_DEPLOY);
}

void f1_race_request_pit(F1Race *r, int car, F1TyreCompound compound) {
    if (!r || car < 0 || car >= F1_DRIVER_COUNT || compound < 0 || compound > F1_TYRE_WET) return;
    r->car[car].pit_requested = 1;
    r->car[car].requested_compound = compound;
}

int f1_race_set_speed(F1Race *r, int speed) {
    static const int allowed[] = {0, 1, 2, 4, 8, 16};
    int i, best = 0;
    if (!r) return 0;
    for (i = 0; i < 6; ++i) if (speed >= allowed[i]) best = allowed[i];
    r->speed = best;
    return best;
}

int f1_race_step(F1Race *r) {
    int i;
    int deploy_safety_car = 0;
    F1TrackState sector_state;
    short old_order[F1_MAX_DRIVERS];
    if (!r || r->finished) return 1;
    memcpy(old_order, r->order, sizeof old_order);
    if (r->neutralized_steps > 0 && --r->neutralized_steps == 0) r->track_state = F1_TRACK_GREEN;
    sector_state = r->track_state;
    r->track_water = clampf(r->track_water + r->rain * 0.018f - (1.0f - r->rain) * 0.006f, 0.0f, 1.0f);
    for (i = 0; i < F1_DRIVER_COUNT; ++i) {
        F1RaceCar *c = &r->car[i];
        const F1DriverDef *d = &F1_DRIVERS[c->driver_index];
        float grip, wear_penalty, pace_gain, ers_gain, skill, noise;
        int sector_ms, pit = 0;
        if (c->retired) continue;
        grip = tyre_grip(c->compound, r->track_water);
        wear_penalty = c->tyre_wear < 35.0f ? (35.0f - c->tyre_wear) * 0.0018f : 0.0f;
        pace_gain = c->pace == F1_PACE_PUSH ? 0.018f : c->pace == F1_PACE_CONSERVE ? -0.018f : 0.0f;
        ers_gain = c->ers_mode == F1_ERS_DEPLOY && c->ers_mj > 0.2f ? 0.012f :
                   c->ers_mode == F1_ERS_HARVEST ? -0.008f : 0.0f;
        skill = (d->pace * 0.55f + d->racecraft * 0.2f + c->car_performance * 0.25f) / 100.0f;
        noise = ((int)(rrnd(r) % 401u) - 200) / 100000.0f;
        sector_ms = (int)(33500.0f * (1.24f - skill * 0.22f + wear_penalty - pace_gain - ers_gain - (grip - 1.0f) + noise));
        if (sector_state == F1_TRACK_SAFETY_CAR) sector_ms = sector_ms * 145 / 100;
        c->last_sector_ms = clampi(sector_ms, 12000, 120000);
        c->total_time_ms += c->last_sector_ms;
        c->fuel_kg = clampf(c->fuel_kg - (0.52f + c->pace * 0.04f), 0.0f, 200.0f);
        c->tyre_wear = clampf(c->tyre_wear - tyre_loss(c->compound) * (0.82f + c->pace * 0.18f), 0.0f, 100.0f);
        if (c->ers_mode == F1_ERS_DEPLOY) c->ers_mj = clampf(c->ers_mj - 0.42f, 0.0f, 4.0f);
        else if (c->ers_mode == F1_ERS_HARVEST) c->ers_mj = clampf(c->ers_mj + 0.36f, 0.0f, 4.0f);
        else c->ers_mj = clampf(c->ers_mj + 0.06f, 0.0f, 4.0f);
        if (r->sector == 2 && c->pit_requested) {
            pit = (int)(F1_CIRCUITS[r->circuit_index].pit_loss_seconds * 1000.0f) + 2200;
            c->total_time_ms += pit;
            c->compound = c->requested_compound;
            c->tyre_wear = 100.0f;
            c->tyre_age_laps = 0;
            c->pit_requested = 0;
            ++c->stopped;
            add_event(r, F1_EVENT_PIT, i, "completa su parada en boxes");
        }
        if (r->sector == 2 && !pit) ++c->tyre_age_laps;
        {
            unsigned int risk = 8u;
            risk += (unsigned int)(r->track_water * 70.0f);
            if (c->pace == F1_PACE_PUSH) risk += 18u;
            if (c->tyre_wear < 25.0f) risk += (unsigned int)(25.0f - c->tyre_wear) * 3u;
            if (d->consistency < 90) risk += (unsigned int)(90 - d->consistency);
            if (rrnd(r) % 1000000u < risk) {
                if (rrnd(r) % 5u == 0u) {
                    c->retired = 1;
                    add_event(r, F1_EVENT_CRASH, i, "abandona tras un accidente");
                    deploy_safety_car = 1;
                } else {
                    c->total_time_ms += 6000 + (int)(rrnd(r) % 5000u);
                    add_event(r, F1_EVENT_SPIN, i, "hace un trompo");
                }
            }
        }
        if (!c->retired && c->fuel_kg <= 0.0f) {
            c->retired = 1;
            add_event(r, F1_EVENT_INFO, i, "abandona sin combustible");
        }
        if (!c->retired) ++c->sectors_completed;
    }
    if (deploy_safety_car) {
        r->track_state = F1_TRACK_SAFETY_CAR;
        r->neutralized_steps = 6;
    }
    sort_order(r);
    for (i = 0; i < F1_DRIVER_COUNT; ++i) if (r->order[i] != old_order[i]) {
        int old_position;
        for (old_position = i + 1; old_position < F1_DRIVER_COUNT; ++old_position)
            if (old_order[old_position] == r->order[i]) break;
        if (old_position < F1_DRIVER_COUNT) {
            F1RaceEvent *event = add_event(r, F1_EVENT_OVERTAKE, r->order[i], "gana una posicion");
            if (event) event->driver_b = old_order[i];
            break;
        }
    }
    ++r->sector;
    if (r->sector == 3) { r->sector = 0; ++r->lap; }
    if (r->lap >= r->laps) { r->finished = 1; sort_order(r); }
    return 0;
}
