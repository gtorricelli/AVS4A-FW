#ifndef HUMIDITY_CONVERSION_H
#define HUMIDITY_CONVERSION_H
#include "sensirion_arch_config.h"

/**
 * sensirion_calc_absolute_humidity() - Calculate absolute humidity from
 *                                      temperature and relative humidity
 *
 * @param temperature_milli_celsius The temperature measurement in milli Degree
 *                                  Celsius, i.e. degree celsius multiplied by
 *                                  1000.
 * @param humidity_milli_percent    The relative humidity measurement in
 *                                  milli Percent, i.e.  percent relative
 *                                  humidity, multiplied by 1000 (0-100000)
 *
 * @return                          The absolute humidity in mg/m^3
 */
uint32_t sensirion_calc_absolute_humidity(int32_t temperature_milli_celsius,
                                          int32_t humidity_milli_percent);


#endif /* HUMIDITY_CONVERSION_H */
