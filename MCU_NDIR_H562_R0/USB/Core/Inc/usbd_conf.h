/**
  ******************************************************************************
  * @file    usbd_conf_template.h
  * @author  MCD Application Team
  * @brief   Header file for the usbd_conf_template.c file
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CONF_H
#define __USBD_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx.h"  /* replace 'stm32xxx' with your HAL driver header filename, ex: stm32f4xx.h */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_CONF
  * @brief USB device low level driver configuration file
  * @{
  */

/** @defgroup USBD_CONF_Exported_Defines
  * @{
  */
/* Activate the IAD option */
#define USBD_COMPOSITE_USE_IAD                      1U

/* Activate the composite builder */
#define USE_USBD_COMPOSITE

/* Activate CDC class in composite builder */
#define USBD_CMPSIT_ACTIVATE_CDC                    1U

/* The definition of endpoint numbers must respect the order of classes instantiation*/
#define CDC1_OUT_EP                           0x01U  /* EP1 for CDC data OUT First Instance */
#define CDC1_IN_EP                            0x81U  /* EP1 for CDC data IN First Instance */
#define CDC1_CMD_EP                           0x82U  /* EP2 for CDC commands First Instance */

#define CDC2_OUT_EP                           0x03U  /* EP2 for CDC data OUT Second Instance */
#define CDC2_IN_EP                            0x83U  /* EP4 for CDC data IN Second Instance */
#define CDC2_CMD_EP                           0x84U  /* EP5 for CDC commands Second Instance */

#define CDC3_OUT_EP                           0x05U  /* EP2 for CDC data OUT Second Instance */
#define CDC3_IN_EP                            0x85U  /* EP4 for CDC data IN Second Instance */
#define CDC3_CMD_EP                           0x86U  /* EP5 for CDC commands Second Instance */


#define USBD_MAX_NUM_INTERFACES               6U
#define USBD_MAX_NUM_CONFIGURATION            1U
#define USBD_MAX_STR_DESC_SIZ                 0x100U
#define USBD_SUPPORT_USER_STRING              1U
#define USBD_SUPPORT_USER_STRING_DESC         1U
#define USBD_SELF_POWERED                     1U
#define USBD_DEBUG_LEVEL                      0U
#define USBD_CONFIG_STR_DESC_IDX              4U
#define USBD_CONFIG_BMATTRIBUTES              0xC0U
#define USBD_CONFIG_MAXPOWER                  0x32U


/** @defgroup USBD_Exported_Macros
  * @{
  */

/* Memory management macros make sure to use static memory allocation */
/** Alias for memory allocation. */
#define USBD_malloc         (void *)USBD_static_malloc

/** Alias for memory release. */
#define USBD_free           USBD_static_free

/** Alias for memory set. */
#define USBD_memset         memset

/** Alias for memory copy. */
#define USBD_memcpy         memcpy

/** Alias for delay. */
#define USBD_Delay          HAL_Delay

/* DEBUG macros */
#if (USBD_DEBUG_LEVEL > 0U)
#define  USBD_UsrLog(...)   do { \
                                 printf(__VA_ARGS__); \
                                 printf("\n"); \
                               } while (0)
#else
#define USBD_UsrLog(...) do {} while (0)
#endif /* (USBD_DEBUG_LEVEL > 0U) */

#if (USBD_DEBUG_LEVEL > 1U)

#define  USBD_ErrLog(...) do { \
                               printf("ERROR: ") ; \
                               printf(__VA_ARGS__); \
                               printf("\n"); \
                             } while (0)
#else
#define USBD_ErrLog(...) do {} while (0)
#endif /* (USBD_DEBUG_LEVEL > 1U) */

#if (USBD_DEBUG_LEVEL > 2U)
#define  USBD_DbgLog(...)   do { \
                                 printf("DEBUG : ") ; \
                                 printf(__VA_ARGS__); \
                                 printf("\n"); \
                               } while (0)
#else
#define USBD_DbgLog(...) do {} while (0)
#endif /* (USBD_DEBUG_LEVEL > 2U) */

/**
  * @}
  */



/**
  * @}
  */


/** @defgroup USBD_CONF_Exported_Types
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CONF_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBD_CONF_Exported_Variables
  * @{
  */
/**
  * @}
  */

/** @defgroup USBD_CONF_Exported_FunctionsPrototype
  * @{
  */
/* Exported functions -------------------------------------------------------*/
void *USBD_static_malloc(uint32_t size);
void USBD_static_free(void *p);
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CONF_H */


/**
  * @}
  */

/**
  * @}
  */
