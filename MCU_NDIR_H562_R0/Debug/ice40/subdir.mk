################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ice40/ice40.c \
../ice40/ice40_programmer.c 

OBJS += \
./ice40/ice40.o \
./ice40/ice40_programmer.o 

C_DEPS += \
./ice40/ice40.d \
./ice40/ice40_programmer.d 


# Each subdirectory must supply rules for building sources it contributes
ice40/%.o ice40/%.su ice40/%.cyclo: ../ice40/%.c ice40/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H562xx -c -I../Core/Inc -I../bme280 -I../sensirion/inc -I../sensirion/utils/inc -I../bms -I../ads7142 -I../tmp117 -I../ilps28qsw -I../USB/Class/CompositeBuilder/Inc -I../ads131m04 -I../ice40 -I../task -I../KedOS/core -I../Drivers/CMSIS/Include -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../USB/Core/Inc -I../USB/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-ice40

clean-ice40:
	-$(RM) ./ice40/ice40.cyclo ./ice40/ice40.d ./ice40/ice40.o ./ice40/ice40.su ./ice40/ice40_programmer.cyclo ./ice40/ice40_programmer.d ./ice40/ice40_programmer.o ./ice40/ice40_programmer.su

.PHONY: clean-ice40

