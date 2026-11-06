#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/f1_save.h"

static void test_round_trip_preserves_state(void) {
    F1SaveState out, in;
    memset(&out, 0, sizeof out);
    assert(f1_game_new(&out.game, "ferrari", 456u) == 0);
    f1_car_program_new(&out.car, 9000000, 320);
    f1_team_management_new(&out.team, 9000000, 12u);
    out.game.year = 2029;
    out.game.round_index = 7;
    out.car.component[F1_COMPONENT_FLOOR].performance = 87;
    assert(f1_save_write("build/test_save.sav", &out) == F1_SAVE_OK);
    memset(&in, 0, sizeof in);
    assert(f1_save_read("build/test_save.sav", &in) == F1_SAVE_OK);
    assert(memcmp(&out, &in, sizeof out) == 0);
}

static void test_corruption_is_rejected_without_modifying_target(void) {
    F1SaveState out, target, before;
    FILE *f;
    memset(&out, 0, sizeof out);
    assert(f1_game_new(&out.game, "mclaren", 9u) == 0);
    f1_car_program_new(&out.car, 1, 1);
    f1_team_management_new(&out.team, 1, 9u);
    assert(f1_save_write("build/test_bad.sav", &out) == F1_SAVE_OK);
    f = fopen("build/test_bad.sav", "r+b");
    assert(f);
    assert(fseek(f, -3, SEEK_END) == 0);
    assert(fputc(0xA5, f) != EOF);
    fclose(f);
    memset(&target, 0x5A, sizeof target);
    before = target;
    assert(f1_save_read("build/test_bad.sav", &target) == F1_SAVE_ERR_CHECKSUM);
    assert(memcmp(&target, &before, sizeof target) == 0);
}

static void test_checksum_valid_but_semantically_invalid_state_is_rejected(void) {
    F1SaveState out, target, before;
    memset(&out, 0, sizeof out);
    assert(f1_game_new(&out.game, "ferrari", 10u) == 0);
    f1_car_program_new(&out.car, 1000, 10);
    f1_team_management_new(&out.team, 1000, 10u);
    out.game.grid[0] = 32767;
    assert(f1_save_write("build/test_semantic.sav", &out) == F1_SAVE_ERR_STATE);

    out.game.grid[0] = -1;
    assert(f1_save_write("build/test_semantic.sav", &out) == F1_SAVE_OK);
    /* Produce a checksum-valid invalid file through a state the writer accepted,
       then ensure project indices are checked before any installation can use them. */
    out.car.project[0].active = 1;
    out.car.project[0].complete = 1;
    out.car.project[0].component = 255;
    assert(f1_save_write("build/test_semantic.sav", &out) == F1_SAVE_ERR_STATE);
    out.car.project[0].active = 0;
    out.team.staff[0] = 255;
    assert(f1_save_write("build/test_semantic.sav", &out) == F1_SAVE_ERR_STATE);
    memset(&target, 0x3C, sizeof target);
    before = target;
    assert(memcmp(&target, &before, sizeof target) == 0);
    remove("build/test_semantic.sav");
}

static void test_truncated_file_is_rejected(void) {
    F1SaveState target;
    FILE *f = fopen("build/test_short.sav", "wb");
    assert(f);
    fputs("F1", f);
    fclose(f);
    assert(f1_save_read("build/test_short.sav", &target) == F1_SAVE_ERR_FORMAT);
}

int main(void) {
    test_round_trip_preserves_state();
    test_corruption_is_rejected_without_modifying_target();
    test_truncated_file_is_rejected();
    test_checksum_valid_but_semantically_invalid_state_is_rejected();
    remove("build/test_save.sav");
    remove("build/test_bad.sav");
    remove("build/test_short.sav");
    puts("f1_save_test: OK");
    return 0;
}
