#ifndef F1_CAR_H
#define F1_CAR_H

#define F1_CAR_PROJECTS 3

typedef enum {
    F1_COMPONENT_FRONT_WING, F1_COMPONENT_REAR_WING, F1_COMPONENT_FLOOR,
    F1_COMPONENT_DIFFUSER, F1_COMPONENT_CHASSIS, F1_COMPONENT_SUSPENSION,
    F1_COMPONENT_COOLING, F1_COMPONENT_BRAKES, F1_COMPONENT_GEARBOX,
    F1_COMPONENT_ENGINE, F1_COMPONENT_ERS, F1_COMPONENT_RELIABILITY,
    F1_COMPONENT_COUNT
} F1ComponentType;

typedef enum {
    F1_CAR_OK = 0, F1_CAR_ERR_BUDGET = -1, F1_CAR_ERR_ATR = -2,
    F1_CAR_ERR_CAPACITY = -3, F1_CAR_ERR_INVALID = -4
} F1CarResult;

typedef struct {
    unsigned char performance, reliability, condition, specification;
} F1CarComponent;

typedef struct {
    unsigned char active, complete, component, gain;
    int days_total, days_done, cost, atr_cost;
} F1CarProject;

typedef struct {
    F1CarComponent component[F1_COMPONENT_COUNT];
    F1CarProject project[F1_CAR_PROJECTS];
    int budget, atr_hours, active_projects;
} F1CarProgram;

void f1_car_program_new(F1CarProgram *car, int budget, int atr_hours);
int f1_car_project_gain(const F1CarProgram *car, F1ComponentType component, int effort);
int f1_car_start_project(F1CarProgram *car, F1ComponentType component,
                         int days, int cost, int atr_cost);
void f1_car_advance_days(F1CarProgram *car, int days);
int f1_car_install_project(F1CarProgram *car, int slot);
void f1_car_apply_damage(F1CarProgram *car, F1ComponentType component, int amount);
void f1_car_repair(F1CarProgram *car, F1ComponentType component, int amount);
int f1_car_overall(const F1CarProgram *car);

#endif
