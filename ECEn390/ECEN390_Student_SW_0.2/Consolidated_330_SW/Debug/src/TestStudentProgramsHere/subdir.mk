################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/TestStudentProgramsHere/foo.c 

OBJS += \
./src/TestStudentProgramsHere/foo.o 

C_DEPS += \
./src/TestStudentProgramsHere/foo.d 


# Each subdirectory must supply rules for building sources it contributes
src/TestStudentProgramsHere/%.o: ../src/TestStudentProgramsHere/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM g++ compiler'
	arm-xilinx-eabi-g++ -Wall -O2 -finline-functions -g3 -I"C:\Users\hutch\ZYBO\HWTest3_SW_Env\Consolidated_330_SW" -c -fmessage-length=0 -I../../HW3_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


