################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USER/APB/apb_regbank_test.c 

OBJS += \
./USER/APB/apb_regbank_test.o 

C_DEPS += \
./USER/APB/apb_regbank_test.d 


# Each subdirectory must supply rules for building sources it contributes
USER/APB/%.o: ../USER/APB/%.c USER/APB/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O3 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -I"C:\Users\30698\tang-nano-4k-apb\MCU\CORE" -I"C:\Users\30698\tang-nano-4k-apb\MCU\PERIPHERAL\Includes" -I"C:\Users\30698\tang-nano-4k-apb\MCU\SYSTEM" -I"C:\Users\30698\tang-nano-4k-apb\MCU\USER" -I"C:\Users\30698\tang-nano-4k-apb\MCU\FreeRTOS\include" -I"C:\Users\30698\tang-nano-4k-apb\MCU\FreeRTOS\portable\GNU\ARM_CM3" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


