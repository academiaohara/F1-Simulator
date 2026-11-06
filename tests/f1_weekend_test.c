#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/f1_game.h"

static void assert_grid_is_permutation(const F1Game *g) {
    int seen[F1_MAX_DRIVERS] = {0};
    int i;
    for (i = 0; i < F1_DRIVER_COUNT; ++i) {
        assert(g->grid[i] >= 0 && g->grid[i] < F1_DRIVER_COUNT);
        assert(!seen[g->grid[i]]);
        seen[g->grid[i]] = 1;
    }
}

static void test_new_game_selects_team_and_calendar(void) {
    F1Game g;
    assert(f1_game_new(&g, "ferrari", 12345u) == 0);
    assert(g.year == 2026);
    assert(g.round_index == 0);
    assert(g.phase == F1_PHASE_PRE_WEEKEND);
    assert(strcmp(g.player_team_id, "ferrari") == 0);
    assert(g.rng == 12345u);
}

static void test_weekend_reaches_race_grid(void) {
    F1Game a, b;
    assert(f1_game_new(&a, "mclaren", 77u) == 0);
    assert(f1_game_new(&b, "mclaren", 77u) == 0);
    assert(f1_start_weekend(&a) == 0);
    assert(a.phase == F1_PHASE_PRACTICE);
    assert(f1_run_qualifying(&a) == 0);
    assert(f1_start_weekend(&b) == 0);
    assert(f1_run_qualifying(&b) == 0);
    assert(a.phase == F1_PHASE_RACE_READY);
    assert_grid_is_permutation(&a);
    assert(memcmp(a.grid, b.grid, sizeof a.grid) == 0);
    assert(memcmp(a.qualifying_ms, b.qualifying_ms, sizeof a.qualifying_ms) == 0);
}

static void test_round_and_season_progression(void) {
    F1Game g;
    int i;
    assert(f1_game_new(&g, "williams", 9u) == 0);
    for (i = 0; i < F1_ROUND_COUNT; ++i) {
        assert(f1_start_weekend(&g) == 0);
        assert(f1_run_qualifying(&g) == 0);
        assert(f1_complete_weekend(&g) == 0);
    }
    assert(g.year == 2027);
    assert(g.round_index == 0);
    assert(g.phase == F1_PHASE_PRE_WEEKEND);
}

static void test_invalid_team_is_rejected(void) {
    F1Game g;
    assert(f1_game_new(&g, "missing_team", 1u) != 0);
}

int main(void) {
    test_new_game_selects_team_and_calendar();
    test_weekend_reaches_race_grid();
    test_round_and_season_progression();
    test_invalid_team_is_rejected();
    puts("f1_weekend_test: OK");
    return 0;
}
