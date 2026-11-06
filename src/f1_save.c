#include "f1_save.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

typedef struct {
    char magic[8];
    unsigned int version, size, checksum;
} SaveHeader;

static unsigned int checksum32(const void *data, unsigned int size) {
    const unsigned char *p = (const unsigned char *)data;
    unsigned int h = 2166136261u, i;
    for (i = 0; i < size; ++i) { h ^= p[i]; h *= 16777619u; }
    return h;
}

static int valid_game(const F1Game *g) {
    int i, seen[F1_MAX_DRIVERS] = {0};
    if (g->version != F1_SAVE_VERSION || g->year < 2026 || g->year > 2200 ||
        g->round_index < 0 || g->round_index >= F1_ROUND_COUNT ||
        g->phase < F1_PHASE_PRE_WEEKEND || g->phase > F1_PHASE_RACE_READY ||
        !f1_team_by_id(g->player_team_id)) return 0;
    for (i = 0; i < F1_DRIVER_COUNT; ++i) {
        if (g->phase == F1_PHASE_PRE_WEEKEND && g->grid[i] == -1) continue;
        if (g->grid[i] < 0 || g->grid[i] >= F1_DRIVER_COUNT || seen[g->grid[i]]) return 0;
        seen[g->grid[i]] = 1;
        if (g->driver_points[i] < 0) return 0;
    }
    return 1;
}

static int valid_car(const F1CarProgram *car) {
    int i;
    if (car->budget < 0 || car->atr_hours < 0 || car->active_projects < 0 ||
        car->active_projects > F1_CAR_PROJECTS) return 0;
    for (i = 0; i < F1_COMPONENT_COUNT; ++i)
        if (car->component[i].performance > 100 || car->component[i].reliability > 100 ||
            car->component[i].condition > 100) return 0;
    for (i = 0; i < F1_CAR_PROJECTS; ++i) if (car->project[i].active) {
        const F1CarProject *p = &car->project[i];
        if (p->component >= F1_COMPONENT_COUNT || p->days_total <= 0 || p->days_done < 0 ||
            p->days_done > p->days_total || p->cost <= 0 || p->atr_cost <= 0) return 0;
    }
    return 1;
}

static int valid_race(const F1Race *r) {
    int i, seen[F1_MAX_DRIVERS] = {0};
    if (r->circuit_index < 0 || r->circuit_index >= F1_CIRCUIT_COUNT || r->laps <= 0 ||
        r->lap < 0 || r->lap > r->laps || r->sector < 0 || r->sector > 2 ||
        r->event_count < 0 || r->event_count > F1_RACE_EVENTS ||
        !isfinite(r->rain) || !isfinite(r->track_water) || !isfinite(r->track_temperature) ||
        r->rain < 0.0f || r->rain > 1.0f || r->track_water < 0.0f || r->track_water > 1.0f)
        return 0;
    for (i = 0; i < F1_DRIVER_COUNT; ++i) {
        if (r->order[i] < 0 || r->order[i] >= F1_DRIVER_COUNT || seen[r->order[i]]) return 0;
        seen[r->order[i]] = 1;
        if (r->car[i].driver_index < 0 || r->car[i].driver_index >= F1_DRIVER_COUNT ||
            r->car[i].compound < F1_TYRE_SOFT || r->car[i].compound > F1_TYRE_WET ||
            r->car[i].pace < F1_PACE_CONSERVE || r->car[i].pace > F1_PACE_PUSH ||
            r->car[i].ers_mode < F1_ERS_HARVEST || r->car[i].ers_mode > F1_ERS_DEPLOY ||
            !isfinite(r->car[i].tyre_wear) || !isfinite(r->car[i].fuel_kg) ||
            !isfinite(r->car[i].ers_mj) || r->car[i].tyre_wear < 0.0f ||
            r->car[i].tyre_wear > 100.0f || r->car[i].fuel_kg < 0.0f ||
            r->car[i].ers_mj < 0.0f || r->car[i].ers_mj > 4.0f) return 0;
    }
    for (i = 0; i < r->event_count; ++i)
        if (r->events[i].type < F1_EVENT_INFO || r->events[i].type > F1_EVENT_WEATHER ||
            !memchr(r->events[i].text, 0, sizeof r->events[i].text)) return 0;
    return 1;
}

static int valid_team(const F1TeamManagement *team) {
    int i;
    if (team->year < 2026 || team->year > 2200 || team->annual_salary_budget < 0 ||
        team->committed_salary < 0 || team->committed_salary > team->annual_salary_budget ||
        team->season_event_count < 0 || team->season_event_count > 5 ||
        !memchr(team->last_event, 0, sizeof team->last_event)) return 0;
    for (i = 0; i < F1_STAFF_COUNT; ++i)
        if (team->staff[i] < 1 || team->staff[i] > 5) return 0;
    for (i = 0; i < F1_TEAM_MAX_DRIVERS; ++i) if (team->contract[i].active) {
        const F1DriverContract *c = &team->contract[i];
        if (c->salary < 0 || c->start_year < 2026 || c->end_year < c->start_year ||
            c->end_year > c->start_year + 4) return 0;
    }
    return 1;
}

static int valid_state(const F1SaveState *state) {
    if (!state || !valid_game(&state->game) || !valid_car(&state->car) ||
        !valid_team(&state->team) || state->race_active > 1) return 0;
    if (state->race_active && !valid_race(&state->race)) return 0;
    return 1;
}

F1SaveResult f1_save_write(const char *path, const F1SaveState *state) {
    char tmp[512];
    SaveHeader h;
    FILE *f;
    if (!path || !state || strlen(path) + 5 >= sizeof tmp) return F1_SAVE_ERR_IO;
    if (!valid_state(state)) return F1_SAVE_ERR_STATE;
    snprintf(tmp, sizeof tmp, "%s.tmp", path);
    memcpy(h.magic, "F1PXSAV", 8);
    h.version = F1_SAVE_FILE_VERSION;
    h.size = (unsigned int)sizeof *state;
    h.checksum = checksum32(state, h.size);
    f = fopen(tmp, "wb");
    if (!f) return F1_SAVE_ERR_IO;
    if (fwrite(&h, sizeof h, 1, f) != 1 || fwrite(state, sizeof *state, 1, f) != 1 || fflush(f) != 0) {
        fclose(f);
        remove(tmp);
        return F1_SAVE_ERR_IO;
    }
    if (fclose(f) != 0) { remove(tmp); return F1_SAVE_ERR_IO; }
#ifdef _WIN32
    if (!MoveFileExA(tmp, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        remove(tmp); return F1_SAVE_ERR_IO;
    }
#else
    if (rename(tmp, path) != 0) { remove(tmp); return F1_SAVE_ERR_IO; }
#endif
    return F1_SAVE_OK;
}

F1SaveResult f1_save_read(const char *path, F1SaveState *state) {
    SaveHeader h;
    F1SaveState temporary;
    FILE *f;
    int extra;
    if (!path || !state) return F1_SAVE_ERR_IO;
    f = fopen(path, "rb");
    if (!f) return F1_SAVE_ERR_IO;
    if (fread(&h, sizeof h, 1, f) != 1 || memcmp(h.magic, "F1PXSAV", 8) != 0 ||
        h.size != sizeof temporary) { fclose(f); return F1_SAVE_ERR_FORMAT; }
    if (h.version != F1_SAVE_FILE_VERSION) { fclose(f); return F1_SAVE_ERR_VERSION; }
    if (fread(&temporary, sizeof temporary, 1, f) != 1) { fclose(f); return F1_SAVE_ERR_FORMAT; }
    extra = fgetc(f);
    fclose(f);
    if (extra != EOF) return F1_SAVE_ERR_FORMAT;
    if (checksum32(&temporary, (unsigned int)sizeof temporary) != h.checksum)
        return F1_SAVE_ERR_CHECKSUM;
    if (!valid_state(&temporary)) return F1_SAVE_ERR_STATE;
    *state = temporary;
    return F1_SAVE_OK;
}
