#ifndef RADAR_CLUSTER_H
#define RADAR_CLUSTER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define RADAR_CLUSTER_MAX_SAMPLES 256U
#define RADAR_CLUSTER_MAX_FRAMES   16U
#define RADAR_CLUSTER_MAX_RESULTS  32U

typedef struct
{
    float range_m;
    float velocity_mps;
    float angle_deg;
    bool angle_valid;
    uint8_t frame_index;
} radar_sample_t;

typedef struct
{
    uint16_t total_points;
    uint16_t frames_seen;
    float mean_points;
    float mean_centroid_range_m;
    float centroid_range_std_m;
    float mean_velocity_mps;
    float mean_angle_deg;
    float angle_std_deg;
    bool angle_valid;
} radar_cluster_result_t;

size_t radar_cluster_analyze(const radar_sample_t *samples,
                             size_t sample_count,
                             uint8_t frame_count,
                             uint16_t min_cluster_points,
                             float range_epsilon_m,
                             float velocity_epsilon_mps,
                             float angle_epsilon_deg,
                             radar_cluster_result_t *results,
                             size_t result_capacity);

#endif /* RADAR_CLUSTER_H */
