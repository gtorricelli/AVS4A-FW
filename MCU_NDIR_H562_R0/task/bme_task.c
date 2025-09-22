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

#include "../bme280/bme280.h"

#define BME_RESET             0
#define BME_WAIT_READY        1
#define BME_INIT              2
#define BME_READY             3
#define BME_ELAB_T            4
#define BME_ELAB_P            5
#define BME_ELAB_ALT          6
#define BME_ELAB_H            7

uint8_t gu8BmeSync = 0;
uint8_t gu8BmeState = BME_RESET;

static void BmeRtTask();
static void BmeMainTask();

static float    bme_ftemp;
static float    bme_fhum;
static float    bme_fpress;


static uint8_t  bmeVerID    = 0;
static uint8_t  bmeStatus   = 0;
static uint8_t  bmeMode     = 0;
static uint16_t bmeStatMeas = 0;

static int32_t rawT;
static int32_t rawP;
static int32_t rawH;

float get_bme_temp()
{
	return bme_ftemp;
}
float get_bme_humidity()
{
	return bme_fhum;
}
float get_bme_pressure()
{
	return bme_fpress;
}

void bme_task_init()
{
	gu8BmeSync  = 0;
	gu8BmeState = BME_RESET;
	//time critical function
	add_cyclical_funct(BmeRtTask, 1000, "bme_sync\0",CRITICAL_TASK);
	add_mainloop_funct(BmeMainTask, "bme-conv\0", 1, 0);
}
static void BmeRtTask()
{
	gu8BmeSync = 1;
}
static void BmeMainTask()
{
	BME280_RESULT bmeRes = 0;
	uint8_t ret  = 0;
    if(gu8BmeSync == 0) return;
    gu8BmeSync = 0;

    switch (gu8BmeState) {
		case BME_RESET:
			BME280_Reset();
			gu8BmeState = BME_WAIT_READY;
			break;
		case BME_WAIT_READY:
			bmeStatus = BME280_GetStatus();
			gu8BmeState = BME_INIT;

//			if(bmeStatus & BME280_STATUS_IM_UPDATE)
//			{
//				gu8BmeState = BME_INIT;
//			}
			break;
		case BME_INIT:
			bmeStatus = BME280_GetStatus();
			bmeVerID  = BME280_GetVersion();
			bmeMode   = BME280_GetMode();
			// Read calibration values
			ret = BME280_Read_Calibration();
			if(ret != BME280_SUCCESS)
			{
				gu8BmeState = BME_RESET;
				return;
			}
			// Set normal mode inactive duration (standby time)
			BME280_SetStandby(BME280_STBY_1s);
			// Set IIR filter constant
			BME280_SetFilter(BME280_FILTER_4);
			// Set oversampling for temperature
			BME280_SetOSRST(BME280_OSRS_T_x4);
			// Set oversampling for pressure
			BME280_SetOSRSP(BME280_OSRS_P_x2);
			// Set oversampling for humidity
			BME280_SetOSRSH(BME280_OSRS_H_x1);
			// Set normal mode (perpetual periodic conversion)
			BME280_SetMode(BME280_MODE_NORMAL);
			gu8BmeState = BME_READY;
			break;
		case BME_READY:
			bmeStatMeas  = BME280_ReadReg(BME280_REG_CTRL_MEAS);
			bmeStatMeas |= BME280_ReadReg(BME280_REG_CTRL_HUM) << 8;
			bmeStatus    = BME280_GetStatus();
			//if(bmeStatus & BME280_STATUS_MEASURING) // "MEASURING" : "READY",
			// Get current status of the chip
			bmeRes       = BME280_Read_UTPH(&rawT, &rawP, &rawH);
			if(bmeRes == BME280_SUCCESS)
				gu8BmeState = BME_ELAB_T;
			break;
		case BME_ELAB_T:
			bme_ftemp   = BME280_CalcTf(rawT);//resolution is 0.01 degree
			gu8BmeState = BME_ELAB_P;
			break;
		case BME_ELAB_P:
			bme_fpress  = BME280_CalcPf(rawP);//resolution is 0.001 Pa
			gu8BmeState = BME_ELAB_H;
			break;
		case BME_ELAB_H:
			bme_fhum    = BME280_CalcHf(rawH);//resolution 0.001 %RH
			gu8BmeState = BME_READY;
			break;
		default:
			break;
	}
}



