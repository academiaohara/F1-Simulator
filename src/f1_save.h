#ifndef F1_SAVE_H
#define F1_SAVE_H

#include "f1_car.h"
#include "f1_game.h"

#define F1_SAVE_FILE_VERSION 1u

typedef enum {
    F1_SAVE_OK = 0, F1_SAVE_ERR_IO = 1, F1_SAVE_ERR_FORMAT = 2,
    F1_SAVE_ERR_VERSION = 3, F1_SAVE_ERR_CHECKSUM = 4
} F1SaveResult;

typedef struct {
    F1Game game;
    F1CarProgram car;
} F1SaveState;

F1SaveResult f1_save_write(const char *path, const F1SaveState *state);
F1SaveResult f1_save_read(const char *path, F1SaveState *state);

#endif
