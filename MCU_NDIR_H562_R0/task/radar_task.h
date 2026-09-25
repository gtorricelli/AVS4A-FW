#ifndef RADAR_TASK_H
#define RADAR_TASK_H

#include <stdint.h>
#include <stdbool.h>

#include "innosent_protocol.h"

#ifndef RADAR_DEFAULT_SENSOR
#define RADAR_DEFAULT_SENSOR INNOSENT_SENSOR_IMD2000
#endif

#ifndef RADAR_DIAGNOSTIC_RAW_DEFAULT
#define RADAR_DIAGNOSTIC_RAW_DEFAULT 0
#endif

/* Provisional bench defaults. Tune these after collecting field logs. */
#ifndef MIN_CLUSTER_POINTS
#define MIN_CLUSTER_POINTS              4U
#endif

#ifndef RADAR_CLUSTER_RANGE_EPSILON_M
#define RADAR_CLUSTER_RANGE_EPSILON_M   0.50f
#endif

#ifndef RADAR_CLUSTER_VELOCITY_EPSILON_MPS
#define RADAR_CLUSTER_VELOCITY_EPSILON_MPS 0.75f
#endif

#ifndef RADAR_CLUSTER_ANGLE_EPSILON_DEG
#define RADAR_CLUSTER_ANGLE_EPSILON_DEG 15.0f
#endif

void radar_task_init(void);
void radar_task(void);
bool radar_task_select_sensor(innosent_sensor_t sensor);
innosent_sensor_t radar_task_get_sensor(void);
void radar_task_set_raw_diagnostic(bool enabled);
bool radar_task_get_raw_diagnostic(void);

#endif /* RADAR_TASK_H */
