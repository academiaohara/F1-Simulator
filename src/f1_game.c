#include "f1_game.h"

#include <limits.h>
#include <string.h>

static int team_index(const char *id) {
    int i;
    for (i = 0; i < F1_TEAM_COUNT; ++i)
        if (strcmp(F1_TEAMS[i].id, id) == 0) return i;
    return -1;
}

unsigned int f1_game_random(F1Game *game) {
    unsigned int x = game->rng ? game->rng : 0xA341316Cu;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    game->rng = x;
    return x;
}

int f1_game_new(F1Game *game, const char *team_id, unsigned int seed) {
    int i;
    if (!game || !team_id || !f1_team_by_id(team_id)) return 1;
    memset(game, 0, sizeof *game);
    game->version = F1_SAVE_VERSION;
    game->rng = seed ? seed : 1u;
    game->year = 2026;
    game->phase = F1_PHASE_PRE_WEEKEND;
    strncpy(game->player_team_id, team_id, sizeof game->player_team_id - 1);
    for (i = 0; i < F1_MAX_DRIVERS; ++i) {
        game->grid[i] = -1;
        game->finish[i] = -1;
        game->qualifying_ms[i] = INT_MAX;
    }
    return 0;
}

int f1_start_weekend(F1Game *game) {
    if (!game || game->phase != F1_PHASE_PRE_WEEKEND ||
        game->round_index < 0 || game->round_index >= F1_ROUND_COUNT) return 1;
    game->phase = F1_PHASE_PRACTICE;
    return 0;
}

int f1_run_qualifying(F1Game *game) {
    int i, j;
    if (!game || game->phase != F1_PHASE_PRACTICE) return 1;
    for (i = 0; i < F1_DRIVER_COUNT; ++i) {
        const F1DriverDef *d = &F1_DRIVERS[i];
        int team = team_index(d->team_id);
        int car = team >= 0 ? F1_TEAMS[team].reputation : 50;
        int noise = (int)(f1_game_random(game) % 801u) - 400;
        game->grid[i] = (short)i;
        game->qualifying_ms[i] = 96000 - d->qualifying * 115 - car * 45 + noise;
    }
    for (i = 1; i < F1_DRIVER_COUNT; ++i) {
        short key = game->grid[i];
        j = i - 1;
        while (j >= 0 && game->qualifying_ms[game->grid[j]] > game->qualifying_ms[key]) {
            game->grid[j + 1] = game->grid[j];
            --j;
        }
        game->grid[j + 1] = key;
    }
    game->phase = F1_PHASE_RACE_READY;
    return 0;
}

int f1_complete_weekend(F1Game *game) {
    static const int points[10] = {25, 18, 15, 12, 10, 8, 6, 4, 2, 1};
    int i;
    if (!game || game->phase != F1_PHASE_RACE_READY) return 1;
    for (i = 0; i < F1_DRIVER_COUNT; ++i) {
        int di = game->grid[i];
        int ti = team_index(F1_DRIVERS[di].team_id);
        game->finish[i] = (short)di;
        if (i < 10) {
            game->driver_points[di] += points[i];
            if (ti >= 0) game->team_points[ti] += points[i];
        }
    }
    ++game->round_index;
    if (game->round_index >= F1_ROUND_COUNT) {
        game->round_index = 0;
        ++game->year;
    }
    game->phase = F1_PHASE_PRE_WEEKEND;
    return 0;
}
