################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../Core/Src/multiengine/hal/Encoder.cc 

CPP_SRCS += \
../Core/Src/multiengine/hal/Display.cpp \
../Core/Src/multiengine/hal/Memory.cpp \
../Core/Src/multiengine/hal/MultiEngineHAL.cpp 

CC_DEPS += \
./Core/Src/multiengine/hal/Encoder.d 

OBJS += \
./Core/Src/multiengine/hal/Display.o \
./Core/Src/multiengine/hal/Encoder.o \
./Core/Src/multiengine/hal/Memory.o \
./Core/Src/multiengine/hal/MultiEngineHAL.o 

CPP_DEPS += \
./Core/Src/multiengine/hal/Display.d \
./Core/Src/multiengine/hal/Memory.d \
./Core/Src/multiengine/hal/MultiEngineHAL.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/multiengine/hal/Display.o: ../Core/Src/multiengine/hal/Display.cpp
	arm-none-eabi-g++ "$<" -mcpu=cortex-m3 -std=gnu++14 -g3 -DUSE_HAL_DRIVER -DSTM32F103xB -DDEBUG -c -I../Core/Inc -I../Core/Inc/multiengine -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"Core/Src/multiengine/hal/Display.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/Src/multiengine/hal/Encoder.o: ../Core/Src/multiengine/hal/Encoder.cc
	arm-none-eabi-g++ "$<" -mcpu=cortex-m3 -std=gnu++14 -g3 -DUSE_HAL_DRIVER -DSTM32F103xB -DDEBUG -c -I../Core/Inc -I../Core/Inc/multiengine -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"Core/Src/multiengine/hal/Encoder.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/Src/multiengine/hal/Memory.o: ../Core/Src/multiengine/hal/Memory.cpp
	arm-none-eabi-g++ "$<" -mcpu=cortex-m3 -std=gnu++14 -g3 -DUSE_HAL_DRIVER -DSTM32F103xB -DDEBUG -c -I../Core/Inc -I../Core/Inc/multiengine -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"Core/Src/multiengine/hal/Memory.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/Src/multiengine/hal/MultiEngineHAL.o: ../Core/Src/multiengine/hal/MultiEngineHAL.cpp
	arm-none-eabi-g++ "$<" -mcpu=cortex-m3 -std=gnu++14 -g3 -DUSE_HAL_DRIVER -DSTM32F103xB -DDEBUG -c -I../Core/Inc -I../Core/Inc/multiengine -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"Core/Src/multiengine/hal/MultiEngineHAL.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

