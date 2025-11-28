/*
 * bme_task.c
 *
 *  Created on: Aug 24, 2024
 *      Author: rfacc
 */
#include <bme_task.h>
#include <stdio.h>
#include "KedOS.h"
#include "main.h"
#include "sht3x.h"
#include "hailo_task.h"
#include "pir_task.h"

uint8_t gu8pirSync = 0;

static void PirRtTask();
static void PirMainTask();

static uint32_t debounce_ms            = 5;    // 20 ms debounce window
static uint32_t timeout_event_ms       = 200;   // 300 ms tempo pir fermo per generare evento
static uint32_t timeout_oscuramento_ms = 1000; // 10000 ms tempo pir oscurato dopo evento precedente

static uint8_t  gu8PirValue[4]; // valore letto in irq dei gpio
static uint8_t  gu8PirValueFiltered[4]; // valore con filtraggio
static uint8_t  gu8PirValueFilteredD[4];// valore precedente per identificare cambio livello
static uint8_t  gu8PirValueEvent[4];// flag che indica che ingresso cambiato da basso a alto
static uint8_t  gu8PirDetectEvent[4];
static uint8_t  gu8PirFilterCount[4];//contatore usato per denoise ingressi
static uint32_t gu8PirCounter[4];    //contatore di ms trascorsi da ultimo evento in ingresso
static uint32_t gu8PirCounterOscuramento[4];//contatore di ms trascorsi da ultimo evento segnalato

uint8_t  getPirvalue(uint8_t ch)
{
	if (ch > 3) return 0;
	return gu8PirValue[ch];
}
void pir_task_init()
{
	int i;
	for(i=0;i<4;i++)
	{
		gu8PirValue[i]          = 1;
		gu8PirValueFiltered[i]  = 0;
		gu8PirValueFilteredD[i] = 0;
		gu8PirValueEvent[i]     = 0;
		gu8PirCounter[i]        = 0;
		gu8PirFilterCount[i]    = 0;
		gu8PirDetectEvent[i]    = 0;
	}
	gu8pirSync  = 0;
	add_mainloop_funct(PirMainTask, "pir-taSK\0", 1, 0);
	add_cyclical_funct(PirRtTask, 1, "pir_sync\0",CRITICAL_TASK);
}



static void PirRtTask()
{
	int i;

	gu8pirSync = 1;

	gu8PirValue[0] = HAL_GPIO_ReadPin(PIR1_GPIO_Port, PIR1_Pin);
	gu8PirValue[1] = HAL_GPIO_ReadPin(PIR2_GPIO_Port, PIR2_Pin);
	gu8PirValue[2] = HAL_GPIO_ReadPin(PIR3_GPIO_Port, PIR3_Pin);
	gu8PirValue[3] = HAL_GPIO_ReadPin(PIR4_GPIO_Port, PIR4_Pin);

	for(i=0;i<4;i++)
	{
		if(gu8PirCounter[i]>0)
		{
			gu8PirCounter[i]--;
			if(gu8PirCounter[i]==0)
			{
				gu8PirDetectEvent[i] = 1;
			}
		}
		if(gu8PirCounterOscuramento[i]>0)
		{
			gu8PirCounterOscuramento[i]--;
		}
	}

}

static void PirMainTask()
{
	int i;
	if(gu8pirSync==0) return;

	gu8pirSync = 0;


	for(i=0;i<4;i++)
	{
		if(gu8PirValue[i] == 0)
		{
			if(gu8PirFilterCount[i]>0)
				gu8PirFilterCount[i]--;
		}
		else
		{
			if(gu8PirFilterCount[i]<debounce_ms)
				gu8PirFilterCount[i]++;
		}
		if(gu8PirFilterCount[i] == 0)
		{
			gu8PirValueFiltered[i] = 1;
		}
		if(gu8PirFilterCount[i] == debounce_ms)
		{
			gu8PirValueFiltered[i] = 0;
		}
		if((gu8PirValueFilteredD[i] == 0) && (gu8PirValueFiltered[i] == 1))
		{
			gu8PirValueEvent[i] = 1;
			gu8PirCounter[i]    = timeout_event_ms;
		}
		gu8PirValueFilteredD[i] = gu8PirValueFiltered[i];

		if(gu8PirDetectEvent[i] == 1)
		{
			gu8PirDetectEvent[i]  = 0;
			if(gu8PirCounterOscuramento[i] == 0)
			{
				gu8PirCounterOscuramento[i] = timeout_oscuramento_ms;
				setPirEvent(i);
			}
		}
	}
}


void setDebounce_ms(uint32_t v)
{
	debounce_ms = v;//primo debounce
}
uint32_t getDebounce_ms()
{
	return debounce_ms;
}
void setMin_PIR_ms(uint32_t v)
{
	timeout_event_ms=v;
}

uint32_t getMin_PIR_ms()
{
	return timeout_event_ms;
}

