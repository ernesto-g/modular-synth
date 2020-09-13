################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../Core/Src/braids/drivers/adc.cc 

CC_DEPS += \
./Core/Src/braids/drivers/adc.d 

OBJS += \
./Core/Src/braids/drivers/adc.o 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/braids/drivers/adc.o: ../Core/Src/braids/drivers/adc.cc
	arm-none-eabi-g++ "$<" -mcpu=cortex-m3 -std=gnu++14 -g3 -DUSE_HAL_DRIVER -DSTM32F103xB -DDEBUG -c -I../Core/Inc -I../Core/Inc/multiengine -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"Core/Src/braids/drivers/adc.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

