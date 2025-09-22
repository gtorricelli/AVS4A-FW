/**
  ******************************************************************************
  * @file    usbd_cdc_if_template.c
  * @author  MCD Application Team
  * @brief   Generic media access Layer.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include <usbd_cdc_if.h>
#include "usb.h"
/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_CDC
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_CDC_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CDC_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CDC_Private_Macros
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_CDC_Private_FunctionPrototypes
  * @{
  */

/* Exported constants --------------------------------------------------------*/
#define APP_RX_DATA_SIZE  512
#define APP_TX_DATA_SIZE  512

#define MAX_CDC_USB 3
uint8_t UserRxBufferFS[MAX_CDC_USB][APP_RX_DATA_SIZE];

extern USBD_HandleTypeDef hUsbDeviceFS;

#define RX_QUEUE_LEN 1024
static uint8_t  RX_BUFFER_QUEUE[MAX_CDC_USB][RX_QUEUE_LEN];
static uint16_t RX_BUFFER_LEN[MAX_CDC_USB] = {0,0,0};
static uint16_t RX_WRITE_POS[MAX_CDC_USB]  = {0,0,0};
static uint16_t RX_READ_POS[MAX_CDC_USB]   = {0,0,0};

#define         DMA_TX_BUFFER 1024
static          uint8_t u8UartTxBuffer[MAX_CDC_USB][DMA_TX_BUFFER];
uint32_t        txBufferTailFS[MAX_CDC_USB]   = {0,0,0};
uint32_t        txBufferHeadFS[MAX_CDC_USB]   = {0,0,0};

static uint8_t  ConnectionStatus[MAX_CDC_USB] = {255,255,255};
static uint8_t  usb_connected[MAX_CDC_USB]  = {0,0,0};
static uint8_t  port_connected[MAX_CDC_USB] = {0,0,0};

#define USB_CDC_ID0 0
#define USB_CDC_ID1 1
#define USB_CDC_ID2 2


static int8_t CDC_Init(void);
static int8_t CDC_DeInit(void);
static int8_t CDC_Control(uint8_t cmd, uint8_t *pbuf, uint16_t length, uint8_t ClassId);
static int8_t CDC_Receive(uint8_t *pbuf, uint32_t *Len, uint8_t ClassId);
static int8_t CDC_TransmitCplt(uint8_t *pbuf, uint32_t *Len, uint8_t ClassId);

USBD_CDC_ItfTypeDef USBD_CDC_fops =
{
  CDC_Init,
  CDC_DeInit,
  CDC_Control,
  CDC_Receive,
  CDC_TransmitCplt
};

USBD_CDC_LineCodingTypeDef linecoding =
{
  115200, /* baud rate*/
  0x00,   /* stop bits-1*/
  0x00,   /* parity - none*/
  0x08    /* nb. of bits 8*/
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  CDC_Init
  *         Initializes the CDC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init(void)
{
	if (hUsbDeviceFS.classId == USBD_CMPSIT_GetClassID(&hUsbDeviceFS, CLASS_TYPE_CDC, 0))
	{
		memset(RX_BUFFER_QUEUE[0], 0x00, RX_QUEUE_LEN);
		RX_BUFFER_LEN[0] = 0;
		RX_WRITE_POS[0]  = 0;
		RX_READ_POS[0]   = 0;
		memset(u8UartTxBuffer[0], 0x00, DMA_TX_BUFFER);
		txBufferTailFS[0] = 0;
		txBufferHeadFS[0] = 0;
	    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS[0]);
	}
	/* Check if the class ID corresponds to instance 1 */
	else if (hUsbDeviceFS.classId == USBD_CMPSIT_GetClassID(&hUsbDeviceFS, CLASS_TYPE_CDC, 1))
	{
		memset(RX_BUFFER_QUEUE[1], 0x00, RX_QUEUE_LEN);
		RX_BUFFER_LEN[1] = 0;
		RX_WRITE_POS[1]  = 0;
		RX_READ_POS[1]   = 0;
		memset(u8UartTxBuffer[1], 0x00, DMA_TX_BUFFER);
		txBufferTailFS[1] = 0;
		txBufferHeadFS[1] = 0;
		USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS[1]);
	}
	/* Check if the class ID corresponds to instance 1 */
	else if (hUsbDeviceFS.classId == USBD_CMPSIT_GetClassID(&hUsbDeviceFS, CLASS_TYPE_CDC, 2))
	{
		memset(RX_BUFFER_QUEUE[2], 0x00, RX_QUEUE_LEN);
		RX_BUFFER_LEN[2] = 0;
		RX_WRITE_POS[2]  = 0;
		RX_READ_POS[2]   = 0;
		memset(u8UartTxBuffer[2], 0x00, DMA_TX_BUFFER);
		txBufferTailFS[2] = 0;
		txBufferHeadFS[2] = 0;
		USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS[2]);
	}
	else
	{
	/* Error: no instancecorresponds to this class ID */
	 return USBD_FAIL;
	}
    return (0);
}

/**
  * @brief  CDC_DeInit
  *         DeInitializes the CDC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit(void)
{
	if (hUsbDeviceFS.classId == USBD_CMPSIT_GetClassID(&hUsbDeviceFS, CLASS_TYPE_CDC, 0)) {
		/* DeInitialize the UART peripheral */
		memset(RX_BUFFER_QUEUE[0], 0x00, RX_QUEUE_LEN);
		RX_BUFFER_LEN[0] = 0;
		RX_WRITE_POS[0]  = 0;
		RX_READ_POS[0]   = 0;
		return (USBD_OK);
	}
	/* Check if the class ID corresponds to instance 1 */
	else if (hUsbDeviceFS.classId == USBD_CMPSIT_GetClassID(&hUsbDeviceFS, CLASS_TYPE_CDC, 1)) {
		/* Reset pointers */
		memset(RX_BUFFER_QUEUE[1], 0x00, RX_QUEUE_LEN);
		RX_BUFFER_LEN[1] = 0;
		RX_WRITE_POS[1]  = 0;
		RX_READ_POS[1]   = 0;

		return (USBD_OK);

	}	/* Check if the class ID corresponds to instance 1 */
	else if (hUsbDeviceFS.classId == USBD_CMPSIT_GetClassID(&hUsbDeviceFS, CLASS_TYPE_CDC, 2)) {
		/* Reset pointers */
		memset(RX_BUFFER_QUEUE[2], 0x00, RX_QUEUE_LEN);
		RX_BUFFER_LEN[2] = 0;
		RX_WRITE_POS[2]  = 0;
		RX_READ_POS[2]   = 0;

		return (USBD_OK);
	} else {
		/* Error: no instancecorresponds to this class ID */
		return USBD_FAIL;
	}
  return (0);
}


/**
  * @brief  CDC_Control
  *         Manage the CDC class requests
  * @param  Cmd: Command code
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control(uint8_t cmd, uint8_t *pbuf, uint16_t length, uint8_t ClassId)
{
  UNUSED(length);
  USBD_SetupReqTypedef * req;

  switch (cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:
      /* Add your code here */
      break;

    case CDC_GET_ENCAPSULATED_RESPONSE:
      /* Add your code here */
      break;

    case CDC_SET_COMM_FEATURE:
      /* Add your code here */
      break;

    case CDC_GET_COMM_FEATURE:
      /* Add your code here */
      break;

    case CDC_CLEAR_COMM_FEATURE:
      /* Add your code here */
      break;

    case CDC_SET_LINE_CODING:
        /* Manage HW only for instance 0 */

      linecoding.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) | \
                                         (pbuf[2] << 16) | (pbuf[3] << 24));
      linecoding.format     = pbuf[4];
      linecoding.paritytype = pbuf[5];
      linecoding.datatype   = pbuf[6];

      /* Add your code here */
      break;

    case CDC_GET_LINE_CODING:
      pbuf[0] = (uint8_t)(linecoding.bitrate);
      pbuf[1] = (uint8_t)(linecoding.bitrate >> 8);
      pbuf[2] = (uint8_t)(linecoding.bitrate >> 16);
      pbuf[3] = (uint8_t)(linecoding.bitrate >> 24);
      pbuf[4] = linecoding.format;
      pbuf[5] = linecoding.paritytype;
      pbuf[6] = linecoding.datatype;

      /* Add your code here */
      break;

    case CDC_SET_CONTROL_LINE_STATE:
		if(ConnectionStatus[ClassId] != hUsbDeviceFS.dev_state)
		{
	    	if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) {
	    		usb_connected[ClassId] = 1;
	    	}
	    	else
	    	{
	    		usb_connected[ClassId] = 0;
	    	}
		}
		ConnectionStatus[ClassId] = hUsbDeviceFS.dev_state;
    	if(ConnectionStatus[ClassId] == USBD_STATE_CONFIGURED)
    	{
    		req = (USBD_SetupReqTypedef *)pbuf;
    		if((req->wValue &0x0001) != 0)
    		{
    			port_connected[ClassId] = 1;
    		}
    		else
    		{
    			port_connected[ClassId] = 0;
    		}
    	}
    	else
    	{
    		port_connected[ClassId] = 0;
    	}
      break;

    case CDC_SEND_BREAK:
      /* Add your code here */
      break;

    default:
      break;
  }

  return (0);
}

/**
  * @brief  CDC_Receive
  *         Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive(uint8_t *Buf, uint32_t *Len, uint8_t ClassId)
{
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    int nTail =0 ;

    if(ClassId >= USB_CDC_NUMBER) return 0;

    nTail = (RX_QUEUE_LEN-RX_BUFFER_LEN[ClassId]);
    if(nTail>*Len)
    {
  	  memcpy(&RX_BUFFER_QUEUE[ClassId][RX_WRITE_POS[ClassId]],UserRxBufferFS[ClassId],*Len);
  	  RX_WRITE_POS[ClassId]+=*Len;
  	  RX_BUFFER_LEN[ClassId]+=*Len;
  	  if(RX_BUFFER_LEN[ClassId]>RX_QUEUE_LEN)RX_BUFFER_LEN[ClassId] = RX_QUEUE_LEN;
    }
    else
    {
  	  memcpy(&RX_BUFFER_QUEUE[ClassId][RX_WRITE_POS[ClassId]],UserRxBufferFS[ClassId],nTail);
  	  memcpy(&RX_BUFFER_QUEUE[0],UserRxBufferFS[ClassId]+nTail,*Len-nTail);
  	  RX_WRITE_POS[ClassId]=*Len-nTail;
  	  RX_BUFFER_LEN[ClassId]+=*Len;
  	  if(RX_BUFFER_LEN[ClassId]>RX_QUEUE_LEN)RX_BUFFER_LEN[ClassId] = RX_QUEUE_LEN;
    }
    return (USBD_OK);
}

int CDC_Get_Char_FS(uint8_t* Buf,uint8_t ClassId)
{
	if(RX_BUFFER_LEN[ClassId]>0)
	{
		*Buf=RX_BUFFER_QUEUE[ClassId][RX_READ_POS[ClassId]];
		RX_READ_POS[ClassId]++;
		if(RX_READ_POS[ClassId]>=RX_QUEUE_LEN)
			RX_READ_POS[ClassId] = 0;
		RX_BUFFER_LEN[ClassId]--;
		return 0;
	}
	return -1;
}

int CDC_USB_IsConnected(uint8_t ClassId)
{
	return usb_connected[ClassId];
}

int CDC_USB_IsPortOpen(uint8_t ClassId)
{
	return port_connected[ClassId];
}

uint8_t CDC_Transmit(uint8_t* Buf, uint16_t Len, uint8_t ClassId)
{
  uint8_t result = USBD_OK;
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassDataCmsit[ClassId];

  if(hcdc == 0) return 0;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len, ClassId);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS, ClassId);
  return result;
}

static uint8_t CDC_Trasmit_Fifo(uint8_t ClassId)
{
	uint8_t result = USBD_OK;
	volatile uint16_t Len;
	uint8_t* pBuffer;
	pBuffer = u8UartTxBuffer[ClassId]+txBufferTailFS[ClassId];
	if (txBufferHeadFS[ClassId] > txBufferTailFS[ClassId]) {
		Len = txBufferHeadFS[ClassId] - txBufferTailFS[ClassId];
	} else {
		Len = DMA_TX_BUFFER - txBufferTailFS[ClassId];
	}
	result = CDC_Transmit((uint8_t *)pBuffer, Len,ClassId);
	return result;
}

uint8_t CDC_Transmit_Buffered(uint8_t * buff, uint16_t len, uint8_t ClassId)
{
	uint16_t nRet = 0;
    uint16_t top = DMA_TX_BUFFER - txBufferHeadFS[ClassId];
	uint8_t* pBuffer;
    if(top > len)
    {
    	pBuffer = u8UartTxBuffer[ClassId]+txBufferHeadFS[ClassId];
    	memcpy(pBuffer,buff,len);
    }
    else
    {
    	pBuffer = u8UartTxBuffer[ClassId]+txBufferHeadFS[ClassId];
    	memcpy(pBuffer,buff,top);
    	pBuffer = u8UartTxBuffer[ClassId];
    	memcpy(pBuffer,&buff[top],len-top);
    }
    txBufferHeadFS[ClassId] = (txBufferHeadFS[ClassId] + len) % DMA_TX_BUFFER;
    CDC_Trasmit_Fifo(ClassId);
    return nRet;
}

/**
  * @brief  CDC_TransmitCplt
  *         Data transmitted callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_TransmitCplt(uint8_t *Buf, uint32_t *Len, uint8_t ClassId)
{
  txBufferTailFS[ClassId] = (txBufferTailFS[ClassId] + *Len) % DMA_TX_BUFFER;
  if (txBufferHeadFS[ClassId] != txBufferTailFS[ClassId]) {
	  CDC_Trasmit_Fifo(ClassId);
  }
  return (0);
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

