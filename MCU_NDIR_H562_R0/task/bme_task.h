/*
 * bme_task.h
 *
 *  Created on: Aug 24, 2024
 *      Author: rfacc
 */

#ifndef BME_TASK_H_
#define BME_TASK_H_

#include "stdint.h"

void   bme_task_init();
float  get_bme_temp();
float  get_bme_humidity();
float  get_bme_pressure();


#endif /* BME_TASK_H_ */
