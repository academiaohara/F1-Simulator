#include "f1_car.h"

#include <string.h>

static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

void f1_car_program_new(F1CarProgram *car, int budget, int atr_hours) {
    int i;
    if (!car) return;
    memset(car, 0, sizeof *car);
    car->budget = budget;
    car->atr_hours = atr_hours;
    for (i = 0; i < F1_COMPONENT_COUNT; ++i) {
        car->component[i].performance = 60;
        car->component[i].reliability = 75;
        car->component[i].condition = 100;
        car->component[i].specification = 1;
    }
}

int f1_car_project_gain(const F1CarProgram *car, F1ComponentType component, int effort) {
    int current, raw, headroom;
    if (!car || component < 0 || component >= F1_COMPONENT_COUNT || effort <= 0) return 0;
    current = car->component[component].performance;
    raw = clampi(effort / 10, 1, 12);
    headroom = 100 - current;
    return clampi((raw * headroom + 49) / 50, 0, headroom);
}

int f1_car_start_project(F1CarProgram *car, F1ComponentType component,
                         int days, int cost, int atr_cost) {
    int i;
    F1CarProject *p;
    if (!car || component < 0 || component >= F1_COMPONENT_COUNT || days <= 0 || cost < 0 || atr_cost < 0)
        return F1_CAR_ERR_INVALID;
    if (cost > car->budget) return F1_CAR_ERR_BUDGET;
    if (atr_cost > car->atr_hours) return F1_CAR_ERR_ATR;
    for (i = 0; i < F1_CAR_PROJECTS && car->project[i].active; ++i) {}
    if (i == F1_CAR_PROJECTS) return F1_CAR_ERR_CAPACITY;
    p = &car->project[i];
    memset(p, 0, sizeof *p);
    p->active = 1;
    p->component = (unsigned char)component;
    p->days_total = days;
    p->cost = cost;
    p->atr_cost = atr_cost;
    p->gain = (unsigned char)f1_car_project_gain(car, component, atr_cost + days);
    car->budget -= cost;
    car->atr_hours -= atr_cost;
    ++car->active_projects;
    return i;
}

void f1_car_advance_days(F1CarProgram *car, int days) {
    int i;
    if (!car || days <= 0) return;
    for (i = 0; i < F1_CAR_PROJECTS; ++i) if (car->project[i].active && !car->project[i].complete) {
        car->project[i].days_done = clampi(car->project[i].days_done + days, 0, car->project[i].days_total);
        if (car->project[i].days_done == car->project[i].days_total) car->project[i].complete = 1;
    }
}

int f1_car_install_project(F1CarProgram *car, int slot) {
    F1CarProject *p;
    F1CarComponent *c;
    if (!car || slot < 0 || slot >= F1_CAR_PROJECTS) return F1_CAR_ERR_INVALID;
    p = &car->project[slot];
    if (!p->active || !p->complete) return F1_CAR_ERR_INVALID;
    c = &car->component[p->component];
    c->performance = (unsigned char)clampi(c->performance + p->gain, 0, 100);
    ++c->specification;
    memset(p, 0, sizeof *p);
    --car->active_projects;
    return F1_CAR_OK;
}

void f1_car_apply_damage(F1CarProgram *car, F1ComponentType component, int amount) {
    if (!car || component < 0 || component >= F1_COMPONENT_COUNT || amount <= 0) return;
    car->component[component].condition = (unsigned char)clampi(car->component[component].condition - amount, 0, 100);
}

void f1_car_repair(F1CarProgram *car, F1ComponentType component, int amount) {
    if (!car || component < 0 || component >= F1_COMPONENT_COUNT || amount <= 0) return;
    car->component[component].condition = (unsigned char)clampi(car->component[component].condition + amount, 0, 100);
}

int f1_car_overall(const F1CarProgram *car) {
    int i, total = 0;
    if (!car) return 0;
    for (i = 0; i < F1_COMPONENT_COUNT; ++i)
        total += car->component[i].performance * car->component[i].condition / 100;
    return total / F1_COMPONENT_COUNT;
}
