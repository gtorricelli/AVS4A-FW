#ifndef PARAMETERS_H_
#define PARAMETERS_H_
#include <eeprom.h>
#include <types.h>

#define HEADER_KEY  0xAA
#define HISTORYDIM  5

typedef struct {
	float    f_R_134a;
	float    f_R_1234;
	float    f_R_22;
	float    f_HC;
	float    f_AIR;
	uint8_t  u8ErrorCode;
	uint16_t u16TestNum;
} _measure;


typedef struct{
	uint16_t eeprom_crc16;
	uint8_t  eepromHeaderKey;
	uint8_t  u8ProtocolID;
	uint8_t  u8FilterCounter;
	uint8_t  u8Error;
	uint16_t u16AirSensorCounter;//stesso valore impostato su matlab,max=ADC_BUF_LEN;
	uint16_t u16InstrumentTestCounter;
	uint8_t  u8LastHistoryIndex;
	_measure stGasMeasureHistory[HISTORYDIM];
}_Parameters;

typedef enum
{
	SAEJ2912=0U,
	SAEJ2927,
	VDA,
	HVAC,
} PROTOCOL_INSTRUMENT;

extern _Parameters g_InstrumentParams;
extern int g_nMeasureHistoryIndex;

uint8_t loadParametersFromEeprom();
void    saveParametersToEeprom();

void dumpInitParsOnFlash();
void sortInitPars(_Parameters*pInitPars ,int nMeasureHistoryIndex);
void eraseFlash();
void initFlashContent();

#endif /* PARAMETERS_H_ */
