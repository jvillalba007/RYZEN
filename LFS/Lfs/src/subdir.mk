################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/API.c \
../src/LFS.c \
../src/compactator.c \
../src/dump.c \
../src/filesystem.c \
../src/memtable.c 

OBJS += \
./src/API.o \
./src/LFS.o \
./src/compactator.o \
./src/dump.o \
./src/filesystem.o \
./src/memtable.o 

C_DEPS += \
./src/API.d \
./src/LFS.d \
./src/compactator.d \
./src/dump.d \
./src/filesystem.d \
./src/memtable.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I../../shared -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


