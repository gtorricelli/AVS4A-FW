#ifndef INNOSENT_PROTOCOL_H
#define INNOSENT_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define INNOSENT_MAX_TARGETS               20U
#define INNOSENT_IMD2000_MAX_TARGETS       20U
#define INNOSENT_IMD2002_MAX_TARGETS       15U
#define INNOSENT_IMD2000_TARGET_FRAME_SIZE 334U
#define INNOSENT_IMD2002_TARGET_FRAME_SIZE 314U
#define INNOSENT_MAX_FRAME_SIZE             INNOSENT_IMD2000_TARGET_FRAME_SIZE

typedef enum
{
    INNOSENT_SENSOR_IMD2000 = 0,
    INNOSENT_SENSOR_IMD2002 = 1
} innosent_sensor_t;

typedef struct
{
    float range_m;
    float velocity_mps;
    float signal_db;
    float eta_s;
    float incident_angle_deg;
    bool angle_valid;
} innosent_target_t;

typedef struct
{
    uint16_t target_count;
    uint16_t target_list_id;
    uint8_t blockage_detected;
    uint8_t blockage_level;
    uint16_t reserved;
    innosent_target_t targets[INNOSENT_MAX_TARGETS];
} innosent_target_list_t;

extern const uint8_t innosent_start_command[];
extern const size_t innosent_start_command_size;
extern const uint8_t innosent_target_request[];
extern const size_t innosent_target_request_size;

const char *innosent_sensor_name(innosent_sensor_t sensor);
size_t innosent_target_frame_size(innosent_sensor_t sensor);
bool innosent_is_start_ack(const uint8_t *frame, size_t size);
bool innosent_decode_targets(innosent_sensor_t sensor,
                             const uint8_t *frame,
                             size_t size,
                             innosent_target_list_t *list);

#endif
