/*
 * bsp.h
 *
 *  Created on: Apr 1, 2023
 *      Author: giova
 */

#ifndef INC_BSP_H_
#define INC_BSP_H_

#include <stdint.h>
#include "usart.h"
#include "tim.h"
#include "i2c.h"
#include "spi.h"
#include "gpio.h"
#include "main.h"
#include "types.h"

#define RXTERMDIM 128
#define FWVERSION "1.0"
#define SYSTEM_CYCLE_CLOCK_HZ 200000000  //clock del timer
#define SYSTEM_CYCLE_CLOCK_HZ_HISPEED 120000000  //clock del timer hispeed
#define ADC_BUF_LEN 1024 //massima dimensione del buffer di acquisizione, quindi anche massima lunghezza di acquisizione
#define NPULSESPERBURST 1 //numero di impulsi per burst

int __io_putchar(int ch);
int __io_getchar(void);
uint32_t _get_sys_clock();
uint32_t _time_measure(uint32_t start);
void     _disable_scheduler();
void     _enable_scheduler();
void     _enable_irq();
void     _disable_irq();
uint32_t time_measure_ms(uint32_t start);
bool_t   time_elapsed_ms(uint32_t start,uint32_t time);
uint32_t get_clock_ms();
int PRINTF( const char *format, ... );
int SPRINTF( uint16_t maxlen,char *apBuf, const char *apFmt, ... );
#define PRINTF(...)			PRINTF(__VA_ARGS__)
#define SPRINTF(...)		SPRINTF(__VA_ARGS__)



#endif /* INC_BSP_H_ */
