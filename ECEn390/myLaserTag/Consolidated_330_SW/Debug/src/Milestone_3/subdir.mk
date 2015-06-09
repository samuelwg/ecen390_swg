################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Milestone_3/detector.c \
../src/Milestone_3/filter.c \
../src/Milestone_3/histogram.c \
../src/Milestone_3/hitLedTimer.c \
../src/Milestone_3/isr.c \
../src/Milestone_3/lockoutTimer.c \
../src/Milestone_3/main.c \
../src/Milestone_3/playTone.c \
../src/Milestone_3/sound.c \
../src/Milestone_3/transmitter.c \
../src/Milestone_3/trigger.c 

OBJS += \
./src/Milestone_3/detector.o \
./src/Milestone_3/filter.o \
./src/Milestone_3/histogram.o \
./src/Milestone_3/hitLedTimer.o \
./src/Milestone_3/isr.o \
./src/Milestone_3/lockoutTimer.o \
./src/Milestone_3/main.o \
./src/Milestone_3/playTone.o \
./src/Milestone_3/sound.o \
./src/Milestone_3/transmitter.o \
./src/Milestone_3/trigger.o 

C_DEPS += \
./src/Milestone_3/detector.d \
./src/Milestone_3/filter.d \
./src/Milestone_3/histogram.d \
./src/Milestone_3/hitLedTimer.d \
./src/Milestone_3/isr.d \
./src/Milestone_3/lockoutTimer.d \
./src/Milestone_3/main.d \
./src/Milestone_3/playTone.d \
./src/Milestone_3/sound.d \
./src/Milestone_3/transmitter.d \
./src/Milestone_3/trigger.d 


# Each subdirectory must supply rules for building sources it contributes
src/Milestone_3/%.o: ../src/Milestone_3/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM g++ compiler'
	arm-xilinx-eabi-g++ -Wall -O3 -finline-functions -g3 -I"F:\ECEn390\myLaserTag\Consolidated_330_SW" -c -fmessage-length=0 -MT"$@" -I../../HW3_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


