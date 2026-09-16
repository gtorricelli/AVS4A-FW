#include "radar_task.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "innosent_protocol.h"
#include "kedOS.h"
#include "main.h"
#include "radar_cluster.h"
#include "usart.h"

#define RADAR_RX_RING_SIZE           1024U
#define RADAR_INTERBYTE_TIMEOUT_MS     10U
#define RADAR_RESPONSE_TIMEOUT_MS     200U
#define RADAR_BOOT_DELAY_MS           200U
#define RADAR_REPORT_PERIOD_MS       1000U
#define RADAR_MAX_RETRIES               3U
#define RADAR_MIN_RANGE_M             0.1f
#define RADAR_MAX_RANGE_M            50.0f
#define RADAR_MAX_ABS_VELOCITY_MPS    8.0f

typedef enum
{
    RADAR_STATE_BOOT_DELAY = 0,
    RADAR_STATE_WAIT_START_ACK,
    RADAR_STATE_WAIT_TARGET_LIST
} radar_state_t;

typedef struct
{
    uint8_t data[INNOSENT_IMD2000_TARGET_FRAME_SIZE];
    size_t length;
    size_t expected_length;
    uint32_t last_byte_ms;
} radar_stream_parser_t;

static volatile uint8_t rx_ring[RADAR_RX_RING_SIZE];
static volatile uint16_t rx_head;
static volatile uint16_t rx_tail;
static volatile uint32_t rx_overruns;
static volatile uint32_t uart_errors;
static uint8_t rx_byte;

static radar_stream_parser_t parser;
static radar_state_t radar_state;
static uint32_t state_started_ms;
static uint32_t window_started_ms;
static uint8_t retry_count;
static bool have_last_list_id;
static uint16_t last_list_id;
static uint32_t lost_lists_window;
static uint32_t invalid_frames_window;
static radar_sample_t window_samples[RADAR_CLUSTER_MAX_SAMPLES];
static size_t window_sample_count;
static uint8_t window_frame_count;

static void parser_reset(void)
{
    parser.length = 0U;
    parser.expected_length = 0U;
}

static bool ring_pop(uint8_t *value)
{
    if (rx_tail == rx_head)
    {
        return false;
    }

    *value = rx_ring[rx_tail];
    rx_tail = (uint16_t)((rx_tail + 1U) % RADAR_RX_RING_SIZE);
    return true;
}

static void send_command(const uint8_t *command, size_t size)
{
    (void)HAL_UART_Transmit(&huart1, (uint8_t *)command, (uint16_t)size, 10U);
}

static void send_start_command(uint32_t now)
{
    send_command(innosent_start_command, innosent_start_command_size);
    radar_state = RADAR_STATE_WAIT_START_ACK;
    state_started_ms = now;
}

static void request_target_list(uint32_t now)
{
    send_command(innosent_target_request, innosent_target_request_size);
    radar_state = RADAR_STATE_WAIT_TARGET_LIST;
    state_started_ms = now;
}

static bool target_is_usable(const innosent_imd2000_target_t *target)
{
    return (target->range_m >= RADAR_MIN_RANGE_M) &&
           (target->range_m <= RADAR_MAX_RANGE_M) &&
           (target->velocity_mps >= -RADAR_MAX_ABS_VELOCITY_MPS) &&
           (target->velocity_mps <= RADAR_MAX_ABS_VELOCITY_MPS);
}

static void collect_target_list(const innosent_imd2000_target_list_t *list)
{
    uint16_t i;

    if (have_last_list_id)
    {
        const uint16_t delta = (uint16_t)(list->target_list_id - last_list_id);
        if (delta > 1U)
        {
            lost_lists_window += (uint16_t)(delta - 1U);
        }
    }
    have_last_list_id = true;
    last_list_id = list->target_list_id;

    if (window_frame_count >= RADAR_CLUSTER_MAX_FRAMES)
    {
        return;
    }

    for (i = 0U; i < list->target_count; ++i)
    {
        if ((window_sample_count < RADAR_CLUSTER_MAX_SAMPLES) &&
            target_is_usable(&list->targets[i]))
        {
            radar_sample_t *sample = &window_samples[window_sample_count++];
            sample->range_m = list->targets[i].range_m;
            sample->velocity_mps = list->targets[i].velocity_mps;
            sample->frame_index = window_frame_count;
        }
    }
    ++window_frame_count;
}

static void process_complete_frame(const uint8_t *frame, size_t size, uint32_t now)
{
    if ((frame[0] == 0x68U) && innosent_is_start_ack(frame, size))
    {
        retry_count = 0U;
        have_last_list_id = false;
        request_target_list(now);
    }
    else if (frame[0] == 0xA2U)
    {
        innosent_imd2000_target_list_t list;

        if (innosent_decode_imd2000_targets(frame, size, &list))
        {
            retry_count = 0U;
            collect_target_list(&list);
            request_target_list(now);
        }
        else
        {
            ++invalid_frames_window;
        }
    }
}

static void parser_consume(uint8_t byte, uint32_t now)
{
    if ((parser.length > 0U) &&
        ((uint32_t)(now - parser.last_byte_ms) > RADAR_INTERBYTE_TIMEOUT_MS))
    {
        parser_reset();
        ++invalid_frames_window;
    }

    if (parser.length == 0U)
    {
        if ((byte != 0x68U) && (byte != 0xA2U))
        {
            return;
        }
        parser.expected_length = (byte == 0xA2U) ? INNOSENT_IMD2000_TARGET_FRAME_SIZE : 0U;
    }

    parser.data[parser.length++] = byte;
    parser.last_byte_ms = now;

    if ((parser.data[0] == 0x68U) && (parser.length == 2U))
    {
        parser.expected_length = (size_t)parser.data[1] + 6U;
        if ((parser.expected_length < 9U) ||
            (parser.expected_length > INNOSENT_IMD2000_TARGET_FRAME_SIZE))
        {
            parser_reset();
            ++invalid_frames_window;
            return;
        }
    }

    if ((parser.expected_length > 0U) && (parser.length == parser.expected_length))
    {
        process_complete_frame(parser.data, parser.length, now);
        parser_reset();
    }
    else if (parser.length >= sizeof(parser.data))
    {
        parser_reset();
        ++invalid_frames_window;
    }
}

static void print_window_report(uint32_t elapsed_ms)
{
    radar_cluster_result_t clusters[RADAR_CLUSTER_MAX_RESULTS];
    const size_t cluster_count =
        radar_cluster_analyze(window_samples, window_sample_count, window_frame_count,
                              MIN_CLUSTER_POINTS, RADAR_CLUSTER_RANGE_EPSILON_M,
                              RADAR_CLUSTER_VELOCITY_EPSILON_MPS, clusters,
                              RADAR_CLUSTER_MAX_RESULTS);
    size_t i;

    printf("RADAR sensor=IMD2000 window_ms=%lu frames=%u lost=%lu invalid=%lu "
           "uart_errors=%lu rx_overruns=%lu clusters=%u min_points=%u\r\n",
           (unsigned long)elapsed_ms, (unsigned int)window_frame_count,
           (unsigned long)lost_lists_window, (unsigned long)invalid_frames_window,
           (unsigned long)uart_errors, (unsigned long)rx_overruns,
           (unsigned int)cluster_count, (unsigned int)MIN_CLUSTER_POINTS);

    for (i = 0U; i < cluster_count; ++i)
    {
        printf("CLUSTER id=%u points_mean=%.2f range_mean_m=%.3f range_sd_m=%.3f "
               "velocity_mean_mps=%.3f frames_seen=%u total_points=%u\r\n",
               (unsigned int)i, (double)clusters[i].mean_points,
               (double)clusters[i].mean_centroid_range_m,
               (double)clusters[i].centroid_range_std_m,
               (double)clusters[i].mean_velocity_mps,
               (unsigned int)clusters[i].frames_seen,
               (unsigned int)clusters[i].total_points);
    }

    window_sample_count = 0U;
    window_frame_count = 0U;
    lost_lists_window = 0U;
    invalid_frames_window = 0U;
}

void radar_task_init(void)
{
    const uint32_t now = get_clock_ms();

    rx_head = 0U;
    rx_tail = 0U;
    rx_overruns = 0U;
    uart_errors = 0U;
    retry_count = 0U;
    have_last_list_id = false;
    window_sample_count = 0U;
    window_frame_count = 0U;
    lost_lists_window = 0U;
    invalid_frames_window = 0U;
    parser_reset();
    radar_state = RADAR_STATE_BOOT_DELAY;
    state_started_ms = now;
    window_started_ms = now;

    (void)HAL_UART_Receive_IT(&huart1, &rx_byte, 1U);
    add_mainloop_funct(radar_task, "Radar", 0U, NO_CRITICAL_TASK);
}

void radar_task(void)
{
    uint8_t byte;
    const uint32_t now = get_clock_ms();

    while (ring_pop(&byte))
    {
        parser_consume(byte, now);
    }

    if ((radar_state == RADAR_STATE_BOOT_DELAY) &&
        ((uint32_t)(now - state_started_ms) >= RADAR_BOOT_DELAY_MS))
    {
        send_start_command(now);
    }
    else if ((radar_state != RADAR_STATE_BOOT_DELAY) &&
             ((uint32_t)(now - state_started_ms) >= RADAR_RESPONSE_TIMEOUT_MS))
    {
        ++retry_count;
        if (retry_count >= RADAR_MAX_RETRIES)
        {
            retry_count = 0U;
            send_start_command(now);
        }
        else if (radar_state == RADAR_STATE_WAIT_START_ACK)
        {
            send_start_command(now);
        }
        else
        {
            request_target_list(now);
        }
    }

    if ((uint32_t)(now - window_started_ms) >= RADAR_REPORT_PERIOD_MS)
    {
        print_window_report((uint32_t)(now - window_started_ms));
        window_started_ms = now;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        const uint16_t next = (uint16_t)((rx_head + 1U) % RADAR_RX_RING_SIZE);

        if (next != rx_tail)
        {
            rx_ring[rx_head] = rx_byte;
            rx_head = next;
        }
        else
        {
            ++rx_overruns;
        }

        (void)HAL_UART_Receive_IT(&huart1, &rx_byte, 1U);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        ++uart_errors;
        (void)HAL_UART_Receive_IT(&huart1, &rx_byte, 1U);
    }
}
