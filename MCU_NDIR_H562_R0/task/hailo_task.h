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
void    setPirEvent(int id);
uint8_t isPirevent();
void    resetPirEvent();



#define LINUX_START_TIMEOUT	60
#define SETUP_TIME	2
#define PINGTIMEOUT 60  //periodo di ping
//tempo in secondi:
#define LEDS_FLASH_TIME	12
#define LINUX_STOP_TIME		15
#define LOOP_ERROR_DELAY	(1*60)
#define LINUX_IDLE_MAXTIME		80

typedef enum {
	INIT_POWER_IDLE      = 0,//avvio
	IDLE                 = 1,//aspetto eventi pir isPirEventActive() per accendere som
	INIT_POWER_SYSTEM    = 2,//stato di attesa condizioni
	CHECK_HAILO_WITH_PING= 3,//invio  comando ping ed aspetto ack, se arriva ack vado in stato LINUX_HAILO_ACTIVE
	LINUX_HAILO_ACTIVE   = 5,//se arriva evento pir invio comando phot altrimenti ho timeout di INATTIVITA_TIMEOUT che se scade vadi in spegimenti
	LINUX_HAILO_STOP_LED = 6,//spegnimento led
	IDLE_WAIT_TIMEOUT    = 7,//attesa fine di idle con guardia
	IDLE_WAIT            = 8,//linux attivo alive
	LINUX_HAILO_SHUTDOWN = 9,//invio nel passaggio precedente comando di shutdown e faccio il ping in attesa che smetta di rispondermi quando non risponde vado in stato di poweroff
	LINUX_HAILO_POWEROFF = 10,//aspetto POWEROFF_TIMEOUT e spengo SOM
} HailoStatus;


#endif /* HAILO_TASK_H_ */
