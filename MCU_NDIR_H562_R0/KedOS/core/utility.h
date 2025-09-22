#ifndef UTILITY_H
#define UTILITY_H

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
void memset8(uint8_t*, uint8_t, uint32_t);
void memcpy32 (uint32_t* src, uint32_t* dst, uint32_t length );
void str_copy(char*,char*,uint8_t);
void int8_t_to_char(uint8_t*, uint8_t);
void word_to_char(uint8_t*, uint16_t);
void long_to_char(uint8_t*, uint32_t);
void int8_t_to_bcd(uint8_t*, uint8_t);
void uint8_t_to_string(uint8_t n, uint8_t* str, uint8_t base);
uint8_t uint16_t_to_string(uint16_t n, int8_t* str, uint8_t base);
void sint8_t_to_string(int8_t n, int8_t* str);
void sword_to_string(int16_t n, int8_t* str);
uint8_t uint32_t_to_string(uint32_t n, int8_t* str, uint8_t base);
uint8_t trim_zero(int8_t *cpy, const int8_t* src);

#endif	/* UTILITY_H*/
