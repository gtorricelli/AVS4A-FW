/*
 * crc.h
 *
 *  Created on: Nov 10, 2023
 *      Author: gpaol
 */

#ifndef INC_CRC_H_
#define INC_CRC_H_
#include "types.h"
uint16_t mbus_crc16(const uint16_t crc16, const uint8_t byte) ;
uint16_t usPacketCRC16(uint8_t *pucFrame, uint16_t usLen);


#endif /* INC_CRC_H_ */
