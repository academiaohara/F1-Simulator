#include <assert.h>
#include <stdio.h>

#include "../src/f1_team.h"

static void test_contracts_respect_budget(void) {
    F1TeamManagement team;
    f1_team_management_new(&team, 5000000, 1000000u);
    assert(f1_team_offer_driver(&team, 0, 6000000, 2) == F1_TEAM_ERR_BUDGET);
    assert(f1_team_offer_driver(&team, 0, 2000000, 3) == F1_TEAM_OK);
    assert(team.contract[0].active);
    assert(team.contract[0].end_year == 2028);
    assert(team.committed_salary == 2000000);
}

static void test_staff_upgrades_are_limited(void) {
    F1TeamManagement team;
    f1_team_management_new(&team, 20000000, 3u);
    assert(f1_team_upgrade_staff(&team, F1_STAFF_STRATEGY) == F1_TEAM_OK);
    assert(team.staff[F1_STAFF_STRATEGY] == 2);
    team.staff[F1_STAFF_STRATEGY] = 5;
    assert(f1_team_upgrade_staff(&team, F1_STAFF_STRATEGY) == F1_TEAM_ERR_LIMIT);
}

static void test_unexpected_events_are_rare_and_capped(void) {
    int seed, total = 0;
    for (seed = 1; seed <= 100; ++seed) {
        F1TeamManagement team;
        f1_team_management_new(&team, 10000000, (unsigned int)seed);
        for (int day = 0; day < 365; ++day) total += f1_team_roll_event(&team, day, 0);
        assert(team.season_event_count <= 5);
    }
    assert(total > 0);
    assert(total < 300);
}

static void test_incident_context_increases_but_does_not_force_event(void) {
    int normal = 0, pressured = 0;
    for (int seed = 1; seed <= 400; ++seed) {
        F1TeamManagement a, b;
        f1_team_management_new(&a, 1, (unsigned int)seed);
        f1_team_management_new(&b, 1, (unsigned int)seed);
        normal += f1_team_roll_event(&a, 20, 0);
        pressured += f1_team_roll_event(&b, 20, 1);
    }
    assert(pressured >= normal);
    assert(pressured < 50);
}

int main(void) {
    test_contracts_respect_budget();
    test_staff_upgrades_are_limited();
    test_unexpected_events_are_rare_and_capped();
    test_incident_context_increases_but_does_not_force_event();
    puts("f1_team_test: OK");
    return 0;
}
