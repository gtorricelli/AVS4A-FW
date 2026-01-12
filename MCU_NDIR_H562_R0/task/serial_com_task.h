#ifndef SERIAL_COM_TASK_H_
#define SERIAL_COM_TASK_H_
#include "types.h"
#include "defines.h"

#define RX_OK                          1
#define NO_ERR                         0
#define RX_ERR_CMD                     -1
#define RX_ERR_ACK                     -2
#define RX_ERR_CHK                     -3
#define RX_ERR_TIMEOUT                 -4

// Definizione del comando seriale
#define MAX_DATA_LEN 128
typedef struct  __attribute__((packed))
{
    uint8_t  soh;
    uint8_t  source;
    uint8_t  destination;
    uint8_t  command_id;
    uint8_t  len;
    uint8_t  RegBuffer[MAX_DATA_LEN];
    uint8_t  crc;
    uint8_t  Ready;
} MessagePacket_t;

typedef struct  __attribute__((packed))
{
    uint32_t timestamp;
    uint8_t  softbkeepalive;
    uint32_t loop_time;
    uint8_t times_per_day;
} status_param_t;

#define CP_PROTOCOL_VERSION 0x00

// codici comando
typedef enum {
	CMD_GET_STATUS    = 0x00,
	CMD_RET_STATUS    = 0x80,
	CMD_GET_PARAM     = 0x01,
	CMD_RET_PARAM     = 0x81,
	CMD_PING_REQ      = 0x02,
	CMD_PING_ACK      = 0x82,
	CMD_LEDS_REQ      = 0x03,
	CMD_ACK_LEDS      = 0x83,
	CMD_HALT_REQ      = 0x04,
	CMD_HALT_ACK      = 0x84,
	CMD_GET_DATE      = 0x05,
	CMD_RET_DATE      = 0x85,
	CMD_SEND_SENSOR   = 0x06,
	CMD_ACK_SENSOR    = 0x86,
} cp_app_command;


enum e_board_id
{
	ID_CPU=0,
	ID_MICRO,
};

typedef struct {
	uint16_t   BatteryVoltage;
	int16_t    BatteryCurrent;
	uint16_t   BatteryChargeLevelAbs;/*0-1000 1000 = 100%*/
	uint16_t   BatteryChargeLevelRelative;/*0-1000 1000 = 100%*/
	int16_t    BatteryAverageCurrent;//corrente media della batteria
	uint8_t    pir_state[4];
	float      bmePressure;
	float      bmeTemp;
	float      bmeHum;
	uint8_t    bUpdate; //non x la cpu
} stSensorData;

typedef struct {
	uint32_t   TimeStamp;
	uint32_t   LoopTime; //seconds
} stLoopData;

int  cp_uart_init(void);
int  request_status(void);
int  send_sensor_data(void);
int  request_flash(void);
int  request_halt(void);
int  request_date(void);
int  send_ping(void);
uint16_t outterm_send(uint8_t * buff, uint16_t len);
#endif /* SERIAL_COM_TASK_H_ */
