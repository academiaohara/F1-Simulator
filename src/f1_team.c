#include "f1_team.h"

#include <string.h>

static unsigned int team_random(F1TeamManagement *team) {
    unsigned int x = team->rng ? team->rng : 1u;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return team->rng = x;
}

void f1_team_management_new(F1TeamManagement *team, int cash, unsigned int seed) {
    int i;
    if (!team) return;
    memset(team, 0, sizeof *team);
    team->cash = cash;
    team->annual_salary_budget = cash;
    team->year = 2026;
    team->rng = seed ? seed : 1u;
    team->last_event_day = -100;
    for (i = 0; i < F1_STAFF_COUNT; ++i) team->staff[i] = 1;
}

int f1_team_offer_driver(F1TeamManagement *team, int driver, int salary, int years) {
    F1DriverContract *c;
    int previous;
    if (!team || driver < 0 || driver >= F1_TEAM_MAX_DRIVERS || salary < 0 || years < 1 || years > 5)
        return F1_TEAM_ERR_INVALID;
    c = &team->contract[driver];
    previous = c->active ? c->salary : 0;
    if (team->committed_salary - previous + salary > team->annual_salary_budget)
        return F1_TEAM_ERR_BUDGET;
    team->committed_salary = team->committed_salary - previous + salary;
    c->active = 1;
    c->salary = salary;
    c->start_year = team->year;
    c->end_year = team->year + years - 1;
    return F1_TEAM_OK;
}

int f1_team_upgrade_staff(F1TeamManagement *team, F1StaffType staff) {
    int level, cost;
    if (!team || staff < 0 || staff >= F1_STAFF_COUNT) return F1_TEAM_ERR_INVALID;
    level = team->staff[staff];
    if (level >= 5) return F1_TEAM_ERR_LIMIT;
    cost = 250000 * level * level;
    if (cost > team->cash) return F1_TEAM_ERR_BUDGET;
    team->cash -= cost;
    team->staff[staff] = (unsigned char)(level + 1);
    return F1_TEAM_OK;
}

int f1_team_roll_event(F1TeamManagement *team, int day, int pressured_context) {
    static const char *events[] = {
        "Un proveedor retrasa una pieza no critica",
        "Un patrocinador propone un objetivo adicional",
        "Un ingeniero encuentra una mejora de proceso",
        "Un piloto solicita una reunion sobre su contrato",
        "La fabrica necesita mantenimiento imprevisto"
    };
    unsigned int threshold;
    if (!team || day < 0 || team->season_event_count >= 5 || day - team->last_event_day < 30) return 0;
    threshold = pressured_context ? 140u : 40u;
    if (team_random(team) % 10000u >= threshold) return 0;
    strncpy(team->last_event, events[team_random(team) % 5u], sizeof team->last_event - 1);
    team->last_event[sizeof team->last_event - 1] = 0;
    team->last_event_day = day;
    ++team->season_event_count;
    return 1;
}
