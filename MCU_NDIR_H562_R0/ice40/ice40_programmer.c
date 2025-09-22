/*
 * ice40_programmer.c
 *
 *  Created on: Dec 20, 2024
 *      Author: rfacc
 */
#include "KedOS.h"
#include "main.h"
#include "bsp.h"
#include "spi.h"
#include "string.h"


extern unsigned int ICE40_CAMERA_CONTROL_bin_len;
extern unsigned char ICE40_CAMERA_CONTROL_bin[];

//#define CPLD_NSS_Pin GPIO_PIN_3
//#define CPLD_NSS_GPIO_Port GPIOB
//#define CPLD_RSTn_Pin GPIO_PIN_4
//#define CPLD_RSTn_GPIO_Port GPIOB
//#define CPLD_DONE_Pin GPIO_PIN_1
//#define CPLD_DONE_GPIO_Port GPIOB

#define ICE_SPI_CS_LOW()	HAL_GPIO_WritePin(CPLD_NSS1_GPIO_Port, CPLD_NSS1_Pin, GPIO_PIN_RESET)
#define ICE_SPI_CS_HIGH()	HAL_GPIO_WritePin(CPLD_NSS1_GPIO_Port, CPLD_NSS1_Pin, GPIO_PIN_SET)
#define ICE_CRST_LOW()		HAL_GPIO_WritePin(CPLD_RSTn_GPIO_Port, CPLD_RSTn_Pin, GPIO_PIN_RESET)
#define ICE_CRST_HIGH()		HAL_GPIO_WritePin(CPLD_RSTn_GPIO_Port, CPLD_RSTn_Pin, GPIO_PIN_SET)
#define ICE_CDONE_GET()		HAL_GPIO_ReadPin(CPLD_DONE_GPIO_Port,CPLD_DONE_Pin)
#define ICE_SPI_DUMMY_BYTE	0xFF
#define ICE_SPI_MAX_XFER	512

/*
 * Write a block of bytes to the ICE SPI
 */
void ICE_SPI_WriteBlk(uint8_t *Data, uint32_t Count)
{
    int ret;
	uint32_t bytes;
	while(Count)
	{
		bytes = (Count > ICE_SPI_MAX_XFER) ? ICE_SPI_MAX_XFER : Count;
		ret = HAL_SPI_Transmit(&hspi1, (uint8_t *)Data, bytes, HAL_MAX_DELAY);
		assert(ret==HAL_OK);            //Should have had no issues.
		Count -= bytes;
		Data += bytes;
	}
}

int ice40_programming()
{
	uint32_t timeout;
	int state = 0;
	int i;
	uint32_t Count;
	uint8_t * pData;

	/* drop reset bit */
	ICE_CRST_LOW();

	/* delay */
	HAL_Delay(1);

	/* drop CS bit to signal slave mode */
	ICE_SPI_CS_LOW();

	/* delay */
	HAL_Delay(1);

	/* Wait for done bit to go inactive */
	timeout = 100;
	while(timeout && (ICE_CDONE_GET()==1))
	{
		timeout--;
	}
	if(!timeout)
	{
		/* Done bit didn't respond to Reset */
		return 1;
	}

	/* raise reset */
	ICE_CRST_HIGH();

	/* delay >1200us to allow FPGA to clear */
	HAL_Delay(2);

	/* send the bitstream */

	pData = ICE40_CAMERA_CONTROL_bin;
	//search preamble
	for(i=0;i<ICE40_CAMERA_CONTROL_bin_len-4;i++)
	{
		if((pData[i] == 0x7E)&&(pData[i+1] == 0xAA)&&(pData[i+2] == 0x99)&&(pData[i+3] == 0x7E))
		{
			state = 1;
			break;
		}
	}

	pData = &ICE40_CAMERA_CONTROL_bin[i];
	Count = ICE40_CAMERA_CONTROL_bin_len -i;

	ICE_SPI_WriteBlk(pData, Count);

    /* new ending logic */
    /* raise CS */
	ICE_SPI_CS_HIGH();

    /* Quick send 160 dummy clocks */
	uint8_t dummy[20] = {0};
	ICE_SPI_WriteBlk(dummy, 20);

    /* error if DONE not asserted */
    if(ICE_CDONE_GET()==0)
    	return 2;

    /* no error handling for now */
	return 0;
}
