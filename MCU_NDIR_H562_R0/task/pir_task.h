/*
 * pir_task.h
 *
 *  Created on: Aug 24, 2024
 *      Author: rfacc
 */

#ifndef PIR_TASK_H_
#define PIR_TASK_H_

#include "stdint.h"
typedef enum {
	PIR0,
	PIR1,
	PIR2,
	PIR3,
	PIR4
}TPirnum;

void   pir_task_init();
uint32_t get_pir1_time_ms();
uint32_t get_pir2_time_ms();
uint32_t get_pir3_time_ms();
uint32_t get_pir4_time_ms();
uint8_t getPir1Counting();
uint8_t getPir2Counting();
uint8_t getPir3Counting();
uint8_t getPir4Counting();
uint32_t getMin_PIR_ms();
void setMin_PIR_ms(uint32_t v);
//uint8_t stopwatchPIR(TPirnum pirnum);
//uint8_t startwatchPIR(TPirnum pirnum);
void setDebounce_ms(uint32_t v);
uint32_t getDebounce_ms();
void setReadingPir(uint8_t v);
uint8_t readPIRS();

//uint8_t readInterruptState(const GPIO_TypeDef *pport,uint16_t pin);
#endif /* BME_TASK_H_ */
