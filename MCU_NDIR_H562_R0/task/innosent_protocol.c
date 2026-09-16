#include "innosent_protocol.h"

#include <string.h>

#define INNOSENT_MASTER_ADDRESS 0x01U
#define INNOSENT_RADAR_ADDRESS  0x64U
#define INNOSENT_FC_START       0xD1U
#define INNOSENT_FC_TARGET_LIST 0xDAU
#define INNOSENT_END_DELIMITER  0x16U

const uint8_t innosent_start_command[] =
    {0x68U, 0x05U, 0x05U, 0x68U, 0x64U, 0x01U, 0xD1U, 0x00U, 0x00U, 0x36U, 0x16U};
const size_t innosent_start_command_size = sizeof(innosent_start_command);

const uint8_t innosent_target_request[] =
    {0x68U, 0x03U, 0x03U, 0x68U, 0x64U, 0x01U, 0xDAU, 0x3FU, 0x16U};
const size_t innosent_target_request_size = sizeof(innosent_target_request);

static uint16_t read_be_u16(const uint8_t *data)
{
    return (uint16_t)(((uint16_t)data[0] << 8U) | data[1]);
}

static bool read_be_float(const uint8_t *data, float *value)
{
    uint32_t bits = ((uint32_t)data[0] << 24U) |
                    ((uint32_t)data[1] << 16U) |
                    ((uint32_t)data[2] << 8U) |
                    (uint32_t)data[3];

    /* Reject NaN and infinity before copying the IEEE-754 bit pattern. */
    if ((bits & 0x7F800000UL) == 0x7F800000UL)
    {
        return false;
    }

    memcpy(value, &bits, sizeof(bits));
    return true;
}

static bool validate_variable_frame(const uint8_t *frame, size_t size)
{
    uint8_t checksum = 0U;
    size_t i;

    if ((frame == NULL) || (size < 9U) || (frame[0] != 0x68U) ||
        (frame[1] != frame[2]) || (frame[3] != 0x68U) ||
        (size != ((size_t)frame[1] + 6U)) ||
        (frame[size - 1U] != INNOSENT_END_DELIMITER))
    {
        return false;
    }

    for (i = 4U; i < (size - 2U); ++i)
    {
        checksum = (uint8_t)(checksum + frame[i]);
    }

    return checksum == frame[size - 2U];
}

bool innosent_is_start_ack(const uint8_t *frame, size_t size)
{
    return validate_variable_frame(frame, size) &&
           (frame[4] == INNOSENT_MASTER_ADDRESS) &&
           (frame[5] == INNOSENT_RADAR_ADDRESS) &&
           (frame[6] == INNOSENT_FC_START);
}

bool innosent_decode_imd2000_targets(const uint8_t *frame,
                                     size_t size,
                                     innosent_imd2000_target_list_t *list)
{
    uint8_t checksum = 0U;
    uint16_t target_count;
    size_t i;

    if ((frame == NULL) || (list == NULL) ||
        (size != INNOSENT_IMD2000_TARGET_FRAME_SIZE) ||
        (frame[0] != 0xA2U) ||
        (frame[1] != INNOSENT_MASTER_ADDRESS) ||
        (frame[2] != INNOSENT_RADAR_ADDRESS) ||
        (frame[3] != INNOSENT_FC_TARGET_LIST) ||
        (frame[size - 1U] != INNOSENT_END_DELIMITER))
    {
        return false;
    }

    for (i = 1U; i < (size - 2U); ++i)
    {
        checksum = (uint8_t)(checksum + frame[i]);
    }
    if (checksum != frame[size - 2U])
    {
        return false;
    }

    target_count = read_be_u16(&frame[4]);
    if (target_count > INNOSENT_IMD2000_MAX_TARGETS)
    {
        return false;
    }

    list->target_count = target_count;
    list->target_list_id = read_be_u16(&frame[6]);
    list->blockage = read_be_u16(&frame[8]);

    for (i = 0U; i < target_count; ++i)
    {
        const uint8_t *record = &frame[12U + (i * 16U)];

        if (!read_be_float(&record[0], &list->targets[i].range_m) ||
            !read_be_float(&record[4], &list->targets[i].velocity_mps) ||
            !read_be_float(&record[8], &list->targets[i].signal_db) ||
            !read_be_float(&record[12], &list->targets[i].eta_s))
        {
            return false;
        }
    }

    return true;
}
