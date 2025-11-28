#include "KedOS.h"
#include "main.h"
#include "iwdg.h"
/*private function fo Protocol */
static void BlinkTask();


void blink_task_init()
{
	//time critical function
	add_cyclical_funct(BlinkTask, 10, "Heartbeat\0",CRITICAL_TASK);
}

static void BlinkTask()
{
	uint8_t bLedValue = 0;
	static unsigned short bLedCnt = 0;
	uint8_t u8Color;
	if((bLedCnt > 18) && (bLedCnt < 20))
	{
		bLedValue = 1;
	}
	else if((bLedCnt > 36) && (bLedCnt < 38)){
		bLedValue = 1;
	}
	else
	{
		bLedValue = 0;
	}
	bLedCnt++;
	if(bLedCnt >= 99){
		bLedCnt = 0;
		HAL_IWDG_Refresh(&hiwdg);
	}

//	u8Color = bLedValue * 128;
//	if(bLedValue == 0)
//	{
//		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin,bLedValue);
//
//	}
//	else
//	{
//		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin,bLedValue);
//	}

}

