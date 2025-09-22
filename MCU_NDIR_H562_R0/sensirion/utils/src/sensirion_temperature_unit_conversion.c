#include "sensirion_temperature_unit_conversion.h"

int32_t sensirion_celsius_to_fahrenheit(int32_t temperature_milli_celsius) {
    /* Conversion equivalent to: temperature_milli_celsius * 9.0/5.0 + 32000
     * Fixed Point: 9.0/5.0 * 2^12 = 7372.8
     * Using int32_t sized two's complement for negatives */
    return ((temperature_milli_celsius * 7373) >> 12) + 32000;
}

int32_t sensirion_fahrenheit_to_celsius(int32_t temperature_milli_fahrenheit) {
    /* Conversion equivalent to:
     * (temperature_milli_fahrenheit - 32000) * 5.0/9.0
     * Fixed Point: 5.0/9.0 * 2^10 = 568.9
     * Using int32_t sized two's complement for negatives */
    return ((temperature_milli_fahrenheit - 32000) * 569) >> 10;
}
