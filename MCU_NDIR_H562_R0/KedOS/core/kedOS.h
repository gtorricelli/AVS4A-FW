#ifndef kedOS_H
#define kedOS_H

/***************************************
 * Included files
 **************************************/
#include "bsp.h"
#include "defines.h"

#define CRITICAL_TASK 		1
#define	NO_CRITICAL_TASK	0


/***************************************
 * Public defines
 **************************************/
typedef struct
{
	uint8_t id;
	bool_t (*write)(uint8_t, const uint8_t*, uint8_t);
	bool_t (*read)(uint8_t, uint8_t*, uint8_t);
	void (*err)(uint8_t, uint8_t);
} OS_OBJ;

typedef void (*PT_FUNC_V)(void);
typedef void (*PT_FUNC_UB)(uint8_t );
typedef void (*PT_FUNC_UBUB)(uint8_t , uint8_t );


/***************************************
 * Public types
 **************************************/
void isr_funct(void);

/***************************************
 * API
 **************************************/
void    init_os(void (**main_loop_f)(void), void (**isr_f)(void));
bool_t  add_mainloop_funct(void (*ml_f)(void), char*, uint8_t, bool_t);
bool_t  remove_mainloop_funct(void (*ml_f)(void));
bool_t  add_cyclical_funct(void (*cyc_f)(void), uint16_t, char*,bool_t);
bool_t  remove_cyclical_funct(void (*cyc_f)(void)) ;
bool_t  test_if_mainloop_funct(void (*ml_f)(void));
uint8_t get_timing_mainloop(uint32_t* time);
void    get_name_mainloop(uint32_t , char*);
uint8_t get_timing_cyclical(uint32_t* );
void    reset_uos_timing(void);
void    get_name_cyclical(uint32_t ,char*);
bool_t  isTaskLive ( uint8_t code ) ;
bool_t  remove_no_critical_task ( void ) ;
uint32_t _get_os_timer();
uint32_t get_clock_ms(void);
uint32_t time_measure_ms(uint32_t _start);
bool_t   time_elapsed_ms(uint32_t _start, uint32_t _time);


#endif	/* uOS_H */
