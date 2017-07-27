################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/hilos/subdir.mk
-include src/funcionesAuxiliares/subdir.mk
-include src/funcionHash/subdir.mk
-include src/cache/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Memoria

dependents:
	-cd /home/utnso/so/tp-2017-1c-Kaprobo/Commons_Kaprobo/Debug && $(MAKE) all

# Tool invocations
Memoria: $(OBJS) $(USER_OBJS) /home/utnso/so/tp-2017-1c-Kaprobo/Commons_Kaprobo/Debug/libCommons_Kaprobo.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "Memoria" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) Memoria
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/so/tp-2017-1c-Kaprobo/Commons_Kaprobo/Debug/libCommons_Kaprobo.so:

-include ../makefile.targets
