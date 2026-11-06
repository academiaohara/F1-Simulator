#ifndef F1_GAME_H
#define F1_GAME_H

#include "f1_data.h"

#define F1_MAX_DRIVERS 24
#define F1_MAX_TEAMS 12
#define F1_SAVE_VERSION 1

typedef enum {
    F1_PHASE_PRE_WEEKEND,
    F1_PHASE_PRACTICE,
    F1_PHASE_RACE_READY
} F1WeekendPhase;

typedef struct {
    unsigned int version, rng;
    int year, round_index;
    F1WeekendPhase phase;
    char player_team_id[F1_ID_LEN];
    short grid[F1_MAX_DRIVERS];
    int qualifying_ms[F1_MAX_DRIVERS];
    short finish[F1_MAX_DRIVERS];
    int driver_points[F1_MAX_DRIVERS];
    int team_points[F1_MAX_TEAMS];
} F1Game;

unsigned int f1_game_random(F1Game *game);
int f1_game_new(F1Game *game, const char *team_id, unsigned int seed);
int f1_start_weekend(F1Game *game);
int f1_run_qualifying(F1Game *game);
int f1_complete_weekend(F1Game *game);

#endif
