/*
 * File name :  packing.c
 * Date :       2016-02-03
 * Author :     aricci
 * SMDS Code :  Utility
 *
 * Revision : 1.0
 *
 * Changelog :
 * 1.0  2016-05-05    
 *
 */
 
/**
 * @file
 * @brief .
 * @author .
 * @version 1.0
 *
 * @details .
 */

/***************************************
 * Included files
 **************************************/
#include "packing.h"

#include "assert.h"

#include "defines.h"
#include "types.h"


/***************************************
 * Private defines
 **************************************/


/***************************************
 * Private types
 **************************************/


/***************************************
 * Private functions declarations
 **************************************/

/***************************************
 * Private instances
 **************************************/


/***************************************
 * Public functions definitions
 **************************************/
/**
 * @brief
 * @details
 * @param
 * @return
 */
uint8_t unpack8(uint8_t *buf, uint8_t val)
{
  assert( buf != NULL );

  buf[0] = val;

  return 1u;
}


/**
 * @brief
 * @details
 * @param
 * @return
 */
uint8_t unpack16(uint8_t *buf, uint16_t val)
{
  assert( buf != NULL );

  buf[0] = (uint8_t) (val >> 8u);
  buf[1] = (uint8_t) (val >> 0u);

  return 2u;
}


/**
 * @brief
 * @details
 * @param
 * @return
 */
uint8_t unpack32(uint8_t *buf, uint32_t val)
{
  assert( buf != NULL );

  buf[0] = (uint8_t) (val >> 24u);
  buf[1] = (uint8_t) (val >> 16u);
  buf[2] = (uint8_t) (val >> 8u);
  buf[3] = (uint8_t) (val >> 0u);

  return 4u;
}


/**
 * @brief
 * @details
 * @param
 * @return
 */
uint8_t unpack16le(uint8_t *buf, uint16_t val)
{
  assert( buf != NULL );

  buf[0] = (uint8_t) (val >> 0u);
  buf[1] = (uint8_t) (val >> 8u);

  return 2u;
}


/**
 * @brief
 * @details
 * @param
 * @return
 */
uint8_t unpack32le(uint8_t *buf, uint32_t val)
{
  assert( buf != NULL );

  buf[0] = (uint8_t) (val >> 0u);
  buf[1] = (uint8_t) (val >> 8u);
  buf[2] = (uint8_t) (val >> 16u);
  buf[3] = (uint8_t) (val >> 24u);

  return 4u;
}


/**
 * @brief
 * @details
 * @param
 * @return
 */
uint8_t pack8(const uint8_t *buf, uint8_t *val)
{
  assert( (buf != NULL) && (val != NULL) );

  *val = buf[0];

  return 1u;
}


/**
 * @brief
 * @details
 * @param
 * @return
 */
uint8_t pack16(const uint8_t *buf, uint16_t *val)
{
  assert( (buf != NULL) && (val != NULL) );

  *val  = ((uint16_t) buf[1]);
  *val  = *val | (uint16_t)(((uint16_t) buf[0]) << 8u);

  return 2u;
}


/**
 * @brief
 * @details
 * @param
 * @return
 */
uint8_t pack32(const uint8_t *buf, uint32_t *val)
{
  assert( (buf != NULL) && (val != NULL) );

  *val  = ((uint32_t) buf[3])                     |
           (uint32_t)(((uint32_t) buf[2]) << 8u)  |
           (uint32_t)(((uint32_t) buf[1]) << 16u) |
           (uint32_t)(((uint32_t) buf[0]) << 24u);

  return 4u;
}


/**
 * @brief
 * @details
 * @param
 * @return
 */
uint8_t pack16le(const uint8_t *buf, uint16_t *val)
{
  assert( (buf != NULL) && (val != NULL) );

  *val  = ((uint16_t) buf[0])     |
           (uint16_t)(((uint16_t) buf[1]) << 8u);

  return 2u;
}


/**
 * @brief
 * @details
 * @param
 * @return
 */
uint8_t pack32le(const uint8_t *buf, uint32_t *val)
{
  assert( (buf != NULL) && (val != NULL) );

  *val  = ((uint32_t) buf[0])         |
          ((uint32_t) buf[1]) << 8u   |
          ((uint32_t) buf[2]) << 16u  |
          ((uint32_t) buf[3]) << 24u;

  return 4u;
}


/***************************************
 * Private functions definitions
 **************************************/


/***************************************
 * End of file
 **************************************/
