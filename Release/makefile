################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := B:/ti/ccs1210new/ccs/tools/compiler/ti-cgt-armllvm_2.1.2.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./IPC_RPC_Comm.o" \
"./app_cpswconfighandler.o" \
"./app_main.o" \
"./app_tcpserver.o" \
"./syscfg/ti_dpl_config.o" \
"./syscfg/ti_drivers_config.o" \
"./syscfg/ti_drivers_open_close.o" \
"./syscfg/ti_pinmux_config.o" \
"./syscfg/ti_power_clock_config.o" \
"./syscfg/ti_board_config.o" \
"./syscfg/ti_board_open_close.o" \
"./syscfg/ti_enet_config.o" \
"./syscfg/ti_enet_open_close.o" \
"./syscfg/ti_enet_soc.o" \
"./syscfg/ti_enet_lwipif.o" \
"./main.o" \
"../linker.cmd" \
$(GEN_CMDS__FLAG) \
-Wl,-lfreertos.am263x.r5f.ti-arm-clang.release.lib \
-Wl,-ldrivers.am263x.r5f.ti-arm-clang.release.lib \
-Wl,-lenet-cpsw.am263x.r5f.ti-arm-clang.release.lib \
-Wl,-llwipif-cpsw-freertos.am263x.r5f.ti-arm-clang.release.lib \
-Wl,-llwip-freertos.am263x.r5f.ti-arm-clang.release.lib \
-Wl,-lboard.am263x.r5f.ti-arm-clang.release.lib \
-Wl,-llwip-contrib-freertos.am263x.r5f.ti-arm-clang.release.lib \
-Wl,-llibc.a \
-Wl,-llibsysbm.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
enet_cpsw_tcpserver_am263x-lp_r5fss0-1_freertos_ti-arm-clang.out \

EXE_OUTPUTS__QUOTED += \
"enet_cpsw_tcpserver_am263x-lp_r5fss0-1_freertos_ti-arm-clang.out" \

BIN_OUTPUTS += \
enet_cpsw_tcpserver_am263x-lp_r5fss0-1_freertos_ti-arm-clang.hex \

BIN_OUTPUTS__QUOTED += \
"enet_cpsw_tcpserver_am263x-lp_r5fss0-1_freertos_ti-arm-clang.hex" \


# All Target
all: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "enet_cpsw_tcpserver_am263x-lp_r5fss0-1_freertos_ti-arm-clang.out"

# Tool invocations
enet_cpsw_tcpserver_am263x-lp_r5fss0-1_freertos_ti-arm-clang.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: Arm Linker'
	"B:/ti/ccs1210new/ccs/tools/compiler/ti-cgt-armllvm_2.1.2.LTS/bin/tiarmclang.exe" -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 -mlittle-endian -mthumb -Os -DSOC_AM263X -g -Wall -Wno-gnu-variable-sized-type-not-at-end -Wno-unused-function -Wl,-m"enet_cpsw_tcpserver.Release.map" -Wl,-i"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/kernel/freertos/lib" -Wl,-i"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/drivers/lib" -Wl,-i"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/board/lib" -Wl,-i"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/lib" -Wl,-i"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/lwip/lib" -Wl,-i"B:/ti/ccs1210new/ccs/tools/compiler/ti-cgt-armllvm_2.1.2.LTS/lib" -Wl,-i"C:/Users/Robi/workspace_v12/enet_cpsw_tcpserver_am263x-lp_r5fss0-1_freertos_ti-arm-clang/Release/syscfg" -Wl,--reread_libs -Wl,--diag_suppress=10063 -Wl,--diag_wrap=off -Wl,--display_error_number -Wl,--warn_sections -Wl,--xml_link_info="enet_cpsw_tcpserver_am263x-lp_r5fss0-1_freertos_ti-arm-clang_linkInfo.xml" -Wl,--ram_model -Wl,--zero_init=on -o "enet_cpsw_tcpserver_am263x-lp_r5fss0-1_freertos_ti-arm-clang.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '
	@$(MAKE) --no-print-directory post-build

enet_cpsw_tcpserver_am263x-lp_r5fss0-1_freertos_ti-arm-clang.hex: $(EXE_OUTPUTS)
	@echo 'Building secondary target: "$@"'
	@echo 'Invoking: Arm Hex Utility'
	"B:/ti/ccs1210new/ccs/tools/compiler/ti-cgt-armllvm_2.1.2.LTS/bin/tiarmhex.exe" --diag_wrap=off -o "enet_cpsw_tcpserver_am263x-lp_r5fss0-1_freertos_ti-arm-clang.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building secondary target: "$@"'
	@echo ' '
	@$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(GEN_MISC_FILES__QUOTED)$(BIN_OUTPUTS__QUOTED)$(GEN_FILES__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "IPC_RPC_Comm.o" "app_cpswconfighandler.o" "app_main.o" "app_tcpserver.o" "syscfg\ti_dpl_config.o" "syscfg\ti_drivers_config.o" "syscfg\ti_drivers_open_close.o" "syscfg\ti_pinmux_config.o" "syscfg\ti_power_clock_config.o" "syscfg\ti_board_config.o" "syscfg\ti_board_open_close.o" "syscfg\ti_enet_config.o" "syscfg\ti_enet_open_close.o" "syscfg\ti_enet_soc.o" "syscfg\ti_enet_lwipif.o" "main.o" 
	-$(RM) "IPC_RPC_Comm.d" "app_cpswconfighandler.d" "app_main.d" "app_tcpserver.d" "syscfg\ti_dpl_config.d" "syscfg\ti_drivers_config.d" "syscfg\ti_drivers_open_close.d" "syscfg\ti_pinmux_config.d" "syscfg\ti_power_clock_config.d" "syscfg\ti_board_config.d" "syscfg\ti_board_open_close.d" "syscfg\ti_enet_config.d" "syscfg\ti_enet_open_close.d" "syscfg\ti_enet_soc.d" "syscfg\ti_enet_lwipif.d" "main.d" 
	-$(RMDIR) $(GEN_MISC_DIRS__QUOTED)
	-@echo 'Finished clean'
	-@echo ' '

post-build:
	-$(MAKE) -C C:\Users\Robi\workspace_v12\enet_cpsw_tcpserver_am263x-lp_r5fss0-1_freertos_ti-arm-clang -f makefile_ccs_bootimage_gen OUTNAME=enet_cpsw_tcpserver_am263x-lp_r5fss0-1_freertos_ti-arm-clang PROFILE=Release MCU_PLUS_SDK_PATH=C:/ti/mcu_plus_sdk_am263x_08_06_00_34 CG_TOOL_ROOT=B:/ti/ccs1210new/ccs/tools/compiler/ti-cgt-armllvm_2.1.2.LTS CCS_INSTALL_DIR=B:\ti\ccs1210new\ccs CCS_IDE_MODE=desktop
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

