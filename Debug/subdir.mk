################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../MsgSched_Agent.cpp \
../MsgSched_Peer.cpp \
../MsgSched_Tick.cpp \
../Tcp_Client.cpp \
../main_sample.cpp 

OBJS += \
./MsgSched_Agent.o \
./MsgSched_Peer.o \
./MsgSched_Tick.o \
./Tcp_Client.o \
./main_sample.o 

CPP_DEPS += \
./MsgSched_Agent.d \
./MsgSched_Peer.d \
./MsgSched_Tick.d \
./Tcp_Client.d \
./main_sample.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


