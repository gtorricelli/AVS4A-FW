#include "innosent_protocol.h"

#include <string.h>

#define INNOSENT_MASTER_ADDRESS 0x01U
#define INNOSENT_RADAR_ADDRESS  0x64U
#define INNOSENT_FC_START       0xD1U
#define INNOSENT_FC_TARGET_LIST 0xDAU
#define INNOSENT_END_DELIMITER  0x16U
#define INNOSENT_TARGET_DATA_OFFSET 12U

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
    uint32_t bits = ((uint32_t)data[0] << 24U) | ((uint32_t)data[1] << 16U) |
                    ((uint32_t)data[2] << 8U) | (uint32_t)data[3];
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
        (size != ((size_t)frame[1] + 6U)) || (frame[size - 1U] != INNOSENT_END_DELIMITER))
    {
        return false;
    }
    for (i = 4U; i < (size - 2U); ++i)
    {
        checksum = (uint8_t)(checksum + frame[i]);
    }
    return checksum == frame[size - 2U];
}

static bool validate_target_frame(const uint8_t *frame, size_t size)
{
    uint8_t checksum = 0U;
    size_t i;
    if ((frame == NULL) || (size < 14U) || (frame[0] != 0xA2U) ||
        (frame[1] != INNOSENT_MASTER_ADDRESS) || (frame[2] != INNOSENT_RADAR_ADDRESS) ||
        (frame[3] != INNOSENT_FC_TARGET_LIST) || (frame[size - 1U] != INNOSENT_END_DELIMITER))
    {
        return false;
    }
    for (i = 1U; i < (size - 2U); ++i)
    {
        checksum = (uint8_t)(checksum + frame[i]);
    }
    return checksum == frame[size - 2U];
}

const char *innosent_sensor_name(innosent_sensor_t sensor)
{
    return (sensor == INNOSENT_SENSOR_IMD2002) ? "IMD2002" : "IMD2000";
}

size_t innosent_target_frame_size(innosent_sensor_t sensor)
{
    return (sensor == INNOSENT_SENSOR_IMD2002) ?
           INNOSENT_IMD2002_TARGET_FRAME_SIZE : INNOSENT_IMD2000_TARGET_FRAME_SIZE;
}

bool innosent_is_start_ack(const uint8_t *frame, size_t size)
{
    return validate_variable_frame(frame, size) && (frame[4] == INNOSENT_MASTER_ADDRESS) &&
           (frame[5] == INNOSENT_RADAR_ADDRESS) && (frame[6] == INNOSENT_FC_START);
}

bool innosent_decode_targets(innosent_sensor_t sensor, const uint8_t *frame,
                             size_t size, innosent_target_list_t *list)
{
    const uint16_t max_targets = (sensor == INNOSENT_SENSOR_IMD2002) ?
                                 INNOSENT_IMD2002_MAX_TARGETS : INNOSENT_IMD2000_MAX_TARGETS;
    const size_t record_size = (sensor == INNOSENT_SENSOR_IMD2002) ? 20U : 16U;
    uint16_t target_count;
    size_t i;

    if ((list == NULL) || (size != innosent_target_frame_size(sensor)) ||
        !validate_target_frame(frame, size))
    {
        return false;
    }
    target_count = read_be_u16(&frame[4]);
    if (target_count > max_targets)
    {
        return false;
    }

    memset(list, 0, sizeof(*list));
    list->target_count = target_count;
    list->target_list_id = read_be_u16(&frame[6]);
    list->blockage_detected = frame[8];
    list->blockage_level = frame[9];
    list->reserved = read_be_u16(&frame[10]);

    for (i = 0U; i < target_count; ++i)
    {
        const uint8_t *record = &frame[INNOSENT_TARGET_DATA_OFFSET + (i * record_size)];
        innosent_target_t *target = &list->targets[i];
        if (!read_be_float(&record[0], &target->range_m) ||
            !read_be_float(&record[4], &target->velocity_mps) ||
            !read_be_float(&record[8], &target->signal_db) ||
            !read_be_float(&record[12], &target->eta_s))
        {
            return false;
        }
        target->angle_valid = (sensor == INNOSENT_SENSOR_IMD2002);
        if (target->angle_valid && !read_be_float(&record[16], &target->incident_angle_deg))
        {
            return false;
        }
    }
    return true;
}
