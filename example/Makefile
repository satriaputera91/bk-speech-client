################################################################################
# Automatically-generated file. Do not edit!
################################################################################

LIBS := -lbkspeech -lcurl -ljson-c -lssl -luuid -lcrypto
RM := rm -rf
C_SRCS += \
	onwav-sync-upload.c 

OBJS += \
	onwav-sync-upload.o 

C_DEPS += \
	onwav-sync-upload.d 

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# All Target
all: onwav-sync-upload

# Each subdirectory must supply rules for building sources it contributes
%.o: %.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

onwav-sync-upload: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -o "onwav-sync-upload" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) onwav-sync-upload
	-@echo ' '

.PHONY: all clean dependents

-include ../makefile.targets
