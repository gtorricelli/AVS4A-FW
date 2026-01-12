################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../KedOS/core/kedOS.c \
../KedOS/core/packing.c \
../KedOS/core/shell.c \
../KedOS/core/utility.c 

OBJS += \
./KedOS/core/kedOS.o \
./KedOS/core/packing.o \
./KedOS/core/shell.o \
./KedOS/core/utility.o 

C_DEPS += \
./KedOS/core/kedOS.d \
./KedOS/core/packing.d \
./KedOS/core/shell.d \
./KedOS/core/utility.d 


# Each subdirectory must supply rules for building sources it contributes
KedOS/core/%.o KedOS/core/%.su KedOS/core/%.cyclo: ../KedOS/core/%.c KedOS/core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H562xx -c -I../Core/Inc -I../bme280 -I../sensirion/inc -I../sensirion/utils/inc -I../bms -I../ads7142 -I../tmp117 -I../ilps28qsw -I../USB/Class/CompositeBuilder/Inc -I../ads131m04 -I../ice40 -I../task -I../KedOS/core -I../Drivers/CMSIS/Include -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../USB/Core/Inc -I../USB/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-KedOS-2f-core

clean-KedOS-2f-core:
	-$(RM) ./KedOS/core/kedOS.cyclo ./KedOS/core/kedOS.d ./KedOS/core/kedOS.o ./KedOS/core/kedOS.su ./KedOS/core/packing.cyclo ./KedOS/core/packing.d ./KedOS/core/packing.o ./KedOS/core/packing.su ./KedOS/core/shell.cyclo ./KedOS/core/shell.d ./KedOS/core/shell.o ./KedOS/core/shell.su ./KedOS/core/utility.cyclo ./KedOS/core/utility.d ./KedOS/core/utility.o ./KedOS/core/utility.su

.PHONY: clean-KedOS-2f-core

