#ifndef F1_DATA_H
#define F1_DATA_H

#define F1_ID_LEN 32
#define F1_NAME_LEN 64
#define F1_MAX_TRACK_POINTS 64

typedef struct { short x, y; } F1TrackPoint;

typedef struct {
    char id[F1_ID_LEN];
    char first_name[24], last_name[32], code[4], nationality[24];
    char team_id[F1_ID_LEN];
    unsigned char number, pace, qualifying, racecraft, consistency;
    unsigned char tyre_management, wet_skill, potential;
} F1DriverDef;

typedef struct {
    char id[F1_ID_LEN], name[F1_NAME_LEN], code[4];
    char country[24], base[32], power_unit_id[F1_ID_LEN];
    unsigned int color_primary, color_secondary;
    unsigned char reputation;
} F1TeamDef;

typedef struct {
    char id[F1_ID_LEN], name[F1_NAME_LEN], country[24];
    float length_km, pit_loss_seconds, safety_car_probability, rain_probability;
    unsigned char laps, overtaking, tyre_degradation, point_count;
    F1TrackPoint points[F1_MAX_TRACK_POINTS];
} F1CircuitDef;

typedef struct {
    char id[16], circuit_id[F1_ID_LEN], name[F1_NAME_LEN];
    unsigned char number, sprint;
} F1RoundDef;

extern const F1DriverDef F1_DRIVERS[];
extern const F1TeamDef F1_TEAMS[];
extern const F1CircuitDef F1_CIRCUITS[];
extern const F1RoundDef F1_ROUNDS[];
extern const int F1_DRIVER_COUNT, F1_TEAM_COUNT, F1_CIRCUIT_COUNT, F1_ROUND_COUNT;

const F1DriverDef *f1_driver_by_id(const char *id);
const F1TeamDef *f1_team_by_id(const char *id);
const F1CircuitDef *f1_circuit_by_id(const char *id);
int f1_data_validate(void);

#endif
