/*
 * sensors_task.h
 *
 *  Created on: Dec 5, 2022
 *      Author: rfacc
 */

#ifndef SENSORS_TASK_H_
#define SENSORS_TASK_H_
#include "stdint.h"
#include "bms.h"

#define CHARGE_SUSPEND      0x00
#define CHARGE_IN_PROGRESS  0x02
#define CHARGE_COMPLETE     0x01
#define CHARGE_ERROR        0x03

void sensors_task_init();
int16_t get_sensor_temp();
int16_t get_sensor_humidity();
uint8_t get_charger_status();
stBatteryData* getBatteryData();


#endif /* SENSORS_TASK_H_ */
