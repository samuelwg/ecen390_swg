################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/laserTag/detector.c \
../src/laserTag/isr.c \
../src/laserTag/main.c \
../src/laserTag/transmitter.c 

OBJS += \
./src/laserTag/detector.o \
./src/laserTag/isr.o \
./src/laserTag/main.o \
./src/laserTag/transmitter.o 

C_DEPS += \
./src/laserTag/detector.d \
./src/laserTag/isr.d \
./src/laserTag/main.d \
./src/laserTag/transmitter.d 


# Each subdirectory must supply rules for building sources it contributes
src/laserTag/%.o: ../src/laserTag/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM g++ compiler'
	arm-xilinx-eabi-g++ -Wall -O3 -finline-functions -g3 -I"C:\Users\hutch\ECEN390Winter2015\ECEN390_Student_SW_0.2\Consolidated_330_SW" -c -fmessage-length=0 -MT"$@" -I../../HW3_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


