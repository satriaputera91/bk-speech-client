################################################################################
# Automatically-generated file. Do not edit!
################################################################################

RM := rm -rf
PREFIX = /usr/local

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

C_SRCS += \
		  bk_http_url_connection.c \
		  bk_queue_result.c \
		  bk_speech_client.c \
		  bk_utils.c

OBJS += \
		bk_http_url_connection.o \
		bk_queue_result.o \
		bk_speech_client.o \
		bk_utils.o 

C_DEPS += \
		  bk_http_url_connection.d \
		  bk_queue_result.d \
		  bk_speech_client.d \
		  bk_utils.d 


# All Target
all: libbkspeech.so libbkspeech.a


# Tool invocations
libbkspeech.so: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -shared -o "libbkspeech.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

libbkspeech.a: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC Archiver'
	ar -r  "libbkspeech.a" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

%.o: %.c
		@echo 'Building file: $<'
		@echo 'Invoking: GCC C Compiler'
		gcc -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
		@echo 'Finished building: $<'
		@echo ' '

# Other Targets
clean:
	-$(RM) $(LIBRARIES)$(OBJS)$(C_DEPS) libbkspeech.so libbkspeech.a
	-@echo ' '

install: 
	mkdir -p $(DESTDIR)$(PREFIX)/include/bkspeech
	cp *.h $(DESTDIR)$(PREFIX)/include/bkspeech
	cp libbkspeech.so libbkspeech.a $(DESTDIR)$(PREFIX)/lib
	ldconfig

.PHONY: all clean dependents install

-include ../makefile.targets
