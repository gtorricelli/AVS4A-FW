#ifndef DEFINES_H
#define DEFINES_H

/***************************************
 * Included files
 **************************************/
/***************************************
 * Included files
 **************************************/
#include "limits.h"
#include <stdint.h>

#include "types.h"
/***************************************
 * Public defines
 **************************************/
#define	REMAP 			(1)

//#define	SET_BIT(x, y)	(x) |= (y)
//#define	CLR_BIT(x, y)   (x) &= ~(y)
#define NIBBLE1(x)		((x) & 0x0F)
#define NIBBLE2(x)		(((x) & 0xF0) >> 4)
#define NIBBLE3(x)		(((x) & 0xF00) >> 8)
#define NIBBLE4(x)		(((x) & 0xF000) >> 12)

//#define ULONG_MAX		(0xFFFFFFFF)

/* General Values */
#define LED_ON			(0)
#define LED_OFF			(1)
#define SET_BIT_HIGH	(1)
#define SET_BIT_LOW		(0)
#define FALSE           (0)
#define TRUE            (1)


#define ENABLE_IRQ		__enable_irq();
#define DISABLE_IRQ		__disable_irq();


#define INC_TIMER(timer)    if(++timer<0)timer=0x7fffffff		



#ifdef MEMORY_ALLOCATION
    #define DEFMEM_INT
    #define DEFMEM_EST __no_init
#else
    #define DEFMEM_INT extern
    #define DEFMEM_EST extern __no_init
#endif


/***************************************
 * Public types
 **************************************/
/* none */

#endif	/* DEFINES_H */
