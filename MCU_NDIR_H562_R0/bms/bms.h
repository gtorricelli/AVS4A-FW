#ifndef BMS_H_
#define BMS_H_
#include "stdint.h"
#include "bsp.h"

typedef struct {
	uint16_t   BatteryVoltage;
	int16_t    BatteryCurrent;
	uint16_t   BatteryChargeLevelAbs;/*0-1000 1000 = 100%*/
	uint16_t   BatteryChargeLevelRelative;/*0-1000 1000 = 100%*/
	float      Temperature;//0.1°C
	uint16_t   BatteryStatus;
	uint16_t   DesignVoltage;
	uint16_t   PackStatus;
	uint16_t   OperationStatus;
} stBatteryData;

int16_t  setBattery_i2c_bus(I2C_HandleTypeDef * hI2c);
int      dumpe_register();
float    getBatteryTemperatureC();
float    getBatteryTemperatureF();
uint16_t getBatteryVoltageMv();/*KED*/
int16_t  getBatteryCurrentMa();/*KED*/
int16_t  getBatteryAverageCurrentMa();/*KED*/
uint8_t  getBatteryMaxError();
uint8_t  getBatteryRelativeStateOfCharge();/*KED*/
//uint8_t  getBatteryAbsoluteStateOfCharge();/*KED*/
uint16_t getBatteryRemainingCapacityMah();
uint16_t getBatteryFullChargeCapacityMah();
uint16_t getBatteryRunTimeToEmptyMin();
uint16_t getBatteryAveragetimeToEmptyMin();
uint16_t getBatteryAveragetimeToFullMin();
uint16_t getBatteryChargingCurrentMa();
uint16_t getBatteryChargingVoltageMv();
//uint16_t getBatteryCycleCount();
//uint16_t getBatteryCellVoltage1Mv();
//uint16_t getBatteryCellVoltage2Mv();
//uint16_t getBatteryCellVoltage3Mv();
//uint16_t getBatteryCellVoltage4Mv();
uint16_t getBatteryStatus();
uint16_t getBatteryDesignVoltage();
uint16_t getBatteryPackStatus();
uint16_t getBatteryOperationStatus();

#endif /* BMS_H_ */
