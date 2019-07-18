################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/config/config_LFS.c 

OBJS += \
./src/config/config_LFS.o 

C_DEPS += \
./src/config/config_LFS.d 


# Each subdirectory must supply rules for building sources it contributes
src/config/%.o: ../src/config/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I../../shared -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


