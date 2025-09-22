/*
 * sf04_tsk.c
 *
 *  Created on: Dec 5, 2022
 *      Author: rfacc
 */
#include <sensors_task.h>
#include <stdio.h>
#include "KedOS.h"
#include "main.h"
#include "sht3x.h"
#include "string.h"
#include "bms.h"

#define SENSOR_IDLE           0
#define INIT_MEASURE_SENSOR1  1
#define READ_MEASURE_SENSOR1  2
#define START_MEASURE_SENSOR1 3
#define READ_BATTERY_DATA     4

#define BATTERY_READ_TIMEOUT  60
static volatile uint8_t charger_stat1;
static volatile uint8_t charger_stat2;
static volatile uint8_t charger_mpp;
static volatile uint8_t charger_term;
static uint8_t chargeFlagActive = 1;


static uint8_t  chargerStatus = 0x00;

uint8_t gu8Sync = 0;
uint8_t gu8State = 0;
uint8_t gu8Wait  = 0;

static int32_t sensor_temp;
static int32_t sensor_hum;
static volatile uint8_t sensor_state = SENSOR_IDLE;
static stBatteryData gBatteryData;
static uint8_t gBatteryReadLoopCnt = 0;
HAL_StatusTypeDef status;
uint8_t cmd[2];
uint8_t result[3];
static int error = 0;
#define SYNC_TIME 10*60*10;

#define I2C_DELAY 1000
#define SFM4100_addr 0x01 << 1

static void SensorRtTask();
static void SensorMainTask();

//static uint8_t i2cScanRes[128];
//int    i2c_Scan(void)
//{
//  int i= 0;
//  int nDetect = 0;
//  uint8_t address = 0;
//  memset(i2cScanRes,0x00,128);
//  for(i=0;i<128;i++)
//  {
//	  address = i<<1;
//	  if (HAL_I2C_IsDeviceReady(&hi2c1, address, 2, 100)==HAL_OK)
//	  {
//		  i2cScanRes[nDetect]=address;
//		  nDetect++;
//	  }
//  }
//  return nDetect;
//}

stBatteryData* getBatteryData()
{
	gBatteryData.BatteryVoltage = getBatteryVoltageMv();
	gBatteryData.BatteryCurrent = getBatteryCurrentMa();
	gBatteryData.Temperature    = getBatteryTemperatureC();
	//gBatteryData.BatteryChargeLevelAbs = getBatteryAbsoluteStateOfCharge();
	gBatteryData.BatteryChargeLevelRelative = getBatteryRelativeStateOfCharge();
	gBatteryData.BatteryStatus  = getBatteryStatus();
	gBatteryData.DesignVoltage  = getBatteryDesignVoltage();
	gBatteryData.PackStatus     = getBatteryPackStatus();
	return &gBatteryData;
}
int16_t get_sensor_temp()
{
	return sensor_temp;
}
int16_t get_sensor_humidity()
{
	return sensor_hum;
}
uint8_t get_charger_status()
{
	return chargerStatus;
}
void sensors_task_init()
{
	error    = -1;
	gu8Sync  = 0;
	gBatteryReadLoopCnt = BATTERY_READ_TIMEOUT;
//	HAL_GPIO_WritePin(SHT_RSTn_GPIO_Port, SHT_RSTn_Pin, GPIO_PIN_SET);
	sensor_state = SENSOR_IDLE;

	//i2c_Scan();
	setBattery_i2c_bus(&hi2c1);

	//time critical function
//	add_cyclical_funct(SensorRtTask, 1000, "sensors_sync\0",CRITICAL_TASK);
	add_cyclical_funct(SensorRtTask, 2000, "sensors_sync\0",CRITICAL_TASK);
	add_mainloop_funct(SensorMainTask, "sensors-conv\0", 1, 0);
}
static void SensorRtTask()
{
	gu8Sync = 1;
}

static void SensorMainTask()
{
	static volatile int nRet1 = 0;
	int32_t temperature;
	int32_t humidity;

    if(gu8Sync == 0) return;
    gu8Sync = 0;

    charger_stat1 = HAL_GPIO_ReadPin(STAT1_GPIO_Port, STAT1_Pin);
    charger_stat2 = HAL_GPIO_ReadPin(STAT2_GPIO_Port, STAT2_Pin);
    charger_mpp   = HAL_GPIO_ReadPin(MPP_EN_GPIO_Port, MPP_EN_Pin);
    charger_term  = HAL_GPIO_ReadPin(TERM_EN_GPIO_Port, TERM_EN_Pin);

    chargerStatus = (charger_stat1&0x1) | (charger_stat2&0x1)<<1;

    switch(chargerStatus)
    {
    case CHARGE_SUSPEND:
    	break;
    case CHARGE_IN_PROGRESS:
    	break;
    case CHARGE_COMPLETE:
    	break;
    case CHARGE_ERROR:
    	break;
    }

    switch (sensor_state) {
		case SENSOR_IDLE:
			sht3x_set_power_mode(SHT3X_MEAS_MODE_HPM);
			sensor_state = INIT_MEASURE_SENSOR1;
			break;
		case INIT_MEASURE_SENSOR1:
			sht3x_i2c_bus(&hi2c1);
			sht3x_measure(0x44);
			sensor_state = READ_MEASURE_SENSOR1;
			break;
		case READ_MEASURE_SENSOR1:
			nRet1= sht3x_read(0x44, &temperature,&humidity);
			if(nRet1== 0)
			{
				sensor_temp = temperature/100;
				sensor_hum  = humidity/100;
			}
			sensor_state = START_MEASURE_SENSOR1;
			break;
		case START_MEASURE_SENSOR1:
			sht3x_i2c_bus(&hi2c1);
			sht3x_measure(0x44);
			sensor_state = READ_MEASURE_SENSOR1;
			break;
		default:
			break;
	}

    gBatteryReadLoopCnt++;
    if(gBatteryReadLoopCnt>= BATTERY_READ_TIMEOUT)
    {
    	gBatteryReadLoopCnt = 0;
    	gBatteryData.BatteryChargeLevelRelative = getBatteryRelativeStateOfCharge();
    	gBatteryData.BatteryVoltage = getBatteryVoltageMv();
    	gBatteryData.BatteryCurrent = getBatteryCurrentMa();
    }
    static uint8_t changeDelay = 0;

    float fbatteryTemp = getBatteryTemperatureC();
    if((fbatteryTemp<-2) || (fbatteryTemp>52))
    {
    	HAL_GPIO_WritePin(GPIOB, MPP_EN_Pin, GPIO_PIN_SET);
    	return ;
    }

    if(chargeFlagActive > 0)
    {
    	if(changeDelay<10)
    	{
    		changeDelay++;
    	}
    	HAL_GPIO_WritePin(GPIOB, MPP_EN_Pin, GPIO_PIN_RESET);
    	HAL_GPIO_WritePin(GPIOB, TERM_EN_Pin, GPIO_PIN_SET);
    	if((chargerStatus == CHARGE_COMPLETE)||(chargerStatus == CHARGE_ERROR))
    	{
    		if(changeDelay>=10)
    		{
				chargeFlagActive = 0;
				changeDelay      = 0;
				HAL_GPIO_WritePin(GPIOB, MPP_EN_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOB, TERM_EN_Pin, GPIO_PIN_RESET);
    		}
    	}
    }
    else
    {
    	if((gBatteryData.BatteryVoltage>0) && (gBatteryData.BatteryVoltage<13400) && gBatteryData.BatteryCurrent >=0)
		{
    		HAL_GPIO_WritePin(GPIOB, MPP_EN_Pin, GPIO_PIN_RESET);
    		chargeFlagActive = 1;
    		changeDelay      = 0;
		}
    }

}



