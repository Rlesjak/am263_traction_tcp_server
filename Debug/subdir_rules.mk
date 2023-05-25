################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"B:/ti/ccs1210new/ccs/tools/compiler/ti-cgt-armllvm_2.1.2.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 -mlittle-endian -mthumb -I"C:/Users/Robi/workspace_v12/TractionDemo_am263x-cc_r5fss0-0_nortos_ti-arm-clang/IpcComm" -I"B:/ti/ccs1210new/ccs/tools/compiler/ti-cgt-armllvm_2.1.2.LTS/include/c" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/kernel/freertos/FreeRTOS-Kernel/include" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/kernel/freertos/portable/TI_ARM_CLANG/ARM_CR5F" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/kernel/freertos/config/am263x/r5f" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/utils/include" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/core" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/core/include" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/core/include/phy" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/core/include/core" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/hw_include" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/soc/am263x" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/hw_include/mdio/V4" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/lwip/lwip-stack/src/include" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/lwip/lwip-port/include" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/lwip/lwip-port/freertos/include" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/core/lwipif/inc" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/lwip/lwip-contrib" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/lwip/lwip-config/am263x" -DSOC_AM263X -D_DEBUG_=1 -g -Wall -Wno-gnu-variable-sized-type-not-at-end -Wno-unused-function -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)" -I"C:/Users/Robi/workspace_v12/enet_cpsw_tcpserver_am263x-lp_r5fss0-1_freertos_ti-arm-clang/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-570043905: ../example.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"B:/ti/ccs1210/ccs/utils/sysconfig_1.16.1/sysconfig_cli.bat" -s "C:/ti/mcu_plus_sdk_am263x_08_06_00_34/.metadata/product.json" -s "C:/ti/mcu_plus_sdk_am263x_08_06_00_34/.metadata/product.json" --script "C:/Users/Robi/workspace_v12/TractionDemo_am263x-cc_r5fss0-0_nortos_ti-arm-clang/trinv.syscfg" --context "r5fss0-0" --script "C:/Users/Robi/workspace_v12/enet_cpsw_tcpserver_am263x-lp_r5fss0-1_freertos_ti-arm-clang/example.syscfg" --context "r5fss0-1" -o "syscfg" --part AM263x --package ZCZ --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/ti_dpl_config.c: build-570043905 ../example.syscfg
syscfg/ti_dpl_config.h: build-570043905
syscfg/ti_drivers_config.c: build-570043905
syscfg/ti_drivers_config.h: build-570043905
syscfg/ti_drivers_open_close.c: build-570043905
syscfg/ti_drivers_open_close.h: build-570043905
syscfg/ti_pinmux_config.c: build-570043905
syscfg/ti_power_clock_config.c: build-570043905
syscfg/ti_board_config.c: build-570043905
syscfg/ti_board_config.h: build-570043905
syscfg/ti_board_open_close.c: build-570043905
syscfg/ti_board_open_close.h: build-570043905
syscfg/ti_enet_config.c: build-570043905
syscfg/ti_enet_config.h: build-570043905
syscfg/ti_enet_open_close.c: build-570043905
syscfg/ti_enet_open_close.h: build-570043905
syscfg/ti_enet_soc.c: build-570043905
syscfg/ti_enet_lwipif.c: build-570043905
syscfg/ti_enet_lwipif.h: build-570043905
syscfg/: build-570043905

syscfg/%.o: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"B:/ti/ccs1210new/ccs/tools/compiler/ti-cgt-armllvm_2.1.2.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 -mlittle-endian -mthumb -I"C:/Users/Robi/workspace_v12/TractionDemo_am263x-cc_r5fss0-0_nortos_ti-arm-clang/IpcComm" -I"B:/ti/ccs1210new/ccs/tools/compiler/ti-cgt-armllvm_2.1.2.LTS/include/c" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/kernel/freertos/FreeRTOS-Kernel/include" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/kernel/freertos/portable/TI_ARM_CLANG/ARM_CR5F" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/kernel/freertos/config/am263x/r5f" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/utils/include" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/core" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/core/include" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/core/include/phy" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/core/include/core" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/hw_include" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/soc/am263x" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/hw_include/mdio/V4" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/lwip/lwip-stack/src/include" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/lwip/lwip-port/include" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/lwip/lwip-port/freertos/include" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/enet/core/lwipif/inc" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/lwip/lwip-contrib" -I"C:/ti/mcu_plus_sdk_am263x_08_06_00_34/source/networking/lwip/lwip-config/am263x" -DSOC_AM263X -D_DEBUG_=1 -g -Wall -Wno-gnu-variable-sized-type-not-at-end -Wno-unused-function -MMD -MP -MF"syscfg/$(basename $(<F)).d_raw" -MT"$(@)" -I"C:/Users/Robi/workspace_v12/enet_cpsw_tcpserver_am263x-lp_r5fss0-1_freertos_ti-arm-clang/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


