#include <assert.h>
#include <stdio.h>

#include "../src/f1_car.h"

static void test_project_respects_budget_and_atr(void) {
    F1CarProgram car;
    f1_car_program_new(&car, 1000000, 100);
    assert(f1_car_start_project(&car, F1_COMPONENT_FLOOR, 80, 1200000, 20) == F1_CAR_ERR_BUDGET);
    assert(f1_car_start_project(&car, F1_COMPONENT_FLOOR, 80, 500000, 120) == F1_CAR_ERR_ATR);
    assert(car.active_projects == 0);
}

static void test_project_finishes_and_requires_installation(void) {
    F1CarProgram car;
    int slot, before;
    f1_car_program_new(&car, 4000000, 200);
    before = car.component[F1_COMPONENT_FRONT_WING].performance;
    slot = f1_car_start_project(&car, F1_COMPONENT_FRONT_WING, 75, 900000, 30);
    assert(slot >= 0);
    assert(car.component[F1_COMPONENT_FRONT_WING].performance == before);
    f1_car_advance_days(&car, 74);
    assert(!car.project[slot].complete);
    f1_car_advance_days(&car, 1);
    assert(car.project[slot].complete);
    assert(f1_car_install_project(&car, slot) == F1_CAR_OK);
    assert(car.component[F1_COMPONENT_FRONT_WING].performance > before);
}

static void test_diminishing_returns_cap_development(void) {
    F1CarProgram car;
    int low, high;
    f1_car_program_new(&car, 10000000, 500);
    car.component[F1_COMPONENT_REAR_WING].performance = 50;
    low = f1_car_project_gain(&car, F1_COMPONENT_REAR_WING, 100);
    car.component[F1_COMPONENT_REAR_WING].performance = 94;
    high = f1_car_project_gain(&car, F1_COMPONENT_REAR_WING, 100);
    assert(low > high);
    assert(high >= 0);
}

static void test_damage_and_reliability_are_bounded(void) {
    F1CarProgram car;
    f1_car_program_new(&car, 1000000, 100);
    f1_car_apply_damage(&car, F1_COMPONENT_SUSPENSION, 200);
    assert(car.component[F1_COMPONENT_SUSPENSION].condition == 0);
    f1_car_repair(&car, F1_COMPONENT_SUSPENSION, 500);
    assert(car.component[F1_COMPONENT_SUSPENSION].condition == 100);
}

int main(void) {
    test_project_respects_budget_and_atr();
    test_project_finishes_and_requires_installation();
    test_diminishing_returns_cap_development();
    test_damage_and_reliability_are_bounded();
    puts("f1_car_test: OK");
    return 0;
}
