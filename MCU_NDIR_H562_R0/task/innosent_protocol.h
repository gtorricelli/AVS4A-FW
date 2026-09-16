#ifndef INNOSENT_PROTOCOL_H
#define INNOSENT_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define INNOSENT_IMD2000_MAX_TARGETS        20U
#define INNOSENT_IMD2000_TARGET_FRAME_SIZE 334U

typedef struct
{
    float range_m;
    float velocity_mps;
    float signal_db;
    float eta_s;
} innosent_imd2000_target_t;

typedef struct
{
    uint16_t target_count;
    uint16_t target_list_id;
    uint16_t blockage;
    innosent_imd2000_target_t targets[INNOSENT_IMD2000_MAX_TARGETS];
} innosent_imd2000_target_list_t;

extern const uint8_t innosent_start_command[];
extern const size_t innosent_start_command_size;
extern const uint8_t innosent_target_request[];
extern const size_t innosent_target_request_size;

bool innosent_is_start_ack(const uint8_t *frame, size_t size);
bool innosent_decode_imd2000_targets(const uint8_t *frame,
                                     size_t size,
                                     innosent_imd2000_target_list_t *list);

#endif /* INNOSENT_PROTOCOL_H */
