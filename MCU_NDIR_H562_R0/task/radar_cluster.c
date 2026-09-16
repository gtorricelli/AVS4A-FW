#include "radar_cluster.h"

#include <stdbool.h>

static uint8_t visited[RADAR_CLUSTER_MAX_SAMPLES];
static uint16_t component[RADAR_CLUSTER_MAX_SAMPLES];
static uint16_t queue[RADAR_CLUSTER_MAX_SAMPLES];

static float abs_float(float value)
{
    return (value < 0.0f) ? -value : value;
}

static float sqrt_newton(float value)
{
    float estimate;
    uint8_t i;

    if (value <= 0.0f)
    {
        return 0.0f;
    }

    estimate = (value > 1.0f) ? value : 1.0f;
    for (i = 0U; i < 8U; ++i)
    {
        estimate = 0.5f * (estimate + (value / estimate));
    }
    return estimate;
}

static bool samples_are_neighbours(const radar_sample_t *a,
                                   const radar_sample_t *b,
                                   float range_epsilon_m,
                                   float velocity_epsilon_mps)
{
    return (abs_float(a->range_m - b->range_m) <= range_epsilon_m) &&
           (abs_float(a->velocity_mps - b->velocity_mps) <= velocity_epsilon_mps);
}

static radar_cluster_result_t summarize_component(const radar_sample_t *samples,
                                                  const uint16_t *indices,
                                                  size_t count,
                                                  uint8_t frame_count)
{
    float range_sum[RADAR_CLUSTER_MAX_FRAMES] = {0.0f};
    float velocity_sum[RADAR_CLUSTER_MAX_FRAMES] = {0.0f};
    uint16_t points_per_frame[RADAR_CLUSTER_MAX_FRAMES] = {0U};
    radar_cluster_result_t result = {0U, 0U, 0.0f, 0.0f, 0.0f, 0.0f};
    float squared_error_sum = 0.0f;
    uint8_t frame;
    size_t i;

    for (i = 0U; i < count; ++i)
    {
        const radar_sample_t *sample = &samples[indices[i]];
        if (sample->frame_index < RADAR_CLUSTER_MAX_FRAMES)
        {
            range_sum[sample->frame_index] += sample->range_m;
            velocity_sum[sample->frame_index] += sample->velocity_mps;
            ++points_per_frame[sample->frame_index];
        }
    }

    result.total_points = (uint16_t)count;
    result.mean_points = (frame_count > 0U) ? ((float)count / (float)frame_count) : 0.0f;

    for (frame = 0U; (frame < frame_count) && (frame < RADAR_CLUSTER_MAX_FRAMES); ++frame)
    {
        if (points_per_frame[frame] > 0U)
        {
            result.mean_centroid_range_m += range_sum[frame] / (float)points_per_frame[frame];
            result.mean_velocity_mps += velocity_sum[frame] / (float)points_per_frame[frame];
            ++result.frames_seen;
        }
    }

    if (result.frames_seen > 0U)
    {
        result.mean_centroid_range_m /= (float)result.frames_seen;
        result.mean_velocity_mps /= (float)result.frames_seen;
    }

    if (result.frames_seen > 1U)
    {
        for (frame = 0U; (frame < frame_count) && (frame < RADAR_CLUSTER_MAX_FRAMES); ++frame)
        {
            if (points_per_frame[frame] > 0U)
            {
                const float centroid = range_sum[frame] / (float)points_per_frame[frame];
                const float error = centroid - result.mean_centroid_range_m;
                squared_error_sum += error * error;
            }
        }
        result.centroid_range_std_m =
            sqrt_newton(squared_error_sum / (float)(result.frames_seen - 1U));
    }

    return result;
}

static void insert_result_sorted(radar_cluster_result_t result,
                                 radar_cluster_result_t *results,
                                 size_t *result_count,
                                 size_t result_capacity)
{
    size_t position;

    if (result_capacity == 0U)
    {
        return;
    }

    position = *result_count;
    if (position < result_capacity)
    {
        ++(*result_count);
    }
    else
    {
        position = result_capacity - 1U;
        if (result.total_points <= results[position].total_points)
        {
            return;
        }
    }

    while ((position > 0U) && (result.total_points > results[position - 1U].total_points))
    {
        if (position < result_capacity)
        {
            results[position] = results[position - 1U];
        }
        --position;
    }
    results[position] = result;
}

size_t radar_cluster_analyze(const radar_sample_t *samples,
                             size_t sample_count,
                             uint8_t frame_count,
                             uint16_t min_cluster_points,
                             float range_epsilon_m,
                             float velocity_epsilon_mps,
                             radar_cluster_result_t *results,
                             size_t result_capacity)
{
    size_t result_count = 0U;
    size_t seed;

    if ((samples == NULL) || (results == NULL) || (frame_count == 0U) ||
        (sample_count == 0U) || (sample_count > RADAR_CLUSTER_MAX_SAMPLES))
    {
        return 0U;
    }

    for (seed = 0U; seed < sample_count; ++seed)
    {
        visited[seed] = 0U;
    }

    /* Connected components in range/velocity space. This is intentionally
       deterministic and allocation-free for the first field prototype. */
    for (seed = 0U; seed < sample_count; ++seed)
    {
        size_t queue_read = 0U;
        size_t queue_write = 0U;
        size_t component_count = 0U;

        if (visited[seed] != 0U)
        {
            continue;
        }

        visited[seed] = 1U;
        queue[queue_write++] = (uint16_t)seed;

        while (queue_read < queue_write)
        {
            const uint16_t current = queue[queue_read++];
            size_t candidate;

            component[component_count++] = current;
            for (candidate = 0U; candidate < sample_count; ++candidate)
            {
                if ((visited[candidate] == 0U) &&
                    samples_are_neighbours(&samples[current], &samples[candidate],
                                           range_epsilon_m, velocity_epsilon_mps))
                {
                    visited[candidate] = 1U;
                    queue[queue_write++] = (uint16_t)candidate;
                }
            }
        }

        if (component_count >= min_cluster_points)
        {
            const radar_cluster_result_t result =
                summarize_component(samples, component, component_count, frame_count);
            insert_result_sorted(result, results, &result_count, result_capacity);
        }
    }

    return result_count;
}
