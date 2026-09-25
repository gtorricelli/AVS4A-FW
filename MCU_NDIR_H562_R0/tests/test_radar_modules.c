#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "innosent_protocol.h"
#include "radar_cluster.h"

static void write_be_u16(uint8_t *data, uint16_t value)
{
    data[0] = (uint8_t)(value >> 8U);
    data[1] = (uint8_t)value;
}

static void write_be_float(uint8_t *data, float value)
{
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    data[0] = (uint8_t)(bits >> 24U);
    data[1] = (uint8_t)(bits >> 16U);
    data[2] = (uint8_t)(bits >> 8U);
    data[3] = (uint8_t)bits;
}

static void finalize_frame(uint8_t *frame, size_t size)
{
    uint8_t checksum = 0U;
    size_t i;
    for (i = 1U; i < (size - 2U); ++i)
    {
        checksum = (uint8_t)(checksum + frame[i]);
    }
    frame[size - 2U] = checksum;
    frame[size - 1U] = 0x16U;
}

static void test_protocol(void)
{
    static const uint8_t ack[] = {0x68U, 0x03U, 0x03U, 0x68U, 0x01U,
                                  0x64U, 0xD1U, 0x36U, 0x16U};
    uint8_t frame[INNOSENT_IMD2000_TARGET_FRAME_SIZE] = {0U};
    uint8_t frame2002[INNOSENT_IMD2002_TARGET_FRAME_SIZE] = {0U};
    innosent_target_list_t list;

    assert(innosent_is_start_ack(ack, sizeof(ack)));

    frame[0] = 0xA2U;
    frame[1] = 0x01U;
    frame[2] = 0x64U;
    frame[3] = 0xDAU;
    write_be_u16(&frame[4], 2U);
    write_be_u16(&frame[6], 42U);
    write_be_u16(&frame[8], 7U);
    write_be_float(&frame[12], 3.5f);
    write_be_float(&frame[16], -0.25f);
    write_be_float(&frame[20], 12.0f);
    write_be_float(&frame[24], 4.0f);
    write_be_float(&frame[28], 8.0f);
    write_be_float(&frame[32], 1.25f);
    write_be_float(&frame[36], 9.0f);
    write_be_float(&frame[40], 2.0f);
    finalize_frame(frame, sizeof(frame));

    assert(innosent_decode_targets(INNOSENT_SENSOR_IMD2000, frame, sizeof(frame), &list));
    assert(list.target_count == 2U);
    assert(list.target_list_id == 42U);
    assert(list.blockage_detected == 0U);
    assert(list.blockage_level == 7U);
    assert(fabsf(list.targets[0].range_m - 3.5f) < 0.0001f);
    assert(fabsf(list.targets[0].velocity_mps + 0.25f) < 0.0001f);
    assert(fabsf(list.targets[1].range_m - 8.0f) < 0.0001f);

    frame[100] ^= 0x01U;
    assert(!innosent_decode_targets(INNOSENT_SENSOR_IMD2000, frame, sizeof(frame), &list));

    frame2002[0] = 0xA2U;
    frame2002[1] = 0x01U;
    frame2002[2] = 0x64U;
    frame2002[3] = 0xDAU;
    write_be_u16(&frame2002[4], 1U);
    write_be_u16(&frame2002[6], 77U);
    frame2002[8] = 1U;
    frame2002[9] = 9U;
    write_be_float(&frame2002[12], 4.25f);
    write_be_float(&frame2002[16], -0.75f);
    write_be_float(&frame2002[20], 15.0f);
    write_be_float(&frame2002[24], 5.0f);
    write_be_float(&frame2002[28], -12.5f);
    finalize_frame(frame2002, sizeof(frame2002));

    assert(innosent_decode_targets(INNOSENT_SENSOR_IMD2002,
                                   frame2002, sizeof(frame2002), &list));
    assert(list.target_count == 1U);
    assert(list.target_list_id == 77U);
    assert(list.blockage_detected == 1U);
    assert(list.blockage_level == 9U);
    assert(list.targets[0].angle_valid);
    assert(fabsf(list.targets[0].incident_angle_deg + 12.5f) < 0.0001f);
    assert(!innosent_decode_targets(INNOSENT_SENSOR_IMD2000,
                                    frame2002, sizeof(frame2002), &list));
}

static void test_clustering(void)
{
    const radar_sample_t samples[] = {
        {3.0f, 0.2f, 10.0f, true, 0U}, {3.2f, 0.1f, 12.0f, true, 0U},
        {3.1f, 0.3f, 14.0f, true, 1U}, {3.3f, 0.2f, 16.0f, true, 1U},
        {3.2f, 0.2f, 18.0f, true, 2U}, {3.4f, 0.3f, 20.0f, true, 2U},
        {9.0f, -1.0f, 0.0f, false, 0U}, {9.1f, -1.1f, 0.0f, false, 1U},
        {15.0f, 3.0f, 0.0f, false, 2U}
    };
    radar_cluster_result_t results[4];
    const size_t count = radar_cluster_analyze(samples, 9U, 3U, 2U,
                                                0.5f, 0.5f, 15.0f, results, 4U);

    assert(count == 2U);
    assert(results[0].total_points == 6U);
    assert(results[0].frames_seen == 3U);
    assert(fabsf(results[0].mean_points - 2.0f) < 0.0001f);
    assert(fabsf(results[0].mean_centroid_range_m - 3.2f) < 0.0001f);
    assert(fabsf(results[0].centroid_range_std_m - 0.1f) < 0.002f);
    assert(results[0].angle_valid);
    assert(fabsf(results[0].mean_angle_deg - 15.0f) < 0.0001f);
    assert(fabsf(results[0].angle_std_deg - 4.0f) < 0.01f);
    assert(results[1].total_points == 2U);
}

int main(void)
{
    test_protocol();
    test_clustering();
    puts("radar module tests: OK");
    return 0;
}
