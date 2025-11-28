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


void pir_task_init()
{
	gu8pirSync  = 0;
	//time critical function
	stPirsState    = INIT_PIRS;
	add_mainloop_funct(PirMainTask, "pir-taSK\0", 1, 0);
	add_cyclical_funct(PirRtTask, 1, "pir_sync\0",CRITICAL_TASK);
}

#define COUNT_PIR_TIMEOUT 3000
#define DENOISE_STEP      5
static uint8_t  gu8PirValue[4];
static uint8_t  gu8PirValueFiltered[4];
static uint8_t  gu8PirValueFilteredD[4];
static uint8_t  gu8PirValueEvent[4];

static uint32_t gu8PirCounter[4];
static uint8_t  gu8PirFilterCount[4];


static void PirRtTask()
{
	gu8pirSync = 1;

	gu8PirValue[0] = HAL_GPIO_ReadPin(PIR1_GPIO_Port, PIR1_Pin);
	gu8PirValue[1] = HAL_GPIO_ReadPin(PIR2_GPIO_Port, PIR2_Pin);
	gu8PirValue[2] = HAL_GPIO_ReadPin(PIR3_GPIO_Port, PIR3_Pin);
	gu8PirValue[3] = HAL_GPIO_ReadPin(PIR4_GPIO_Port, PIR4_Pin);

	if(gu8PirCounter[0]>0)
	{
		gu8PirCounter[0]--;
	}
	if(gu8PirCounter[1]>0)
	{
		gu8PirCounter[1]--;
	}
	if(gu8PirCounter[2]>0)
	{
		gu8PirCounter[2]--;
	}
	if(gu8PirCounter[3]>0)
	{
		gu8PirCounter[3]--;
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
			if(gu8PirFilterCount[i]<DENOISE_STEP)
				gu8PirFilterCount[i]++;
		}
		if(gu8PirFilterCount[i] == 0)
		{
			gu8PirValueFiltered[i] = 1;
		}
		if(gu8PirFilterCount[i] == DENOISE_STEP)
		{
			gu8PirValueFiltered[i] = 0;
		}
		if((gu8PirValueFilteredD[i] == 0) && (gu8PirValueFiltered[i] == 1))
		{
			gu8PirValueEvent[i] = 1;
			gu8PirCounter[i]    = COUNT_PIR_TIMEOUT;
		}
		gu8PirValueFilteredD[i] = gu8PirValueFiltered[i];
	}


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

void resetCounters()
{
	pir1_ms_time=pir2_ms_time=pir3_ms_time=pir4_ms_time=0;
}

//static void PirMainTask()
//{
//	_PirStatus stPirsNextState;
//	uint8_t pirs;
//	static uint8_t OutEn=0;
//	if(gu8pirSync == 0) return;
//	gu8pirSync = 0;
//	if(stPirsState != stPirsStateOld)
//	{
//		stPirsStateOld = stPirsState;
//	}
//	stPirsNextState=stPirsStateOld;
//	switch(stPirsState)
//	{
//	case INIT_PIRS:
//		pir1_debouncing=pir2_debouncing=pir3_debouncing=pir4_debouncing=0;
//		pir1_time_counting=pir2_time_counting=pir3_time_counting=pir4_time_counting=0;
//		stPirsNextState=CHECKINPUTS;
//		stPirsState = stPirsNextState;
//		printf("Reset Usound\n\r");
//		OutEn=0;
//		break;
//	case CHECKINPUTS:
//		pirs=readPIRS();
//		if(pirs&PIR1MASK)
//		{
//			pir1_debouncing=1;
//			stPirsNextState=CHECKDEBOUNCE;
//		}
//		else if(pirs&PIR2MASK)
//		{
//			pir2_debouncing=1;
//			stPirsNextState=CHECKDEBOUNCE;
//		}
//		else if(pirs&PIR3MASK)
//		{
//			pir3_debouncing=1;
//			stPirsNextState=CHECKDEBOUNCE;
//		}
//		else if(pirs&PIR4MASK)
//		{
//			pir4_debouncing=1;
//			stPirsNextState=CHECKDEBOUNCE;
//		}
//		if(stPirsNextState!=stPirsStateOld) resetCounters();//inizio debounce
//		stPirsState = stPirsNextState;
//		break;
//	case CHECKDEBOUNCE:
//		pirs=readPIRS();
//		if     (pir1_debouncing&&(pirs&PIR1MASK))
//		{
//			if(pir1_ms_time>debounce_ms)
//			{
//				pir1_time_counting=1;
//				stPirsNextState = CHECKCOUNT;
//			}
//		}
//
//		else if(pir2_debouncing&&(pirs&PIR2MASK))
//		{
//			if(pir2_ms_time>debounce_ms)
//			{
//				pir2_time_counting=1;
//				stPirsNextState = CHECKCOUNT;
//			}
//		}
//		else if(pir3_debouncing&&(pirs&PIR3MASK))
//		{
//			if(pir3_ms_time>debounce_ms)
//			{
//				pir3_time_counting=1;
//				stPirsNextState = CHECKCOUNT;
//			}
//		}
//		else if(pir4_debouncing&&(pirs&PIR4MASK))
//		{
//			if(pir4_ms_time>debounce_ms)
//			{
//				pir4_time_counting=1;
//				stPirsNextState = CHECKCOUNT;
//			}
//		}
//		else
//		{
//			stPirsNextState = INIT_PIRS;
//		}
//		stPirsState = stPirsNextState;
//		break;
//	case CHECKCOUNT:
//		pirs=readPIRS();
//		if     (pir1_time_counting&&(pirs&PIR1MASK))
//		{
//			if(pir1_ms_time>min_PIR_ms+debounce_ms)
//			{
//				if(!OutEn)
//					printf("PIR1->set Usound\n\r");
//				//attivazione uscita
//				OutEn=1;
//			}
//		}
//		else if(pir2_time_counting&&(pirs&PIR2MASK))
//		{
//			if(pir2_ms_time>min_PIR_ms+debounce_ms)
//			{
//				if(!OutEn)
//					printf("PIR2->set Usound\n\r");
//				//attivazione uscita
//				OutEn=1;
//			}
//		}
//		else if(pir3_time_counting&&(pirs&PIR3MASK))
//		{
//			if(pir3_ms_time>min_PIR_ms+debounce_ms)
//			{
//				if(!OutEn)
//					printf("PIR3->set Usound\n\r");
//				//attivazione uscita
//				OutEn=1;
//			}
//		}
//		else if(pir4_time_counting&&(pirs&PIR4MASK))
//		{
//			if(pir4_ms_time>min_PIR_ms+debounce_ms)
//			{
//				if(!OutEn)
//					printf("PIR4->set Usound\n\r");
//				//attivazione uscita
//				OutEn=1;
//			}
//		}
//		else
//		{
//			stPirsNextState = INIT_PIRS;
//		}
//		stPirsState = stPirsNextState;
//		break;
//	case PIRS_LAST:
//	default:
//		stPirsState = INIT_PIRS;
//		break;
//	}
//	pir1_ms_time++;
//	pir2_ms_time++;
//	pir3_ms_time++;
//	pir4_ms_time++;
//}

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
