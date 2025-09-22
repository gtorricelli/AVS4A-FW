
/**
  ******************************************************************************

  EEPROM.c Using the HAL I2C Functions
  Author:   ControllersTech
  Updated:  Feb 16, 2021

  ******************************************************************************
  Copyright (C) 2017 ControllersTech.com

  This is a free software under the GNU license, you can redistribute it and/or modify it under the terms
  of the GNU General Public License version 3 as published by the Free Software Foundation.
  This software library is shared with public for educational purposes, without WARRANTY and Author is not liable for any damages caused directly
  or indirectly by this software, read more about this on the GNU General Public License.

  ******************************************************************************
*/

#include <eeprom.h>
#include "math.h"
#include "string.h"
#include "bsp.h"

// Define the I2C
#define EEPROM_I2C &hi2c1

/*****************************************************************************************************************************************/
/* write the data to the EEPROM
 * @page is the number of the start page. Range from 0 to PAGE_NUM-1
 * @offset is the start byte offset in the page. Range from 0 to PAGE_SIZE-1
 * @data is the pointer to the data to write in bytes
 * @size is the size of the data
 */
void eeprom_write(uint16_t offset, uint8_t *data, uint16_t size)
{
	HAL_I2C_Mem_Write(EEPROM_I2C, EEPROM_ADDR, offset, 2, &data[0], size, 1000);  // write the data to the EEPROM
}

/* READ the data from the EEPROM
 * @page is the number of the start page. Range from 0 to PAGE_NUM-1
 * @offset is the start byte offset in the page. Range from 0 to PAGE_SIZE-1
 * @data is the pointer to the data to write in bytes
 * @size is the size of the data
 */
void eeprom_read(uint16_t offset, uint8_t *data, uint16_t size)
{
	HAL_I2C_Mem_Read(EEPROM_I2C, EEPROM_ADDR, offset, 2, &data[0], size, 1000);
}

