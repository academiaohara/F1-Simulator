#ifndef F1_AI_H
#define F1_AI_H

#include "f1_race.h"

/* accuracy is 0..1 and controls estimate noise, never access to future state. */
void f1_ai_update(F1Race *race, int car_index, float accuracy);
void f1_ai_update_all(F1Race *race, const char *human_team_id, float accuracy);

#endif
