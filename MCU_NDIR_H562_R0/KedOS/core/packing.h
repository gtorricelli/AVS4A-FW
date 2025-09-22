/*
 * File name :  packing.h
 * Date :    2016-02-03
 * Author :    aricci
 * SMDS Code :  00702  Utility
 *
 * Revision :  1.0
 *
 * Changelog :
 * 1.0  2016-02-03  change header format and revision strategy (from SVN:389)
 *
 */
 
#ifndef PACKING_H
#define PACKING_H

/***************************************
 * Included files
 **************************************/
#include "bsp.h"
#include "defines.h"

/***************************************
 * Public defines
 **************************************/

/***************************************
 * Public types
 **************************************/

/***************************************
 * API
 **************************************/
extern uint8_t unpack8(uint8_t *buf, uint8_t val);
extern uint8_t unpack16(uint8_t *buf, uint16_t val);
extern uint8_t unpack32(uint8_t *buf, uint32_t val);
extern uint8_t unpack16le(uint8_t *buf, uint16_t val);
extern uint8_t unpack32le(uint8_t *buf, uint32_t val);
extern uint8_t pack8(const uint8_t *buf, uint8_t *val);
extern uint8_t pack16(const uint8_t *buf, uint16_t *val);
extern uint8_t pack32(const uint8_t *buf, uint32_t *val);
extern uint8_t pack16le(const uint8_t *buf, uint16_t *val);
extern uint8_t pack32le(const uint8_t *buf, uint32_t *val);



#endif  /* PACKING_H */
