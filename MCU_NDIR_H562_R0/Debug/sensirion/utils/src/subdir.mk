################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../sensirion/utils/src/sensirion_humidity_conversion.c \
../sensirion/utils/src/sensirion_temperature_unit_conversion.c 

OBJS += \
./sensirion/utils/src/sensirion_humidity_conversion.o \
./sensirion/utils/src/sensirion_temperature_unit_conversion.o 

C_DEPS += \
./sensirion/utils/src/sensirion_humidity_conversion.d \
./sensirion/utils/src/sensirion_temperature_unit_conversion.d 


# Each subdirectory must supply rules for building sources it contributes
sensirion/utils/src/%.o sensirion/utils/src/%.su sensirion/utils/src/%.cyclo: ../sensirion/utils/src/%.c sensirion/utils/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H562xx -c -I../Core/Inc -I../bme280 -I../sensirion/inc -I../sensirion/utils/inc -I../bms -I../ads7142 -I../tmp117 -I../ilps28qsw -I../USB/Class/CompositeBuilder/Inc -I../ads131m04 -I../ice40 -I../task -I../KedOS/core -I../Drivers/CMSIS/Include -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../USB/Core/Inc -I../USB/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-sensirion-2f-utils-2f-src

clean-sensirion-2f-utils-2f-src:
	-$(RM) ./sensirion/utils/src/sensirion_humidity_conversion.cyclo ./sensirion/utils/src/sensirion_humidity_conversion.d ./sensirion/utils/src/sensirion_humidity_conversion.o ./sensirion/utils/src/sensirion_humidity_conversion.su ./sensirion/utils/src/sensirion_temperature_unit_conversion.cyclo ./sensirion/utils/src/sensirion_temperature_unit_conversion.d ./sensirion/utils/src/sensirion_temperature_unit_conversion.o ./sensirion/utils/src/sensirion_temperature_unit_conversion.su

.PHONY: clean-sensirion-2f-utils-2f-src

