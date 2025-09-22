#include <parameters.h>
#include <string.h>
#include "crc.h"

_Parameters g_InstrumentParams;

void setDefaultParameters()
{
	int i=0;
	g_InstrumentParams.eepromHeaderKey = HEADER_KEY;
	g_InstrumentParams.u8ProtocolID    = SAEJ2912;
	g_InstrumentParams.u8FilterCounter = 0;
	g_InstrumentParams.u8Error = 0;
	g_InstrumentParams.u16AirSensorCounter = 0;
	g_InstrumentParams.u16InstrumentTestCounter = 0;
	g_InstrumentParams.u8LastHistoryIndex = 0;
	for(i=0;i<HISTORYDIM;i++)
	{
		memset(&g_InstrumentParams.stGasMeasureHistory[i],0x00,sizeof(_measure));
	}
}
uint8_t loadParametersFromEeprom()
{
	_Parameters stEepromParameters;
	uint16_t dsize = sizeof(_Parameters);

	eeprom_read(0, (uint8_t*)&stEepromParameters, dsize);
	uint16_t eeprom_crc16= usPacketCRC16((uint8_t*)&stEepromParameters.eepromHeaderKey,dsize-2);
	if((stEepromParameters.eepromHeaderKey != HEADER_KEY) || (eeprom_crc16 != stEepromParameters.eeprom_crc16))
	{
		return 1;
	}
	else
	{
		setDefaultParameters();
		saveParametersToEeprom();
		return 0;
	}
	return 0;
}
void saveParametersToEeprom()
{
	uint16_t dsize        = sizeof(_Parameters);
	g_InstrumentParams.eeprom_crc16= usPacketCRC16((uint8_t*)&g_InstrumentParams.eepromHeaderKey,dsize-2);
	eeprom_write(0, (uint8_t*)&g_InstrumentParams, dsize);
}




