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

#include "pir_task.h"



uint8_t gu8pirSync = 0;

static void PirRtTask();
static void PirMainTask();
typedef enum {
	INIT_PIRS				,
	CHECKINPUTS				,
	CHECKDEBOUNCE			,
	CHECKCOUNT				,
	PIRS_LAST				,
} _PirStatus;

static _PirStatus stPirsState    = INIT_PIRS;
static _PirStatus stPirsStateOld = PIRS_LAST;

static uint8_t     pir1_debouncing=0;
static uint8_t     pir2_debouncing=0;
static uint8_t     pir3_debouncing=0;
static uint8_t     pir4_debouncing=0;
static uint8_t     pir1_time_counting=0;
static uint8_t     pir2_time_counting=0;
static uint8_t     pir3_time_counting=0;
static uint8_t     pir4_time_counting=0;
static uint32_t    pir1_ms_time=0;
static uint32_t    pir2_ms_time=0;
static uint32_t    pir3_ms_time=0;
static uint32_t    pir4_ms_time=0;
static uint32_t    min_PIR_ms=2000;//2 sec.

static uint32_t debounce_ms = 20; // 20 ms debounce window

//
//
//uint8_t startwatchPIR(TPirnum pirnum)
//{
//  if(pirnum<PIR1||pirnum>PIR4)
//	  return 0;
//  switch(pirnum)
//  {
//  case PIR1:
//	  pir1_ms_time=0;
//	  pir1_time_counting=1;
//	  break;
//  case PIR2:
//	  pir2_ms_time=0;
//	  pir2_time_counting=1;
//	  break;
//  case PIR3:
//	  pir3_ms_time=0;
//	  pir3_time_counting=1;
//	  break;
//  case PIR4:
//	  pir4_ms_time=0;
//	  pir4_time_counting=1;
//	  break;
//  default:
//	  return 0;
//  }
//  return 1;
//}

//uint8_t stopwatchPIR(TPirnum pirnum)
//{
//  if(pirnum<PIR1||pirnum>PIR4)
//	  return 0;
//  switch(pirnum)
//  {
//  case PIR1:
//	  pir1_time_counting=0;
//	  break;
//  case PIR2:
//	  pir2_time_counting=0;
//	  break;
//  case PIR3:
//	  pir3_time_counting=0;
//	  break;
//  case PIR4:
//	  pir4_time_counting=0;
//	  break;
//  default:
//	  return 0;
//  }
//  return 1;
//}

void pir_task_init()
{
	gu8pirSync  = 0;
	//time critical function
	stPirsState    = INIT_PIRS;
	add_mainloop_funct(PirMainTask, "pir-taSK\0", 1, 0);
	add_cyclical_funct(PirRtTask, 1, "pir_sync\0",CRITICAL_TASK);
}

static void PirRtTask()
{
	gu8pirSync = 1;
}

void setMin_PIR_ms(uint32_t v)
{
	min_PIR_ms=v;
}

uint32_t getMin_PIR_ms()
{
	return min_PIR_ms;
}

uint8_t getPir1Counting()
{
	return pir1_time_counting;
}

uint8_t getPir2Counting()
{
	return pir2_time_counting;
}

uint8_t getPir3Counting()
{
	return pir3_time_counting;
}

uint8_t getPir4Counting()
{
	return pir4_time_counting;
}
#define PIR1MASK (1<<0)
#define PIR2MASK (1<<1)
#define PIR3MASK (1<<2)
#define PIR4MASK (1<<3)
#define PIRSMASK (PIR1MASK|PIR2MASK|PIR3MASK|PIR4MASK)
#define REVERSEINPUTS

uint8_t readPIRS()
{
	GPIO_PinState status;
	uint8_t pirs,bits=0;
	status=HAL_GPIO_ReadPin(PIR1_GPIO_Port, PIR1_Pin);
	if(status==GPIO_PIN_SET)
		bits=PIR1MASK|bits;
	status=HAL_GPIO_ReadPin(PIR2_GPIO_Port, PIR2_Pin);
	if(status==GPIO_PIN_SET)
		bits=PIR2MASK|bits;
	status=HAL_GPIO_ReadPin(PIR3_GPIO_Port, PIR3_Pin);
	if(status==GPIO_PIN_SET)
		bits=PIR3MASK|bits;
	status=HAL_GPIO_ReadPin(PIR4_GPIO_Port, PIR4_Pin);
	if(status==GPIO_PIN_SET)
		bits=PIR4MASK|bits;
#ifdef REVERSEINPUTS
	pirs=~bits;
#else
	pirs=bits;
#endif
	return (pirs&PIRSMASK);
}

void resetCounters()
{
	pir1_ms_time=pir2_ms_time=pir3_ms_time=pir4_ms_time=0;
}

static void PirMainTask()
{
	_PirStatus stPirsNextState;
	uint8_t pirs;
	static uint16_t step=0;
	static uint8_t OutEn=0;
	if(gu8pirSync == 0) return;
	gu8pirSync = 0;
	if(stPirsState != stPirsStateOld)
	{
		stPirsStateOld = stPirsState;
	}
	stPirsNextState=stPirsStateOld;
	switch(stPirsState)
	{
	case INIT_PIRS:
		pir1_debouncing=pir2_debouncing=pir3_debouncing=pir4_debouncing=0;
		pir1_time_counting=pir2_time_counting=pir3_time_counting=pir4_time_counting=0;
		stPirsNextState=CHECKINPUTS;
		stPirsState = stPirsNextState;
		OutEn=0;
		break;
	case CHECKINPUTS:
		pirs=readPIRS();
		if(pirs&PIR1MASK)
		{
			pir1_debouncing=1;
			stPirsNextState=CHECKDEBOUNCE;
		}
		else if(pirs&PIR2MASK)
		{
			pir2_debouncing=1;
			stPirsNextState=CHECKDEBOUNCE;
		}
		else if(pirs&PIR3MASK)
		{
			pir3_debouncing=1;
			stPirsNextState=CHECKDEBOUNCE;
		}
		else if(pirs&PIR4MASK)
		{
			pir4_debouncing=1;
			stPirsNextState=CHECKDEBOUNCE;
		}
		if(stPirsNextState!=stPirsStateOld) resetCounters();//inizio debounce
		stPirsState = stPirsNextState;
		break;
	case CHECKDEBOUNCE:
		pirs=readPIRS();
		if     (pir1_debouncing&&(pirs&PIR1MASK))
		{
			if(pir1_ms_time>debounce_ms)
			{
				++step;
				printf("\n\rPIR1->set Usound %d\n\r",step);
				pir1_time_counting=1;
				stPirsNextState = CHECKCOUNT;
				OutEn=1;
			}
		}

		else if(pir2_debouncing&&(pirs&PIR2MASK))
		{
			if(pir2_ms_time>debounce_ms)
			{
				++step;
				printf("\n\rPIR2->set Usound %d\n\r",step);
				pir2_time_counting=1;
				stPirsNextState = CHECKCOUNT;
				OutEn=1;
			}
		}
		else if(pir3_debouncing&&(pirs&PIR3MASK))
		{
			if(pir3_ms_time>debounce_ms)
			{
				++step;
				printf("\n\rPIR3->set Usound %d\n\r",step);
				pir3_time_counting=1;
				stPirsNextState = CHECKCOUNT;
				OutEn=1;
			}
		}
		else if(pir4_debouncing&&(pirs&PIR4MASK))
		{
			if(pir4_ms_time>debounce_ms)
			{
				++step;
				printf("\n\rPIR4->set Usound %d\n\r",step);
				pir4_time_counting=1;
				stPirsNextState = CHECKCOUNT;
				OutEn=1;
			}
		}
		else
		{
			stPirsNextState = INIT_PIRS;
		}
		stPirsState = stPirsNextState;
		break;
	case CHECKCOUNT:

		if     (pir1_time_counting)
		{
			if(pir1_ms_time>min_PIR_ms+debounce_ms)
			{
				if(OutEn)
					printf("PIR1->reset Usound %d\n\r",step);
				//
				stPirsNextState = INIT_PIRS;
			}
		}
		else if(pir2_time_counting)
		{
			if(pir2_ms_time>min_PIR_ms+debounce_ms)
			{
				if(OutEn)
					printf("PIR2->reset Usound %d\n\r",step);
				//
				stPirsNextState = INIT_PIRS;
			}
		}
		else if(pir3_time_counting)
		{
			if(pir3_ms_time>min_PIR_ms+debounce_ms)
			{
				if(OutEn)
					printf("PIR3->reset Usound %d\n\r",step);
				//
				stPirsNextState = INIT_PIRS;
			}
		}
		else if(pir4_time_counting)
		{
			if(pir4_ms_time>min_PIR_ms+debounce_ms)
			{
				if(OutEn)
					printf("PIR4->reset Usound %d\n\r",step);
				//
				stPirsNextState = INIT_PIRS;
			}
		}


		stPirsState = stPirsNextState;
		break;
	case PIRS_LAST:
	default:
		stPirsState = INIT_PIRS;
		break;
	}
	pir1_ms_time++;
	pir2_ms_time++;
	pir3_ms_time++;
	pir4_ms_time++;
}

uint32_t get_pir1_time_ms()
{
	return pir1_ms_time;
}

uint32_t get_pir2_time_ms()
{
	return pir2_ms_time;
}

uint32_t get_pir3_time_ms()
{
	return pir3_ms_time;
}

uint32_t get_pir4_time_ms()
{
	return pir4_ms_time;
}
void setDebounce_ms(uint32_t v)
{
	debounce_ms = v;//primo debounce
}

uint32_t getDebounce_ms()
{
	return debounce_ms;
}



//#define INVERSELEVEL
//uint8_t readInterruptState(const GPIO_TypeDef *pport,uint16_t pin)
//{
//	uint8_t ret=0;
//	GPIO_PinState val=HAL_GPIO_ReadPin(pport, pin);
//	if(val== GPIO_PIN_SET)
//		ret=1;
//	else ret=0;
//#ifdef INVERSELEVEL
//	ret=!ret;
//#endif
//	return ret;
//}
