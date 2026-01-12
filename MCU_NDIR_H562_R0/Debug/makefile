################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include task/subdir.mk
-include sensirion/utils/src/subdir.mk
-include sensirion/src/subdir.mk
-include ice40/subdir.mk
-include bms/subdir.mk
-include bme280/subdir.mk
-include USB/Core/Src/subdir.mk
-include USB/Class/CompositeBuilder/Src/subdir.mk
-include USB/Class/CDC/Src/subdir.mk
-include KedOS/core/subdir.mk
-include Drivers/STM32H5xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := MCU_NDIR_H562_R0
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
MCU_NDIR_H562_R0.elf \

MAP_FILES += \
MCU_NDIR_H562_R0.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
MCU_NDIR_H562_R0.list \

OBJCOPY_BIN += \
MCU_NDIR_H562_R0.bin \


# All Target
all:
	+@$(MAKE) --no-print-directory pre-build && $(MAKE) --no-print-directory main-build

# Main-build Target
main-build: MCU_NDIR_H562_R0.elf secondary-outputs

# Tool invocations
MCU_NDIR_H562_R0.elf MCU_NDIR_H562_R0.map: $(OBJS) $(USER_OBJS) C:\Lavoro\QINOVA\STM32WS\MCU_NDIR_H562_R0\STM32H562RGTX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "MCU_NDIR_H562_R0.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m33 -T"C:\Lavoro\QINOVA\STM32WS\MCU_NDIR_H562_R0\STM32H562RGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="MCU_NDIR_H562_R0.map" -Wl,--gc-sections -static -L../Middlewares/ST/ARM/DSP/Lib --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -u _printf_float
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

MCU_NDIR_H562_R0.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "MCU_NDIR_H562_R0.list"
	@echo 'Finished building: $@'
	@echo ' '

MCU_NDIR_H562_R0.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "MCU_NDIR_H562_R0.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) MCU_NDIR_H562_R0.bin MCU_NDIR_H562_R0.elf MCU_NDIR_H562_R0.list MCU_NDIR_H562_R0.map default.size.stdout
	-@echo ' '

pre-build:
	cd ../bin; cp ICE44_CAMERA_CONTROL_impl_1.bin ICE40_CAMERA_CONTROL.bin; xxd.exe -i ICE40_CAMERA_CONTROL.bin ../ice40/ice40.c
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified pre-build

-include ../makefile.targets
