################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../shared/connect.c \
../shared/console.c \
../shared/protocolo.c \
../shared/socket.c \
../shared/utils.c 

OBJS += \
./shared/connect.o \
./shared/console.o \
./shared/protocolo.o \
./shared/socket.o \
./shared/utils.o 

C_DEPS += \
./shared/connect.d \
./shared/console.d \
./shared/protocolo.d \
./shared/socket.d \
./shared/utils.d 


# Each subdirectory must supply rules for building sources it contributes
shared/%.o: ../shared/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


