/*
 * Hailo_task.c
 *
 *  Created on: 5 mar 2024
 *      Author: rfacc
 */

#include "hailo_task.h"
#include "KedOS.h"
#include "main.h"
#include "serial_com_task.h"
#include "utility.h"
#include "rtc_task.h"



uint8_t gu8HailoSync  = 0;
HailoStatus gu8HailoState = INIT_POWER_IDLE;


static void hailo_syncTask();
static void hailo_elab_task();
uint8_t isHailoFlagActive();
uint8_t setHailoRequestFlag(uint8_t val);
static int32_t  nTimeOut = 10;
static uint8_t  gbHailoRequest=1;
static uint8_t  gbRtcRequest=0;
static uint8_t  gbLinuxAlive=0;
static uint8_t  g_u8LinuxReadyFlag = 0;
static uint8_t  g_u8HailoReadyFlag = 0;
static uint8_t  l_startuplinux = 0;
static uint8_t  g_DisableTask = 0;
static uint8_t  l_bpirEvent=0;

#define ALIVE_DEFAULT 0 //@ked

static void EnableSOM(GPIO_PinState v)
{
	HAL_GPIO_WritePin(EN_12V_GPIO_Port, EN_12V_Pin, v);
}

void DisableHailoTask(uint8_t value)
{
	g_DisableTask = value;
}

static void Hailo_syncTask();
static void Hailo_elab_task();

void hailo_task_init()
{
	gu8HailoSync  = 0;
	gu8HailoState = INIT_POWER_IDLE;
	gbLinuxAlive  = ALIVE_DEFAULT;
	gbHailoRequest= 1;
	nTimeOut      = 10;
	g_DisableTask = 0;
	//time critical function
	add_cyclical_funct(Hailo_syncTask, 1000, "Hailo_sync\0",CRITICAL_TASK);
	add_mainloop_funct(Hailo_elab_task, "Hailo_state\0", 1, 0);
}

static void Hailo_syncTask()
{
	gu8HailoSync = 1;
}

void setReadyStatus()
{
	g_u8LinuxReadyFlag=1;
}

void resetReadyStatus()
{
	g_u8LinuxReadyFlag=0;
}

void setReadyHailo(uint8_t on)
{
	g_u8HailoReadyFlag=on;
}

uint8_t getReadyHailo()
{
	return g_u8HailoReadyFlag;
}

int GetHailoPhase()
{
	return gu8HailoState;
}

int  GetHailoTimeout()
{
	return nTimeOut;
}

int enwhiteled(uint8_t on)
{
	if(on>0)
	{
		HAL_GPIO_WritePin(EN_WHITE_GPIO_Port, EN_WHITE_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(EN_WHITE_GPIO_Port, EN_WHITE_Pin, GPIO_PIN_RESET);
	}
}

static void Hailo_elab_task()
{
	if(gu8HailoSync == 0) return;
	gu8HailoSync = 0;
	if(nTimeOut>0) nTimeOut--;
	if(g_DisableTask > 0)
	{
//		HAL_GPIO_WritePin(EN_SOM_GPIO_Port, EN_SOM_Pin, GPIO_PIN_SET);
//		HAL_GPIO_WritePin(EN_LTE_PWR_GPIO_Port, EN_LTE_PWR_Pin, GPIO_PIN_SET);
		return;
	}

	switch(gu8HailoState)
	{
	case INIT_POWER_IDLE:
		EnableSOM(GPIO_PIN_RESET);
        rtc_task_init();//avvio del task rtc, one shot
		gu8HailoState = IDLE;
		HAL_Delay(800);
		break;
	case IDLE:
		enwhiteled( GPIO_PIN_RESET);
		if(isPirevent())
		{
			resetPirEvent();
			nTimeOut = SETUP_TIME;
			gu8HailoState = INIT_POWER_SYSTEM;
		}
		break;
	case INIT_POWER_SYSTEM :
		if(nTimeOut==0)//linux non si avvia oppure non risponde alla richiesta di Ready
		{
			if(!isRtcFlagActive())
			{
				segnala_errore(RTCTIMEGETERROR);
				gu8HailoState = IDLE;
			}
			if(!isHailoFlagActive())
			{
				segnala_errore(HAILOFLAG);
				gu8HailoState = IDLE;
			}
			return;
		}
		l_startuplinux=1;
		EnableSOM(GPIO_PIN_SET);
		enwhiteled( GPIO_PIN_RESET);
		gu8HailoState = CHECK_HAILO_WITH_PING;
		nTimeOut = PINGTIMEOUT;
		send_ping();
		break;

	case CHECK_HAILO_WITH_PING:
		if(nTimeOut==0)
		{//linux non si avvia oppure non risponde alla richiesta di Ready
			segnala_errore(LINUXOFFERROR);
			gu8HailoState = LINUX_HAILO_POWEROFF;
			return;
		}
		if(g_u8LinuxReadyFlag == 0)
		{//aspetto che linux si sia attivato
			return;
		}
		if(l_startuplinux)
		{//avvio subito operazioni, linux appena avviato
			gu8HailoState = LINUX_HAILO_ACTIVE;
		 	l_startuplinux=0;
		}
		else
		{//da gestire comandi in arrivo da linux o ritorno a nuovo ping dopo intervallo di tempo
			if(isLinuxAlive()==0)
			{//massimo tempo di attesa con linux attivo senza eventi
				nTimeOut = LINUX_IDLE_MAXTIME;
			    gu8HailoState = IDLE_WAIT_TIMEOUT;
			}
			else
			{//linux sempre attivo
				nTimeOut = PINGTIMEOUT;//ping periodico
				gu8HailoState = IDLE_WAIT;
			}
		}
		break;
	case LINUX_HAILO_ACTIVE:
		//e accende leds dissuasivi
		enwhiteled( GPIO_PIN_SET);
	    //invia dati
		send_sensor_data();
		setReadyHailo (0);
		nTimeOut      = LEDS_FLASH_TIME;
		gu8HailoState = LINUX_HAILO_STOP_LED;
		break;
	case  LINUX_HAILO_STOP_LED:
		//Timeout spengimento leds
		if(nTimeOut> 0)
		{
			return;
		}
		setReadyHailo (1);
		enwhiteled( GPIO_PIN_RESET);
		nTimeOut = PINGTIMEOUT;
		gu8HailoState = CHECK_HAILO_WITH_PING;
		send_ping();
		break;
	case IDLE_WAIT_TIMEOUT:
		if(isPirevent())
		{
			resetPirEvent();
			gu8HailoState = LINUX_HAILO_ACTIVE;
			return;
		}
		if(nTimeOut>0) return;
		request_halt();//time out, chiude linux
		gu8HailoState = LINUX_HAILO_SHUTDOWN;
		nTimeOut=LINUX_STOP_TIME;
		break;
	case IDLE_WAIT:
		if(isPirevent())
		{
			resetPirEvent();
			gu8HailoState = LINUX_HAILO_ACTIVE;
		}
		if(nTimeOut<= 0)
		{//nuovo ping periodico
			gu8HailoState = CHECK_HAILO_WITH_PING;
			send_ping();
		}
		break;
	case LINUX_HAILO_SHUTDOWN:
		if(nTimeOut>0) return;
	//	UART4_MUX_DISABLE();
		HAL_Delay(100);
		//spenge il sistema e attende che sia dato il sincronismo di una nuova accensione dall'rtc_task
		EnableSOM(GPIO_PIN_RESET);
		//HAL_UART_MspDeInit(&huart4);
    	gu8HailoState = LINUX_HAILO_POWEROFF;
		break;
	case LINUX_HAILO_POWEROFF:
    	gu8HailoState = IDLE;
		break;
	}
}

uint8_t setHailoRequestFlag(uint8_t val)
{
	gbHailoRequest = val;
	return val;
}

uint8_t isHailoFlagActive()
{
	uint8_t value  = gbHailoRequest;
	return value;
}

uint8_t setRtcRequestFlag(uint8_t val)
{
	gbRtcRequest = val;
	return val;
}

uint8_t isRtcFlagActive()
{
	uint8_t value  = gbRtcRequest;
	return value;
}



void setPirEvent(int id)
{
	char string[256];
	memset(string,0,256);
	sprintf(string,"Pir Event[%d]\r\n",id);
	outterm_send(string,strlen(string));
	l_bpirEvent=1;
}

void resetPirEvent()
{
	l_bpirEvent=0;
}

uint8_t isPirevent()
{
	return l_bpirEvent;
}

int  setLinuxAlive(uint8_t enable)
{//se abilitato evita di chiudere linux dopo il timeout
	if(enable != 0)enable=1;
	gbLinuxAlive = enable;
	return gbLinuxAlive;
}

int isLinuxAlive()
{
	return gbLinuxAlive;
}


