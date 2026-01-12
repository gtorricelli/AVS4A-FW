#include "stdio.h"
#include "string.h"
#include "kedOS.h"
#include "serial_com_task.h"
#include "bsp.h"
#include "packing.h"
#include "bms.h"
#include "sensors_task.h"
#include "bme_task.h"
#include "main.h"
#include "time.h"
#include "bsp.h"
#include "usb.h"
#include "hailo_task.h"
#include "rtc_task.h"

#include <usbd_cdc_if.h>

static int cp_process_packet(void);
static int cp_decode_packet();
static int cp_send_packet(MessagePacket_t* pMessage);

static uint8_t u8TickState = 0;
static uint8_t u8ActiveState = 0;
#define SIZE_DATE_INFO 8
static unsigned char l_date[SIZE_DATE_INFO];


static customDataPacket stCustomCommand;

#define MESSAGE_HEADER_SIZE 4
typedef enum {
          SOH  = 0xAA,
          ETX  = 0x03,
          ACK  = 0x06,
          NAK  = 0x15,
} controlChars;

enum {
	WAIT_SOH      = 0,
	WAIT_HEADER   = 1,
	WAIT_DATA     = 2,
	WAIT_CRC      = 3,
} MessageDecoderStatus;

MessagePacket_t gRxMessage;
MessagePacket_t gTxMessage;
static stSensorData gSensorData;


static void cp_uart_task(void);
static void cp_uart_timer_task(void);

uint16_t outterm_send(uint8_t * buff, uint16_t len)
{
	return CDC_Transmit_Buffered(buff, len,CDC_SERIAL2);
}

uint16_t sp_uart_send(uint8_t * buff, uint16_t len)
{
	return CDC_Transmit_Buffered(buff, len,CDC_SERIAL0);
}

int sp_uart_receive(uint8_t* _char)
{
	return CDC_Get_Char_FS(_char,CDC_SERIAL0);
}

int   cp_uart_init(void)
{
	u8ActiveState = 0;
	u8TickState   = 0;

	stCustomCommand.Len = 16;
	memset(stCustomCommand.buffer,0x00,255);

	add_mainloop_funct(cp_uart_task, "cp_uart",1,0);
	add_cyclical_funct(cp_uart_timer_task, 10, "cp_uart timer\0",0);
	memset(l_date,0,SIZE_DATE_INFO);
	return 0;
}

void cp_uart_timer_task(void)
{
	u8TickState = 1;
}

void cp_uart_task()
{
	int nRet = 0;

	nRet = cp_process_packet();
	if(nRet == RX_OK)
	{
		cp_decode_packet();
	}
}
static int cp_process_packet(void)
{
	uint8_t bRet;
	uint8_t btChar;
	static uint8_t  u8RxStatus        = 0;
	static uint8_t  u8ReadCnt         = 0;
	static uint8_t  u8CRC_Calc        = 0;
	static uint32_t startTimer        = 0;
	static uint32_t timeoutTimer      = 0;
	uint8_t bElapsed = 0;
	int nRet = 0x00;

	uint8_t* pMessage = (uint8_t*)&gRxMessage;

	bElapsed = time_elapsed_ms(startTimer,timeoutTimer);
	if((u8RxStatus > WAIT_SOH) && (bElapsed > 0))
	{
		u8RxStatus  = WAIT_SOH;
		nRet        = RX_ERR_TIMEOUT;
	}

	bRet = sp_uart_receive(&btChar);
	if(bRet == 0)
	{
		switch (u8RxStatus){
		case WAIT_SOH:
			pMessage[0] = btChar;
			if(btChar == SOH)
			{
				u8RxStatus                 = WAIT_HEADER;
				u8ReadCnt                  = 1;
				u8CRC_Calc                 = 0;
				startTimer                 = _get_sys_clock();
				timeoutTimer               = 10;
			}
			break;
		case WAIT_HEADER:
			u8CRC_Calc ^= btChar;
			pMessage[u8ReadCnt] = btChar;
			u8ReadCnt++;
			if(u8ReadCnt == MESSAGE_HEADER_SIZE+1)
			{
				u8ReadCnt    = 0;
				u8RxStatus   = WAIT_DATA;
				startTimer   = _get_sys_clock();
				timeoutTimer = gRxMessage.len*4+10;
			}
			break;
		case WAIT_DATA:
			u8CRC_Calc ^= btChar;
			gRxMessage.RegBuffer[u8ReadCnt] = btChar;
			u8ReadCnt++;
			if(u8ReadCnt >= (gRxMessage.len+1))
			{
				u8RxStatus = WAIT_CRC;
			}
			break;
		case WAIT_CRC:
			if(btChar == u8CRC_Calc)
			{
				nRet       = RX_OK;
				gRxMessage.Ready = 1;
			}
			else
			{
				nRet       = RX_ERR_CHK;
				u8ReadCnt  = 0;
			}
			u8RxStatus  = WAIT_SOH;
			break;
		default :
			u8RxStatus  = WAIT_SOH;
			break;
		}
	}

	return nRet;
}


static int cp_send_packet(MessagePacket_t* pMessage)
{
	static uint8_t pBuffer[256];
	uint8_t crc = 0x00;
	uint16_t Index = 0;
	int i;
	pBuffer[Index++] = SOH;
	pBuffer[Index++] = pMessage->source;
	pBuffer[Index++] = pMessage->destination;
	pBuffer[Index++] = pMessage->command_id;
	pBuffer[Index++] = pMessage->len;
	for(i=0;i<pMessage->len+1;i++)
	{
		pBuffer[Index++] = pMessage->RegBuffer[i];
	}
	for(i=1;i<=MESSAGE_HEADER_SIZE+pMessage->len+1;i++)
	{
		crc ^= pBuffer[i];
	}
	pBuffer[Index++] = crc;

	sp_uart_send(pBuffer, Index);
	return 0;
}

static int  cp_decode_packet()
{
	static uint32_t rx_cnt = 0;
	if(gRxMessage.destination != ID_MICRO) return -1;

	rx_cnt++;
	switch(gRxMessage.command_id)
	{
	case CMD_RET_STATUS:
		break;
	case CMD_ACK_LEDS:
		break;
	case CMD_RET_PARAM:
		break;
	case CMD_PING_ACK:
		printf("PING ACK!\r\n");
		setReadyStatus();
		break;
	case CMD_RET_DATE://legge il tempo da linux che ha internet
		uint32_t unixTime;
		memcpy((uint8_t*)&unixTime,(unsigned char*)gRxMessage.RegBuffer,4);
		rtc_unix_write_utc((time_t)unixTime);
		break;
	case CMD_RX_CUSTOM:
		//valuto lunghezza
		if(gRxMessage.len > 255)
			gRxMessage.len = 255;
		stCustomCommand.Len = gRxMessage.len;
		memcpy(stCustomCommand.buffer,(unsigned char*)gRxMessage.RegBuffer,stCustomCommand.Len);
		break;
	}

	return 0;
}

int  request_status(void)
{
	MessagePacket_t gTxMessage;
	static uint8_t cnt = 0;
	int nBytes = 0;

	gTxMessage.source       = ID_MICRO;
	gTxMessage.destination  = ID_CPU;
	gTxMessage.command_id   = CMD_GET_STATUS;
	nBytes += unpack8(&gTxMessage.RegBuffer[nBytes],(uint8_t)cnt++);
	gTxMessage.len          = nBytes-1;

	cp_send_packet(&gTxMessage);

	return 0;
}
int  send_sensor_data(void)
{
	MessagePacket_t gTxMessage;
	int nBytes = 0;
	uint32_t * u32fVal;
	uint32_t tUnixTime = (uint32_t)rtc_unix_read_utc();

	gSensorData.BatteryChargeLevelRelative = getBatteryRelativeStateOfCharge();
	gSensorData.BatteryCurrent             = getBatteryCurrentMa();
	gSensorData.BatteryVoltage             = getBatteryVoltageMv();
	gSensorData.BatteryAverageCurrent      = getBatteryAverageCurrentMa();
	gSensorData.bmePressure                = get_bme_pressure();
	gSensorData.bmeTemp                    = get_bme_temp();
	gSensorData.bmeHum                     = get_bme_humidity();


	gTxMessage.source       = ID_MICRO;
	gTxMessage.destination  = ID_CPU;
	gTxMessage.command_id   = CMD_SEND_SENSOR;

	nBytes += unpack32le(&gTxMessage.RegBuffer[nBytes],tUnixTime);

	nBytes += unpack16le(&gTxMessage.RegBuffer[nBytes],gSensorData.BatteryVoltage);
	nBytes += unpack16le(&gTxMessage.RegBuffer[nBytes],gSensorData.BatteryCurrent);
	nBytes += unpack16le(&gTxMessage.RegBuffer[nBytes],gSensorData.BatteryChargeLevelRelative);
	nBytes += unpack16le(&gTxMessage.RegBuffer[nBytes],gSensorData.BatteryChargeLevelAbs);
	nBytes += unpack16le(&gTxMessage.RegBuffer[nBytes],gSensorData.BatteryAverageCurrent);

	nBytes += unpack8(&gTxMessage.RegBuffer[nBytes],gSensorData.pir_state[0]);
	nBytes += unpack8(&gTxMessage.RegBuffer[nBytes],gSensorData.pir_state[1]);
	nBytes += unpack8(&gTxMessage.RegBuffer[nBytes],gSensorData.pir_state[2]);
	nBytes += unpack8(&gTxMessage.RegBuffer[nBytes],gSensorData.pir_state[3]);

	u32fVal = (uint32_t*)&gSensorData.bmePressure;
	nBytes += unpack32le(&gTxMessage.RegBuffer[nBytes],*u32fVal);

	u32fVal = (uint32_t*)&gSensorData.bmeTemp;
	nBytes += unpack32le(&gTxMessage.RegBuffer[nBytes],*u32fVal);

	u32fVal = (uint32_t*)&gSensorData.bmeHum;
	nBytes += unpack32le(&gTxMessage.RegBuffer[nBytes],*u32fVal);

	nBytes += unpack8(&gTxMessage.RegBuffer[nBytes],gSensorData.bUpdate);

	gTxMessage.len = nBytes-1;
	cp_send_packet(&gTxMessage);

	return 0;
}

int  request_flash(void)
{
	MessagePacket_t gTxMessage;
	static uint8_t cnt = 0;
	int nBytes = 0;

	gTxMessage.source       = ID_MICRO;
	gTxMessage.destination  = ID_CPU;
	gTxMessage.command_id   = CMD_LEDS_REQ;
	nBytes += unpack8(&gTxMessage.RegBuffer[nBytes],(uint8_t)cnt++);
	gTxMessage.len          = nBytes-1;
	cp_send_packet(&gTxMessage);

	return 0;
}
int  request_halt(void)
{
	MessagePacket_t gTxMessage;
	static uint8_t cnt = 0;
	int nBytes = 0;

	gTxMessage.source       = ID_MICRO;
	gTxMessage.destination  = ID_CPU;
	gTxMessage.command_id   = CMD_HALT_REQ;
	nBytes += unpack8(&gTxMessage.RegBuffer[nBytes],(uint8_t)cnt++);
	gTxMessage.len          = nBytes-1;
	cp_send_packet(&gTxMessage);

	return 0;
}
int  request_param(void)
{
	MessagePacket_t gTxMessage;
	static uint8_t cnt = 0;
	int nBytes = 0;

	gTxMessage.source       = ID_MICRO;
	gTxMessage.destination  = ID_CPU;
	gTxMessage.command_id   = CMD_GET_PARAM;
	nBytes += unpack8(&gTxMessage.RegBuffer[nBytes],(uint8_t)cnt++);
	gTxMessage.len          = nBytes-1;
	cp_send_packet(&gTxMessage);

	return 0;
}

int  request_date(void)
{//richiede il tempo da linux che ha internet
	MessagePacket_t gTxMessage;
	static uint8_t cnt = 0;
	int nBytes = 0;

	gTxMessage.source       = ID_MICRO;
	gTxMessage.destination  = ID_CPU;
	gTxMessage.command_id   = CMD_GET_DATE;
	nBytes += unpack8(&gTxMessage.RegBuffer[nBytes],(uint8_t)cnt++);
	gTxMessage.len          = nBytes-1;
	cp_send_packet(&gTxMessage);
	return 0;
}

int  send_ping(void)
{
	MessagePacket_t gTxMessage;
	static uint8_t cnt = 0;
	int nBytes = 0;
	resetReadyStatus();
	gTxMessage.source       = ID_MICRO;
	gTxMessage.destination  = ID_CPU;
	gTxMessage.command_id   = CMD_PING_REQ;
	nBytes += unpack8(&gTxMessage.RegBuffer[nBytes],(uint8_t)cnt++);
	gTxMessage.len          = nBytes-1;
	cp_send_packet(&gTxMessage);
	return 0;
}
