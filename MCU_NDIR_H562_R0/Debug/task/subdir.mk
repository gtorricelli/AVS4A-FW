################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../task/blink_task.c \
../task/bme_task.c \
../task/hailo_task.c \
../task/pir_task.c \
../task/rtc_task.c \
../task/sensors_task.c \
../task/serial_com_task.c \
../task/shell_task.c 

OBJS += \
./task/blink_task.o \
./task/bme_task.o \
./task/hailo_task.o \
./task/pir_task.o \
./task/rtc_task.o \
./task/sensors_task.o \
./task/serial_com_task.o \
./task/shell_task.o 

C_DEPS += \
./task/blink_task.d \
./task/bme_task.d \
./task/hailo_task.d \
./task/pir_task.d \
./task/rtc_task.d \
./task/sensors_task.d \
./task/serial_com_task.d \
./task/shell_task.d 


# Each subdirectory must supply rules for building sources it contributes
task/%.o task/%.su task/%.cyclo: ../task/%.c task/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H562xx -c -I../Core/Inc -I../bme280 -I../sensirion/inc -I../sensirion/utils/inc -I../bms -I../ads7142 -I../tmp117 -I../ilps28qsw -I../USB/Class/CompositeBuilder/Inc -I../ads131m04 -I../ice40 -I../task -I../KedOS/core -I../Drivers/CMSIS/Include -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../USB/Core/Inc -I../USB/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-task

clean-task:
	-$(RM) ./task/blink_task.cyclo ./task/blink_task.d ./task/blink_task.o ./task/blink_task.su ./task/bme_task.cyclo ./task/bme_task.d ./task/bme_task.o ./task/bme_task.su ./task/hailo_task.cyclo ./task/hailo_task.d ./task/hailo_task.o ./task/hailo_task.su ./task/pir_task.cyclo ./task/pir_task.d ./task/pir_task.o ./task/pir_task.su ./task/rtc_task.cyclo ./task/rtc_task.d ./task/rtc_task.o ./task/rtc_task.su ./task/sensors_task.cyclo ./task/sensors_task.d ./task/sensors_task.o ./task/sensors_task.su ./task/serial_com_task.cyclo ./task/serial_com_task.d ./task/serial_com_task.o ./task/serial_com_task.su ./task/shell_task.cyclo ./task/shell_task.d ./task/shell_task.o ./task/shell_task.su

.PHONY: clean-task

