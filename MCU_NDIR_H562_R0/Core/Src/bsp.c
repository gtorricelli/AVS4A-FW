/*
 * bsp.c
 *
 *  Created on: Apr 1, 2023
 *      Author: rfacc
 */

#include "bsp.h"
#include "KedOS.h"
#include "usb.h"
#include <usbd_cdc_if.h>

static uint32_t u32_DelayPrescaler;
//#define USB_SHELL 1
#define USB_PORT CDC_SERIAL1
//#undef USB_SHELL
#define USB_SHELL


int _write(int file, char *ptr, int len) {
	uint8_t Ret=USBD_BUSY;
#ifdef USB_SHELL
	Ret=CDC_Transmit_Buffered((uint8_t*) ptr, len,USB_PORT);
    while(Ret==USBD_BUSY)
    {
    	Ret=CDC_Transmit_Buffered((uint8_t*) ptr, len, USB_PORT);
    	HAL_Delay(1);
    }
#else
	Ret = HAL_UART_Transmit(&huart3, (uint8_t*)&ptr, len,0xffff);
#endif

    return len;
}

int __io_putchar(int ch)
{
	uint8_t Ret=USBD_BUSY;

#ifdef USB_SHELL
	Ret = CDC_Transmit_Buffered((uint8_t*) &ch, 1, USB_PORT);
	while (Ret == USBD_BUSY) {
		Ret = CDC_Transmit_Buffered((uint8_t*) &ch, 1, USB_PORT);
		HAL_Delay(1);
	}
#else
	Ret = HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1,0xffff);
#endif
	return Ret;
}

int __io_getchar(void)
{
  uint8_t ch = 0;
  HAL_StatusTypeDef res;


#ifdef USB_SHELL
	res = CDC_Get_Char_FS((uint8_t*) &ch, USB_PORT);
#else
	  __HAL_UART_CLEAR_OREFLAG(&huart1);
	  res=HAL_UART_Receive(&huart1, (uint8_t *)&ch, 1, 0);
#endif

  if (res != HAL_OK)
     return -1;
  return ch;
}

void     _disable_scheduler()
{

}

void     _enable_scheduler()
{

}
void     _enable_irq()
{
	__enable_irq();
}

void     _disable_irq()
{
	__disable_irq();
}

uint32_t _get_sys_clock()
{
	return _get_os_timer();
}

/**
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
uint32_t _time_measure(uint32_t start)
{
	uint32_t difference, current;

	current = _get_sys_clock();

	if ( current < start )
	{
		difference = ULONG_MAX - (start - current - 1);
	}
	else
	{
		difference = current - start;
	}

	return difference;
}

bool_t   _time_elapsed(uint32_t start,uint32_t time)
{
	bool_t elapsed = FALSE;

	if (_time_measure(start) >= time )
	{
		elapsed = TRUE;
	}

	return elapsed;
}
/**
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
uint32_t time_measure_ms(uint32_t start)
{
	return _time_measure(start);
}

bool_t   time_elapsed_ms(uint32_t start,uint32_t time)
{
	bool_t elapsed = FALSE;

	if (_time_measure(start) >= time )
	{
		elapsed = TRUE;
	}

	return elapsed;
}
uint32_t get_clock_ms()
{
	return _get_os_timer();
}
void _delayInit(void) {
	/* While loop takes 4 cycles */
	/* For 1 us delay, we need to divide with 4M */
	u32_DelayPrescaler = HAL_RCC_GetHCLKFreq() / 4000000;
}


void     _delay_us(uint32_t delay_us)
{
	delay_us = delay_us * u32_DelayPrescaler - 10;
	/* 4 cycles for one loop */
	while (delay_us--);
}

void     _delay_ms(uint32_t delay_ms)
{
	/* Multiply millis with multipler */
	/* Substract 10 */
	delay_ms = 1000 * delay_ms * u32_DelayPrescaler - 10;
	/* 4 cycles for one loop */
	while (delay_ms--);
}

