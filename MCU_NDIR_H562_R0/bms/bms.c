#include "bms.h"

const uint8_t bq40z50DeviceAddress  = 0x55;//0x0B;

//#define BQ40Z50_TEMPERATURE              0x08
//#define BQ40Z50_VOLTAGE                  0x09
//#define BQ40Z50_CURRENT                  0x0A
//#define BQ40Z50_AVERAGE_CURRENT          0x0B
//#define BQ40Z50_MAX_ERROR                0x0C
//#define BQ40Z50_RELATIVE_STATE_OF_CHARGE 0x0D
//#define BQ40Z50_ABSOLUTE_STATE_OF_CHARGE 0x0E
//#define BQ40Z50_REMAINING_CAPACITY       0x0F
//#define BQ40Z50_FULL_CHARGE_CAPACITY     0x10
//#define BQ40Z50_RUNTIME_TO_EMPTY         0x11
//#define BQ40Z50_AVERAGE_TIME_TO_EMPTY    0x12
//#define BQ40Z50_AVERAGE_TIME_TO_FULL     0x13
//#define BQ40Z50_CHARGING_CURRENT         0x14
//#define BQ40Z50_CHARGING_VOLTAGE         0x15
//#define BQ40Z50_BATTERY_STATUS           0x16
//#define BQ40Z50_CYCLE_COUNT              0x17
//#define BQ40Z50_DESIGN_VOLTAGE           0x19
//#define BQ40Z50_CELL_VOLTAGE_1           0x3F
//#define BQ40Z50_CELL_VOLTAGE_2           0x3E
//#define BQ40Z50_CELL_VOLTAGE_3           0x3D
//#define BQ40Z50_CELL_VOLTAGE_4           0x3C
//#define BQ40Z50_OPERATION_STATUS         0x41
//#define BQ40Z50_PACK_STATUS              0x45

#define BQ40Z50_TEMPERATURE              0x06
#define BQ40Z50_VOLTAGE                  0x08
#define BQ40Z50_REMAINING_CAPACITY       0x10
#define BQ40Z50_FULL_CHARGE_CAPACITY     0x12
#define BQ40Z50_AVERAGE_CURRENT          0x14
#define BQ40Z50_CURRENT                  0x0C
#define BQ40Z50_MAX_ERROR                0x0E
#define BQ40Z50_RELATIVE_STATE_OF_CHARGE 0x2C
#define BQ40Z50_RUNTIME_TO_EMPTY         0x04
//#define BQ40Z50_ABSOLUTE_STATE_OF_CHARGE 0x02
#define BQ40Z50_AVERAGE_TIME_TO_EMPTY    0x04
#define BQ40Z50_CHARGING_CURRENT         0x32
#define BQ40Z50_CHARGING_VOLTAGE         0x30
#define BQ40Z50_AVERAGE_TIME_TO_FULL     0x18
#define BQ40Z50_BATTERY_STATUS           0x0A
#define BQ40Z50_PACK_STATUS              0x0A

#define BQ40Z50_CYCLE_COUNT              0x17
#define BQ40Z50_DESIGN_VOLTAGE           0x19
#define BQ40Z50_CELL_VOLTAGE_1           0x3F
#define BQ40Z50_CELL_VOLTAGE_2           0x3E
#define BQ40Z50_CELL_VOLTAGE_3           0x3D
#define BQ40Z50_CELL_VOLTAGE_4           0x3C
#define BQ40Z50_OPERATION_STATUS         0x41
uint8_t  readRegister(uint8_t addr);

I2C_HandleTypeDef *hi2c_bms;

int16_t setBattery_i2c_bus(I2C_HandleTypeDef * hI2c)
{
	hi2c_bms = hI2c;
	return 0;
}

int dumpe_register()
{
	uint16_t registerValue = 0x00;
	int i;
	for(i=0;i<100;i++)
	{
		registerValue = readRegister(i<<1);
		printf("Batt %.3d: %.4x \n\r",i,registerValue);
	}
}
uint8_t  readRegister(uint8_t addr)
{
	uint8_t u8Value = 0;
	HAL_I2C_Mem_Read(hi2c_bms, bq40z50DeviceAddress<<1, addr, 1, &u8Value, 1, 1000);
	return u8Value;
}
uint16_t readRegister16(uint8_t addr)
{
	int nRet = 0;
	uint8_t u8Value[2] = {0,0};
	uint16_t u16Value = 0;
	nRet = HAL_I2C_Mem_Read(hi2c_bms, bq40z50DeviceAddress<<1, addr, 1, u8Value, 2, 1000);
	if(nRet != 0)
	{
		I2C1_Error();
		return 0;
	}



	u16Value = (uint16_t)(u8Value[1]<<8) |  (uint16_t)u8Value[0];

	return u16Value;
}

static inline float dK_to_C(uint16_t dK) {
    // °C = deciKelvin * 0.1 - 273.15
	//return ((float)dK * 0.1f) - 273.15f;
    //return ((float)dK * 0.1f) - 273.15f;
	return ((float)dK*0.1f)-273.15f;
}

float    getBatteryTemperatureC()
{
  uint16_t temperature = readRegister16(BQ40Z50_TEMPERATURE); //In 0.1 K
  float tempC = dK_to_C(temperature);
  return(tempC);
}

float    getBatteryTemperatureF()
{
  return((getBatteryTemperatureC() * 9.0/5) + 32.0);
}

uint16_t getBatteryVoltageMv()
{
  return(readRegister16(BQ40Z50_VOLTAGE)); //In mV
}

int16_t  getBatteryCurrentMa()
{
  return(readRegister16(BQ40Z50_CURRENT)); //In mA
}

int16_t  getBatteryAverageCurrentMa()
{
  return(readRegister16(BQ40Z50_AVERAGE_CURRENT)); //In mA
}

uint8_t  getBatteryMaxError()
{
  uint8_t maxError = readRegister(BQ40Z50_MAX_ERROR); //In %
  return(maxError);
}

uint8_t  getBatteryRelativeStateOfCharge()
{
  uint8_t relStateOfCharge = readRegister(BQ40Z50_RELATIVE_STATE_OF_CHARGE); //In %
  return(relStateOfCharge);
}

//uint8_t  getBatteryAbsoluteStateOfCharge()
//{
//  uint8_t absStateOfCharge = readRegister(BQ40Z50_ABSOLUTE_STATE_OF_CHARGE); //In %
//  return(absStateOfCharge);
//}

uint16_t getBatteryRemainingCapacityMah()
{
  return(readRegister16(BQ40Z50_REMAINING_CAPACITY)); //In mAh when CAPM = 0
}

uint16_t getBatteryFullChargeCapacityMah()
{
  return(readRegister16(BQ40Z50_FULL_CHARGE_CAPACITY)); //In mAh when CAPM = 0
}

uint16_t getBatteryRunTimeToEmptyMin()
{
  return(readRegister16(BQ40Z50_RUNTIME_TO_EMPTY)); //In minutes
}

uint16_t getBatteryAveragetimeToEmptyMin()
{
  return(readRegister16(BQ40Z50_AVERAGE_TIME_TO_EMPTY)); //In minutes
}

uint16_t getBatteryAveragetimeToFullMin()
{
  return(readRegister16(BQ40Z50_AVERAGE_TIME_TO_FULL)); //In minutes
}

uint16_t getBatteryChargingCurrentMa()
{
  return(readRegister16(BQ40Z50_CHARGING_CURRENT)); //In mA
}

uint16_t getBatteryChargingVoltageMv()
{
  return(readRegister16(BQ40Z50_CHARGING_VOLTAGE)); //In mV
}

//uint16_t getBatteryCycleCount()
//{
//  return(readRegister16(BQ40Z50_CYCLE_COUNT));
//}

//uint16_t getBatteryCellVoltage1Mv()
//{
//  return(readRegister16(BQ40Z50_CELL_VOLTAGE_1));
//}
//
//uint16_t getBatteryCellVoltage2Mv()
//{
//  return(readRegister16(BQ40Z50_CELL_VOLTAGE_2));
//}
//
//uint16_t getBatteryCellVoltage3Mv()
//{
//  return(readRegister16(BQ40Z50_CELL_VOLTAGE_3));
//}

uint16_t getBatteryStatus()
{
  return(readRegister16(BQ40Z50_BATTERY_STATUS));
}
uint16_t getBatteryDesignVoltage()
{
	return(readRegister16(BQ40Z50_DESIGN_VOLTAGE));
}
uint16_t getBatteryPackStatus()
{
	return(readRegister16(BQ40Z50_PACK_STATUS));
}
uint16_t getBatteryOperationStatus()
{
	return(readRegister16(BQ40Z50_OPERATION_STATUS));
}

