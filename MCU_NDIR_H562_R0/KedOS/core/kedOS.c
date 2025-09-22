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
#include "kedOS.h"

#include "bsp.h"
#include "utility.h"




/***************************************
 * Private defines
 **************************************/
#define	MAX_MAIN_LOOP_FUNCT		(15)
#define	MAX_TIMING_FUNCT		(15)

/***************************************
 * Private types
 **************************************/
typedef struct
{
	void 	 (*funct)(void);
	uint8_t	 critical_task ;	// Indica se il task � critico (non � di quelli removibili)
	char     pid_name[10];
	uint8_t  code_name ;
	uint32_t performance;
} MAIN_LOOP_FUNCT_ST;

typedef struct
{
	void 	    (*funct)(void);
	bool_t	 	critical_task ;	// Indica se il task � critico (non � di quelli removibili)
	char        pid_name[10];
	uint16_t	cycles;			// @@@@ RG 20/12/11
	uint32_t	performance;
} CYCLICAL_FUNCT_ST;

 typedef struct
{
	MAIN_LOOP_FUNCT_ST functs[MAX_MAIN_LOOP_FUNCT];
	uint8_t	funct_size;
} LOOP_FUNCT_ST;



typedef struct
{
	CYCLICAL_FUNCT_ST functs[MAX_TIMING_FUNCT];
	uint8_t	funct_size;
} TIMING_FUNCT_ST;

 typedef struct
{
	LOOP_FUNCT_ST	 ml_functs;
	TIMING_FUNCT_ST	 t_functs;
	uint32_t	     isr_counter;
} uOS_ST;

/***************************************
 * Private functions declarations
 **************************************/
static void main_loop_funct(void);
/***************************************
 * Private instances
 **************************************/
static uOS_ST local ;

/***************************************
 * Public functions definitions
 **************************************/
/**
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
void init_os(void (**main_loop_f)(void), void (**isr_f)(void))
{
	uint8_t ii;
	
	for (ii = 0; ii < MAX_MAIN_LOOP_FUNCT; ii++)
	{
		local.ml_functs.functs[ii].funct = NULL;
	}

	for (ii = 0; ii < MAX_TIMING_FUNCT; ii++)
	{
		local.t_functs.functs[ii].funct = NULL;
		local.t_functs.functs[ii].cycles = 1;
	}
	
	local.ml_functs.funct_size = 0;
	local.t_functs.funct_size = 0;
	local.isr_counter = 0;

	*main_loop_f = main_loop_funct;
	*isr_f = isr_funct;

	return;
}


/**
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
bool_t add_mainloop_funct(void (*ml_f)(void), char* str_ID, uint8_t code, uint8_t bIsCritical )
{
	bool_t ret_val = FALSE;
	_disable_scheduler();
	
	if (ml_f != NULL)
	{
		if (local.ml_functs.funct_size < MAX_MAIN_LOOP_FUNCT)
		{
			local.ml_functs.functs[local.ml_functs.funct_size].funct = ml_f;
			str_copy(str_ID,local.ml_functs.functs[local.ml_functs.funct_size].pid_name,10);
			local.ml_functs.functs[local.ml_functs.funct_size].code_name = code ;
			local.ml_functs.functs[local.ml_functs.funct_size].performance = 0 ;
			local.ml_functs.functs[local.ml_functs.funct_size].critical_task = bIsCritical ;
			local.ml_functs.funct_size++;
			ret_val = TRUE;
		}
	}
	_enable_scheduler();
	
	return ret_val;
}

/**
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
bool_t remove_mainloop_funct(void (*ml_f)(void))
{
	bool_t ret_val = FALSE;
	bool_t bFindProcess = FALSE ;
	uint8_t u8_0,u8_1,u8_index;
	u8_index = MAX_MAIN_LOOP_FUNCT ;

	_disable_scheduler();

	if ( ml_f != NULL )
	{
		//find function index
		for ( u8_0 = 0 ; u8_0 < local.ml_functs.funct_size ; u8_0 ++ )
		{
			if ( local.ml_functs.functs[u8_0].funct == ml_f )
			{
				u8_index = u8_0 ;
				bFindProcess = TRUE ;
			}
		}
		if ( bFindProcess == FALSE )
		{
			_enable_scheduler();
			return ret_val ;
		}

		//remove function
		for( u8_0 = u8_index ; u8_0 < ( local.ml_functs.funct_size - 1 ) ; u8_0 ++ )
		{
			local.ml_functs.functs[u8_0].funct = local.ml_functs.functs[u8_0+1].funct;
			local.ml_functs.functs[u8_0].code_name = local.ml_functs.functs[u8_0+1].code_name;
			local.ml_functs.functs[u8_0].performance = local.ml_functs.functs[u8_0+1].performance;
			local.ml_functs.functs[u8_0].critical_task = local.ml_functs.functs[u8_0+1].critical_task ;
			for( u8_1 = 0 ; u8_1 < sizeof(local.ml_functs.functs[u8_0].pid_name); u8_1 ++ )
				local.ml_functs.functs[u8_0].pid_name[u8_1]= local.ml_functs.functs[u8_0+1].pid_name[u8_1];

		}
		local.ml_functs.funct_size -- ;
	}
	_enable_scheduler();

	return ret_val;
}

bool_t isTaskLive ( uint8_t code )
{
	bool_t ret_val = FALSE;
	uint8_t u8_c0;

	//find function index
	for ( u8_c0 = 0 ; u8_c0 < local.ml_functs.funct_size ; u8_c0 ++ )
	{
		if ( local.ml_functs.functs[ u8_c0 ].code_name == code )
		{
			ret_val = TRUE ;
		}
	}
	return ret_val;
}

uint8_t get_timing_mainloop(uint32_t* time)
{
	uint8_t i;
	for(i=0;i<local.ml_functs.funct_size;i++)
		time[i] = local.ml_functs.functs[i].performance;
	return local.ml_functs.funct_size;
}

void get_name_mainloop(uint32_t pid, char* Str)
{
	if(pid >= MAX_MAIN_LOOP_FUNCT )  return;
	str_copy(local.ml_functs.functs[pid].pid_name,Str,10);
}

uint8_t get_timing_cyclical(uint32_t* time)
{
	uint8_t i;
	for(i=0;i<local.t_functs.funct_size;i++)
		time[i] = local.t_functs.functs[i].performance;
	return local.t_functs.funct_size;
}
void reset_uos_timing(void)
{
	uint8_t i;
	for(i=0;i<local.t_functs.funct_size;i++)
		local.t_functs.functs[i].performance = 0;
	for(i=0;i<local.ml_functs.funct_size;i++)
		local.ml_functs.functs[i].performance = 0;
}

void get_name_cyclical(uint32_t pid, char* Str)
{
	if(pid >= MAX_TIMING_FUNCT )  return;
	str_copy(local.t_functs.functs[pid].pid_name,Str,10);

}



/**
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
bool_t add_cyclical_funct(void (*cyc_f)(void), uint16_t cycles, char* str_ID, bool_t bIsCritical)
{
	bool_t ret_val = FALSE;
	_disable_scheduler();

	if (cyc_f != NULL)
	{
		if (local.t_functs.funct_size < MAX_TIMING_FUNCT)
		{
			local.t_functs.functs[local.t_functs.funct_size].funct = cyc_f;
			str_copy(str_ID,local.t_functs.functs[local.t_functs.funct_size].pid_name,10);
			local.t_functs.functs[local.t_functs.funct_size].performance = 0 ;
			local.t_functs.functs[local.t_functs.funct_size].critical_task = bIsCritical ;
			if (cycles == 0)
			{
				local.t_functs.functs[local.t_functs.funct_size].cycles = 1;
			}
			else
			{
				local.t_functs.functs[local.t_functs.funct_size].cycles = cycles;
			}

			local.t_functs.funct_size++;
			
			ret_val = TRUE;
		}
	}
	_enable_scheduler();

	return ret_val;
}

bool_t remove_cyclical_funct(void (*cyc_f)(void))
{
	bool_t ret_val = FALSE;
	bool_t bFindProcess = FALSE ;
	uint8_t u8_0,u8_1,u8_index;
	u8_index=MAX_TIMING_FUNCT;

	_disable_scheduler();

	if ( cyc_f != NULL )
	{
		//find function index
		for ( u8_0 = 0 ; u8_0 < local.t_functs.funct_size ; u8_0 ++ )
		{
			if ( local.t_functs.functs[u8_0].funct == cyc_f )
			{
				u8_index = u8_0 ;
				bFindProcess = TRUE ;
			}
		}
		if ( bFindProcess == FALSE )
		{
			_enable_scheduler();
			return ret_val ;
		}

		// remove function
		for ( u8_0 = u8_index ; u8_0 < ( local.t_functs.funct_size - 1 ) ; u8_0 ++ )
		{
			local.t_functs.functs[u8_0].funct = local.t_functs.functs[u8_0+1].funct;
			local.t_functs.functs[u8_0].performance = local.t_functs.functs[u8_0+1].performance;
			local.t_functs.functs[u8_0].critical_task = local.t_functs.functs[u8_0+1].critical_task ;
			for( u8_1 = 0 ; u8_1 < sizeof(local.t_functs.functs[u8_0].pid_name); u8_1 ++ )
				local.t_functs.functs[u8_0].pid_name[u8_1]= local.t_functs.functs[u8_0+1].pid_name[u8_1];
		}
		local.t_functs.funct_size--;
	}

	_enable_scheduler();

	return ret_val;
}


/***************************************
 * Private functions definitions
 **************************************/
/**
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
static void main_loop_funct(void)
{
	uint8_t ii;
	uint32_t temp;

	for (ii=0; ii < local.ml_functs.funct_size; ii++)
	{
		temp =	 _get_sys_clock();
		local.ml_functs.functs[ii].funct();
		temp = _time_measure(temp);
		if (local.ml_functs.functs[ii].performance < temp)
		{
			local.ml_functs.functs[ii].performance = temp;
		}
	}

	return;
}

/**
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
void isr_funct(void)
{
	uint8_t ii;
	uint32_t temp;

	for (ii=0; ii < local.t_functs.funct_size; ii++)
	{
		if ( (local.isr_counter % local.t_functs.functs[ii].cycles) == 0)
		{
			temp = _get_sys_clock();
			local.t_functs.functs[ii].funct();
			temp = _time_measure(temp);
			if (local.t_functs.functs[ii].performance < temp)
			{
				local.t_functs.functs[ii].performance = temp;
			}
		}
	}

	local.isr_counter++;

	return;
}

/* -------------------------------------------------------------------------
   =========================================================================

                 R E M O V E _ N O _ C R I T I C A L _ T A S K

		Rimozione forzata di tutti i task main e cyclical taggati
		come non critici.
   =========================================================================
   ------------------------------------------------------------------------- */
bool_t remove_no_critical_task ( void )
{
	/* -------------------------- Variabili ---------------------------- */
	bool_t ret_val = FALSE;
	bool_t bFindProcess = FALSE ;
	uint8_t u8_0,u8_1,u8_index;
	u8_index = MAX_MAIN_LOOP_FUNCT ;
	/* ----------------------------------------------------------------- */

	_disable_scheduler();

	while ( 1 )
	{
		// find function no critical
		bFindProcess = FALSE ;
		u8_0 = local.ml_functs.funct_size - 1 ;
		do
		{
			if ( local.ml_functs.functs[u8_0].critical_task == NO_CRITICAL_TASK )
			{
				u8_index = u8_0 ;
				bFindProcess = TRUE ;
			}
			if ( u8_0 )
				u8_0 -- ;
		} while ( ( u8_0 > 0 ) && ( ! bFindProcess ) ) ;

		if ( bFindProcess == TRUE )
		{
			// remove function
			if ( u8_index != ( local.ml_functs.funct_size - 1 ) )
			{
				for( u8_0 = u8_index ; u8_0 < ( local.ml_functs.funct_size - 1 ) ; u8_0 ++ )
				{
					local.ml_functs.functs[u8_0].funct = local.ml_functs.functs[u8_0+1].funct;
					local.ml_functs.functs[u8_0].code_name = local.ml_functs.functs[u8_0+1].code_name;
					local.ml_functs.functs[u8_0].performance = local.ml_functs.functs[u8_0+1].performance;
					local.ml_functs.functs[u8_0].critical_task = local.ml_functs.functs[u8_0+1].critical_task;
					for( u8_1 = 0 ; u8_1 < sizeof(local.ml_functs.functs[u8_0].pid_name); u8_1 ++ )
						local.ml_functs.functs[u8_0].pid_name[u8_1]= local.ml_functs.functs[u8_0+1].pid_name[u8_1];

					ret_val = TRUE ;
				}
			}
			if(local.ml_functs.funct_size)
				local.ml_functs.funct_size -- ;
		}
		else
			break ;
	}


	while (1)
	{
		//find function index
		// find function no critical
		bFindProcess = FALSE ;
		u8_0 = local.t_functs.funct_size - 1 ;
		do
		{
			if ( local.t_functs.functs[u8_0].critical_task == NO_CRITICAL_TASK )
			{
				u8_index = u8_0 ;
				bFindProcess = TRUE ;
			}
			if ( u8_0 )
				u8_0 -- ;
		} while ( ( u8_0 > 0 ) && ( ! bFindProcess ) ) ;

		if ( bFindProcess == TRUE )
		{
			// remove function
			if ( u8_index != ( local.t_functs.funct_size - 1 ) )
			{
				for ( u8_0 = u8_index ; u8_0 < ( local.t_functs.funct_size - 1 ) ; u8_0 ++ )
				{
					local.t_functs.functs[u8_0].funct = local.t_functs.functs[u8_0+1].funct;
					local.t_functs.functs[u8_0].performance = local.t_functs.functs[u8_0+1].performance;
					local.t_functs.functs[u8_0].critical_task= local.t_functs.functs[u8_0+1].critical_task;
					for( u8_1 = 0 ; u8_1 < sizeof(local.t_functs.functs[u8_0].pid_name); u8_1 ++ )
						local.t_functs.functs[u8_0].pid_name[u8_1]= local.t_functs.functs[u8_0+1].pid_name[u8_1];
				}
				ret_val = TRUE ;
			}
			if(local.t_functs.funct_size)
				local.t_functs.funct_size -- ;
		}
		else
			break ;
	}

	_enable_scheduler();

	return ret_val;

}

uint32_t _get_os_timer()
{
	return local.isr_counter;
}


/***************************************
 * End of file
 **************************************/
