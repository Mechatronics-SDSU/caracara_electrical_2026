################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/button_state.c \
../Core/Src/depth_sensor.c \
../Core/Src/dropper.c \
../Core/Src/gpio.c \
../Core/Src/grabber.c \
../Core/Src/main.c \
../Core/Src/motor_control.c \
../Core/Src/power_manager.c \
../Core/Src/rgb_led.c \
../Core/Src/safety_manager.c \
../Core/Src/stm32g4xx_hal_msp.c \
../Core/Src/stm32g4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_state.c \
../Core/Src/system_stm32g4xx.c \
../Core/Src/torpedo.c \
../Core/Src/usb_manager.c 

OBJS += \
./Core/Src/button_state.o \
./Core/Src/depth_sensor.o \
./Core/Src/dropper.o \
./Core/Src/gpio.o \
./Core/Src/grabber.o \
./Core/Src/main.o \
./Core/Src/motor_control.o \
./Core/Src/power_manager.o \
./Core/Src/rgb_led.o \
./Core/Src/safety_manager.o \
./Core/Src/stm32g4xx_hal_msp.o \
./Core/Src/stm32g4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_state.o \
./Core/Src/system_stm32g4xx.o \
./Core/Src/torpedo.o \
./Core/Src/usb_manager.o 

C_DEPS += \
./Core/Src/button_state.d \
./Core/Src/depth_sensor.d \
./Core/Src/dropper.d \
./Core/Src/gpio.d \
./Core/Src/grabber.d \
./Core/Src/main.d \
./Core/Src/motor_control.d \
./Core/Src/power_manager.d \
./Core/Src/rgb_led.d \
./Core/Src/safety_manager.d \
./Core/Src/stm32g4xx_hal_msp.d \
./Core/Src/stm32g4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_state.d \
./Core/Src/system_stm32g4xx.d \
./Core/Src/torpedo.d \
./Core/Src/usb_manager.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../USB_Device/App -I../USB_Device/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/button_state.cyclo ./Core/Src/button_state.d ./Core/Src/button_state.o ./Core/Src/button_state.su ./Core/Src/depth_sensor.cyclo ./Core/Src/depth_sensor.d ./Core/Src/depth_sensor.o ./Core/Src/depth_sensor.su ./Core/Src/dropper.cyclo ./Core/Src/dropper.d ./Core/Src/dropper.o ./Core/Src/dropper.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/grabber.cyclo ./Core/Src/grabber.d ./Core/Src/grabber.o ./Core/Src/grabber.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/motor_control.cyclo ./Core/Src/motor_control.d ./Core/Src/motor_control.o ./Core/Src/motor_control.su ./Core/Src/power_manager.cyclo ./Core/Src/power_manager.d ./Core/Src/power_manager.o ./Core/Src/power_manager.su ./Core/Src/rgb_led.cyclo ./Core/Src/rgb_led.d ./Core/Src/rgb_led.o ./Core/Src/rgb_led.su ./Core/Src/safety_manager.cyclo ./Core/Src/safety_manager.d ./Core/Src/safety_manager.o ./Core/Src/safety_manager.su ./Core/Src/stm32g4xx_hal_msp.cyclo ./Core/Src/stm32g4xx_hal_msp.d ./Core/Src/stm32g4xx_hal_msp.o ./Core/Src/stm32g4xx_hal_msp.su ./Core/Src/stm32g4xx_it.cyclo ./Core/Src/stm32g4xx_it.d ./Core/Src/stm32g4xx_it.o ./Core/Src/stm32g4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_state.cyclo ./Core/Src/system_state.d ./Core/Src/system_state.o ./Core/Src/system_state.su ./Core/Src/system_stm32g4xx.cyclo ./Core/Src/system_stm32g4xx.d ./Core/Src/system_stm32g4xx.o ./Core/Src/system_stm32g4xx.su ./Core/Src/torpedo.cyclo ./Core/Src/torpedo.d ./Core/Src/torpedo.o ./Core/Src/torpedo.su ./Core/Src/usb_manager.cyclo ./Core/Src/usb_manager.d ./Core/Src/usb_manager.o ./Core/Src/usb_manager.su

.PHONY: clean-Core-2f-Src

