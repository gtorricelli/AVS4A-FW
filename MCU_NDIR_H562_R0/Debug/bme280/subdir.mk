################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../bme280/bme280.c 

OBJS += \
./bme280/bme280.o 

C_DEPS += \
./bme280/bme280.d 


# Each subdirectory must supply rules for building sources it contributes
bme280/%.o bme280/%.su bme280/%.cyclo: ../bme280/%.c bme280/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H562xx -c -I../Core/Inc -I../bme280 -I../sensirion/inc -I../sensirion/utils/inc -I../bms -I../ads7142 -I../tmp117 -I../ilps28qsw -I../USB/Class/CompositeBuilder/Inc -I../ads131m04 -I../ice40 -I../task -I../KedOS/core -I../Drivers/CMSIS/Include -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../USB/Core/Inc -I../USB/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-bme280

clean-bme280:
	-$(RM) ./bme280/bme280.cyclo ./bme280/bme280.d ./bme280/bme280.o ./bme280/bme280.su

.PHONY: clean-bme280

