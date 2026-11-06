#ifndef F1_RACE_H
#define F1_RACE_H

#include "f1_game.h"

#define F1_RACE_EVENTS 64

typedef enum { F1_TYRE_SOFT, F1_TYRE_MEDIUM, F1_TYRE_HARD,
               F1_TYRE_INTERMEDIATE, F1_TYRE_WET } F1TyreCompound;
typedef enum { F1_PACE_CONSERVE, F1_PACE_STANDARD, F1_PACE_PUSH } F1Pace;
typedef enum { F1_ERS_HARVEST, F1_ERS_BALANCED, F1_ERS_DEPLOY } F1ErsMode;
typedef enum { F1_TRACK_GREEN, F1_TRACK_VSC, F1_TRACK_SAFETY_CAR, F1_TRACK_RED } F1TrackState;
typedef enum { F1_EVENT_INFO, F1_EVENT_OVERTAKE, F1_EVENT_PIT,
               F1_EVENT_SPIN, F1_EVENT_CRASH, F1_EVENT_WEATHER } F1RaceEventType;

typedef struct {
    short driver_index;
    F1TyreCompound compound, requested_compound;
    F1Pace pace;
    F1ErsMode ers_mode;
    float tyre_wear, fuel_kg, ers_mj;
    unsigned char car_performance;
    int tyre_age_laps, total_time_ms, last_sector_ms;
    unsigned char pit_requested, stopped, retired;
} F1RaceCar;

typedef struct {
    F1RaceEventType type;
    short lap, sector, driver_a, driver_b;
    unsigned char replay;
    char text[80];
} F1RaceEvent;

typedef struct {
    unsigned int rng;
    int circuit_index, lap, laps, sector, speed, finished;
    float rain, track_water, track_temperature;
    F1TrackState track_state;
    int neutralized_steps;
    F1RaceCar car[F1_MAX_DRIVERS];
    short order[F1_MAX_DRIVERS];
    F1RaceEvent events[F1_RACE_EVENTS];
    int event_count;
} F1Race;

int f1_race_setup(F1Race *race, F1Game *game, int circuit_index, int short_laps);
int f1_race_step(F1Race *race);
void f1_race_set_order(F1Race *race, int car, F1Pace pace, F1ErsMode ers);
void f1_race_request_pit(F1Race *race, int car, F1TyreCompound compound);
void f1_race_set_team_performance(F1Race *race, const char *team_id, int performance);
int f1_race_set_speed(F1Race *race, int speed);

#endif
