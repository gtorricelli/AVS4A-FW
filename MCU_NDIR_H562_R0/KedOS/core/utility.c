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
#include "utility.h"

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
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
void memset8(uint8_t* src, uint8_t value, uint32_t length)
{
    uint32_t ii;

	if ( src != NULL )
	{
		for (ii = 0; ii < length; ++ii)
		{
			*src++ = value;
		}
	}

    return;
}

void memcpy32 (uint32_t* src, uint32_t* dst, uint32_t length )
{
    uint32_t ii;

	if ( src != NULL )
	{
		for (ii = 0; ii < length; ++ii)
		{
			*dst++ = *src++ ;
		}
	}
}

/**
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
void int8_t_to_char( uint8_t *ch, uint8_t value )
{
    /* MSB */
	*ch = NIBBLE2(value) + 0x30;
	if (*ch > 0x39)
	{
		*ch += 7;
	}

	ch++;

    /* LSB */
	*ch = NIBBLE1(value) + 0x30;
	if (*ch > 0x39)
	{
		*ch += 7;
	}

	ch++;

	*ch = 0x00;

	return;
}


/**
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
void uint8_t_to_string(uint8_t n, uint8_t* str, uint8_t base)
{
    uint8_t rem, value = n;
    uint8_t ii, size;

    size = (base == 10) ? 3 : 2;

    for (ii = 0; ii < size; ii++)
    {
        rem = value % base;
        value   = value / base;
        str[size - ii - 1] = (rem > 9) ? rem + 'A' - 10 : rem + '0';
    }

    str[ii] = '\0';

	trim_zero((int8_t*)str,(int8_t*)str);

	return;
}


/**
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
uint8_t uint16_t_to_string(uint16_t n, int8_t* str, uint8_t base)
{
    uint8_t rem;
    uint16_t value = n;
    uint8_t ii, size;

    size = (base == 10) ? 5 : 4;

    for (ii = 0; ii < size; ii++)
    {
        rem   = value % base;
        value = value / base;
        str[size - ii - 1] = (rem > 9) ? rem + 'A' - 10 : rem + '0';
    }

    str[ii] = '\0';

	size = size - trim_zero(str, str);

	return size;
}


/**
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
uint8_t uint32_t_to_string(uint32_t n, int8_t* str, uint8_t base)
{
    uint8_t rem;
    uint32_t value = n;
    uint8_t ii, size;

    size = (base == 10) ? 10 : 8;

    for (ii = 0; ii < size; ii++)
    {
        rem   = value % base;
        value = value / base;
        str[size - ii - 1] = (rem > 9) ? rem + 'A' - 10 : rem + '0';
    }

    str[ii] = '\0';

	size = size - trim_zero(str, str);

	return size;
}


/**
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
void sint8_t_to_string(int8_t n, int8_t* str)
{
	uint8_t rem, value;
	uint8_t ii, size   = 3;

	str[0] = (n < 0) ? '-' : '+';
	value  = (n < 0) ? -n : n;

	for (ii = 0; ii < size; ii++)
	{
	    rem   = value % 10;
	    value = value / 10;
	    str[size - ii] = (rem > 9) ? rem + 'A' - 10 : rem + '0';
	}

	str[ii + 1] = '\0';

	trim_zero(str, str);

	return;
}


/**
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
void int16_t_to_string(int16_t n, int8_t* str)
{
    uint8_t rem;
    uint16_t value;
    uint8_t ii, size   = 5;

    str[0] = (n < 0) ? '-' : '+';
    value  = (n < 0) ? -n : n;

    for (ii = 0; ii < size; ii++)
    {
        rem   = value % 10;
        value = value / 10;
        str[size - ii] = (rem > 9) ? rem + 'A' - 10 : rem + '0';
    }

    str[ii + 1] = '\0';
	
	trim_zero(str, str);

	return;
}


/**
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
void word_to_char( uint8_t *ch, uint16_t value )
{
    uint8_t temp;

    /* int8_t MSB */
    temp = ((value & 0xFF00 ) >> 8);
    int8_t_to_char(ch, temp);

    /* int8_t LSB */
    temp = value & 0x00FF ;
    int8_t_to_char(&ch[2], temp);
	
	return;
}


/**
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
void long_to_char( uint8_t *ch, uint32_t value )
{
    uint16_t temp;

    /* word MSB */
    temp = ( (value & 0xffff0000 ) >> 16 );
    word_to_char(ch, temp);

    /* int8_t LSB */
    temp = value & 0x0000ffff;
    word_to_char(&ch[4], temp);
	
	return;
}


/**
 * @brief A dummy function.
 *
 * @details This dummy function does nothing useful.
 * @param p1: Just a parameter.
 * @return outcome of the operation
 */
void int8_t_to_bcd( uint8_t *ch, uint8_t n )
{
    uint8_t rem, value = n;
    uint8_t ii;

    for (ii = 0; ii < 3; ii++)
    {
        rem = value % 10;
        value = value / 10;

        ch[2 - ii] = rem;
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
uint8_t trim_zero(int8_t *cpy, const int8_t* src)
{
	bool_t stop = FALSE;
	uint8_t ii = 0;

	while (*src != '\0')
	{
		if (stop == FALSE)
		{
			if (*src != '0')
			{
				stop = TRUE;
				*cpy++ = *src;
			}
			else
			{
				ii++;
			}
		}
		else
		{
			*cpy++ = *src;
		}
		
		src++;
	}

	if (stop == FALSE)
	{
		*cpy++ = '0';
	}

	*cpy = '\0';

	return ii;
}

void str_copy(char* src,char* cpy, uint8_t len_max)
{
	uint8_t ii = 0;

	while (*src != '\0' && ii<(len_max-1))
	{
		cpy[ii] = *src;
		src++;
		ii++;

	}
	while (ii<(len_max-1))
	{
		cpy[ii]=' ';
		ii++;
	}
	cpy[ii] = 0;

}


/***************************************
 * Private functions definitions
 **************************************/

/***************************************
 * End of file
 **************************************/
