#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/f1_data.h"

static void test_roster_shape(void) {
    assert(F1_DRIVER_COUNT == 22);
    assert(F1_TEAM_COUNT == 11);
    assert(F1_CIRCUIT_COUNT == 24);
    assert(F1_ROUND_COUNT == 24);
    assert(f1_data_validate() == 0);
}

static void test_stable_ids_are_unique(void) {
    int i, j;
    for (i = 0; i < F1_DRIVER_COUNT; ++i)
        for (j = i + 1; j < F1_DRIVER_COUNT; ++j)
            assert(strcmp(F1_DRIVERS[i].id, F1_DRIVERS[j].id) != 0);
    for (i = 0; i < F1_TEAM_COUNT; ++i)
        for (j = i + 1; j < F1_TEAM_COUNT; ++j)
            assert(strcmp(F1_TEAMS[i].id, F1_TEAMS[j].id) != 0);
}

static void test_relations_resolve(void) {
    int i;
    for (i = 0; i < F1_DRIVER_COUNT; ++i)
        assert(f1_team_by_id(F1_DRIVERS[i].team_id) != NULL);
    for (i = 0; i < F1_ROUND_COUNT; ++i)
        assert(f1_circuit_by_id(F1_ROUNDS[i].circuit_id) != NULL);
}

static void test_track_shapes_are_normalized(void) {
    int i, p;
    for (i = 0; i < F1_CIRCUIT_COUNT; ++i) {
        assert(F1_CIRCUITS[i].point_count >= 8);
        for (p = 0; p < F1_CIRCUITS[i].point_count; ++p) {
            assert(F1_CIRCUITS[i].points[p].x >= 0);
            assert(F1_CIRCUITS[i].points[p].x <= 1000);
            assert(F1_CIRCUITS[i].points[p].y >= 0);
            assert(F1_CIRCUITS[i].points[p].y <= 1000);
        }
    }
}

static void test_calendar_tracks_have_distinct_real_geometry(void) {
    int i, j;
    for (i = 0; i < F1_CIRCUIT_COUNT; ++i) {
        int duplicate = 0;
        for (j = 0; j < i; ++j) {
            int p, same = F1_CIRCUITS[i].point_count == F1_CIRCUITS[j].point_count;
            for (p = 0; same && p < F1_CIRCUITS[i].point_count; ++p)
                same = F1_CIRCUITS[i].points[p].x == F1_CIRCUITS[j].points[p].x &&
                       F1_CIRCUITS[i].points[p].y == F1_CIRCUITS[j].points[p].y;
            duplicate += same;
        }
        assert(duplicate == 0);
    }
}

static void test_circuit_distance_and_laps_are_real_values(void) {
    const F1CircuitDef *albert = f1_circuit_by_id("albert_park");
    const F1CircuitDef *spa = f1_circuit_by_id("spa");
    assert(albert && albert->laps == 58);
    assert(albert->length_km > 5.27f && albert->length_km < 5.29f);
    assert(spa && spa->laps == 44);
    assert(spa->length_km > 7.00f && spa->length_km < 7.01f);
}

int main(void) {
    test_roster_shape();
    test_stable_ids_are_unique();
    test_relations_resolve();
    test_track_shapes_are_normalized();
    test_calendar_tracks_have_distinct_real_geometry();
    test_circuit_distance_and_laps_are_real_values();
    puts("f1_data_test: OK");
    return 0;
}
