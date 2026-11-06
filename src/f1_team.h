#ifndef F1_TEAM_H
#define F1_TEAM_H

#define F1_TEAM_MAX_DRIVERS 24

typedef enum { F1_STAFF_TECHNICAL, F1_STAFF_AERO, F1_STAFF_STRATEGY,
               F1_STAFF_PIT_CREW, F1_STAFF_SCOUTING, F1_STAFF_COUNT } F1StaffType;
typedef enum { F1_TEAM_OK = 0, F1_TEAM_ERR_BUDGET = -1,
               F1_TEAM_ERR_LIMIT = -2, F1_TEAM_ERR_INVALID = -3 } F1TeamResult;

typedef struct {
    unsigned char active;
    int salary, start_year, end_year;
} F1DriverContract;

typedef struct {
    int cash, annual_salary_budget, committed_salary, year;
    unsigned int rng;
    unsigned char staff[F1_STAFF_COUNT];
    F1DriverContract contract[F1_TEAM_MAX_DRIVERS];
    int last_event_day, season_event_count;
    char last_event[96];
} F1TeamManagement;

void f1_team_management_new(F1TeamManagement *team, int cash, unsigned int seed);
int f1_team_offer_driver(F1TeamManagement *team, int driver, int salary, int years);
int f1_team_upgrade_staff(F1TeamManagement *team, F1StaffType staff);
int f1_team_roll_event(F1TeamManagement *team, int day, int pressured_context);

#endif
