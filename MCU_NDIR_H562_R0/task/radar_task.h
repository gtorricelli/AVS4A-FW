#ifndef RADAR_TASK_H
#define RADAR_TASK_H

#include <stdint.h>

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

void radar_task_init(void);
void radar_task(void);

#endif /* RADAR_TASK_H */
