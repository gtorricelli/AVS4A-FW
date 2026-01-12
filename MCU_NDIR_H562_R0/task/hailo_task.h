/*
 * hailo_task.h
 *
 *  Created on: 5 mar 2024
 *      Author: rfacc
 */

#ifndef HAILO_TASK_H_
#define HAILO_TASK_H_
#include "main.h"


void    setReadyStatus();
void 	resetReadyStatus();
int     send_sensor_data(void);
int     request_flash(void);




void    hailo_task_init();


uint8_t setRtcRequestFlag(uint8_t val);
uint8_t isRtcFlagActive();

int     isLinuxAlive();
int     setLinuxAlive(uint8_t enable);
void    setPirEvent(int id);
uint8_t isPirevent();
void    resetPirEvent();



#define LINUX_START_TIMEOUT	60
#define LINUX_IDLE_MAXTIME	80
#define SETUP_TIME	        2
//tempo in secondi:
#define LEDS_FLASH_TIME	12
#define LINUX_STOP_TIME		15
#define LOOP_ERROR_DELAY	(1*60)

typedef enum {
	INIT_SYSTEM          = 0,//avvio
	SYSTEM_IDLE          = 1,//aspetto eventi pir isPirEventActive() per accendere som
	INIT_POWER_SYSTEM    = 2,//stato di attesa condizioni
	CHECK_HAILO_STARTED  = 3,//invio  comando ping ed aspetto ack, se arriva ack vado in stato LINUX_HAILO_ACTIVE
	LINUX_HAILO_ACTIVE   = 4,//se arriva evento pir invio comando phot altrimenti ho timeout di INATTIVITA_TIMEOUT che se scade vadi in spegimenti
	LINUX_HAILO_POWEROFF = 5,//aspetto POWEROFF_TIMEOUT e spengo SOM
} HailoStatus;


#endif /* HAILO_TASK_H_ */
