#include "f1_data.h"

#include <string.h>

const F1DriverDef *f1_driver_by_id(const char *id) {
    int i;
    for (i = 0; i < F1_DRIVER_COUNT; ++i)
        if (strcmp(F1_DRIVERS[i].id, id) == 0) return &F1_DRIVERS[i];
    return 0;
}

const F1TeamDef *f1_team_by_id(const char *id) {
    int i;
    for (i = 0; i < F1_TEAM_COUNT; ++i)
        if (strcmp(F1_TEAMS[i].id, id) == 0) return &F1_TEAMS[i];
    return 0;
}

const F1CircuitDef *f1_circuit_by_id(const char *id) {
    int i;
    for (i = 0; i < F1_CIRCUIT_COUNT; ++i)
        if (strcmp(F1_CIRCUITS[i].id, id) == 0) return &F1_CIRCUITS[i];
    return 0;
}

int f1_data_validate(void) {
    int i, j;
    if (F1_DRIVER_COUNT < 20 || F1_TEAM_COUNT < 10 || F1_CIRCUIT_COUNT < 20) return 1;
    for (i = 0; i < F1_DRIVER_COUNT; ++i) {
        if (!F1_DRIVERS[i].id[0] || !f1_team_by_id(F1_DRIVERS[i].team_id)) return 2;
        for (j = i + 1; j < F1_DRIVER_COUNT; ++j)
            if (strcmp(F1_DRIVERS[i].id, F1_DRIVERS[j].id) == 0) return 3;
    }
    for (i = 0; i < F1_ROUND_COUNT; ++i)
        if (!f1_circuit_by_id(F1_ROUNDS[i].circuit_id)) return 4;
    for (i = 0; i < F1_CIRCUIT_COUNT; ++i)
        if (F1_CIRCUITS[i].point_count < 8 || F1_CIRCUITS[i].point_count > F1_MAX_TRACK_POINTS) return 5;
    return 0;
}
