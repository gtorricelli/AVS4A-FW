#ifndef SENSIRION_TEMPERATURE_UNIT_CONVERSION_H
#define SENSIRION_TEMPERATURE_UNIT_CONVERSION_H
#include "sensirion_arch_config.h"


/**
 * sensirion_celsius_to_fahrenheit() - Convert temperature in degree Celsius
 *                                     (Centigrade) to degree Fahrenheit
 *
 * Note that inputs not in the range -291degC <= degC <= 291 will result in
 * invalid results.
 *
 * @param temperature_milli_celsius     The temperature measurement in milli
 *                                      degree Celsius, i.e. degree Celsius
 *                                      multiplied by 1000.
 *
 * @return                              The temperature measurement in milli
 *                                      degree Fahrenheit, i.e. degree
 *                                      Fahrenheit multiplied by 1000.
 */
int32_t sensirion_celsius_to_fahrenheit(int32_t temperature_milli_celsius);

/**
 * sensirion_fahrenheit_to_celsius() - Convert temperature in degree Fahrenheit
 *                                     to degree Celsius (Centigrade)
 *
 * Note that inputs not in the range -3571degF <= degF <= 3635degF will result
 * in invalid results.
 *
 * @param temperature_milli_fahrenheit  The temperature measurement in milli
 *                                      degree Fahrenheit, i.e. degree
 *                                      Fahrenheit multiplied by 1000.
 *
 * @return                              The temperature measurement in milli
 *                                      degree Celsius, i.e. degree Celsius
 *                                      multiplied by 1000.
 */
int32_t sensirion_fahrenheit_to_celsius(int32_t temperature_milli_fahrenheit);

#endif /* SENSIRION_TEMPERATURE_UNIT_CONVERSION_H */
