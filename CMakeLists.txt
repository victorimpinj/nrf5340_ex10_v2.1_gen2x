#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)
set(VERSION 1.2.24)

# Use C 2011
set (CMAKE_C_STANDARD 11)

# No warnings
add_definitions(-w)

# Set OS type for Zephyr
add_definitions(-DEX10_OSAL_TYPE=EX10_OS_TYPE_BARE_METAL)
set(CMAKE_BUILD_TYPE "Debug")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rain_cloud
    VERSION ${VERSION}
    DESCRIPTION "Impinj Zephyr RTOS Reader for nRF5340 Dev Kit"
    LANGUAGES C
)

set(BOARD_PATH ${PROJECT_SOURCE_DIR}/src/board/zephyr_rtos)
set(EX10 ${PROJECT_SOURCE_DIR}/src/src/ex10)
set(EX10_GEN2X ${PROJECT_SOURCE_DIR}/src/src_gen2x/ex10)

target_sources(
    app
    PRIVATE

    src/main.c

    src/rain/spitest.c
    src/rain/getboardversion.c
    src/rain/simple_ramping_example.c
    #src/rain/continuous_inventory.c
    src/rain/gen2x_continuous_inventory.c

    ${BOARD_PATH}/board_spec.c
    ${BOARD_PATH}/calibration.c
    ${BOARD_PATH}/calibration_v5.c
    ${BOARD_PATH}/driver_list.c
    ${BOARD_PATH}/ex10_gpio.c
    ${BOARD_PATH}/ex10_print.c
    ${BOARD_PATH}/ex10_random.c
    ${BOARD_PATH}/ex10_random.c
    ${BOARD_PATH}/ex10_rx_baseband_filter.c
    ${BOARD_PATH}/ex10_osal.c
    ${BOARD_PATH}/fifo_buffer_pool.c
    ${BOARD_PATH}/gpio_driver.c
    ${BOARD_PATH}/rssi_compensation_lut.c
    ${BOARD_PATH}/spi_driver.c
    ${BOARD_PATH}/time_helpers.c
    ${BOARD_PATH}/uart_driver.c
    ${BOARD_PATH}/uart_helpers.c

    ${BOARD_PATH}/calibration_lut_gen2x.c
    ${BOARD_PATH}/ex10_rx_baseband_filter_gen2x.c
    ${BOARD_PATH}/rssi_compensation_lut_gen2x.c

    ${EX10}_api/aggregate_op_builder.c 
    ${EX10}_api/application_registers.c 
    ${EX10}_api/board_init_core.c 
    ${EX10}_api/board_init.c 
    ${EX10}_api/command_transactor.c 
    ${EX10}_api/commands.c 
    ${EX10}_api/crc16.c 
    ${EX10}_api/event_fifo_printer.c 
    ${EX10}_api/event_packet_parser.c 
    ${EX10}_api/ex10_active_region.c 
    ${EX10}_api/ex10_api_strings.c 
    ${EX10}_api/ex10_autoset_modes.c 
    ${EX10}_api/ex10_boot_health.c 
    ${EX10}_api/ex10_device_time.c 
    ${EX10}_api/ex10_dynamic_power_ramp.c 
    ${EX10}_api/ex10_event_fifo_queue.c 
    ${EX10}_api/ex10_gen2_reply_string.c 
    ${EX10}_api/ex10_helpers.c 
    ${EX10}_api/ex10_inventory.c 

    ${EX10}_api/ex10_ops.c 
    ${EX10}_api/ex10_power_modes.c 
    ${EX10}_api/ex10_protocol.c 

    ${EX10}_api/ex10_regulatory.c 
    ${EX10}_api/ex10_result_strings.c 
    ${EX10}_api/ex10_result.c 
    ${EX10}_api/ex10_rf_power.c 
    ${EX10}_api/ex10_select_commands.c 
    ${EX10}_api/ex10_simple_example_init.c   
    ${EX10}_api/ex10_test.c 
    ${EX10}_api/ex10_utils.c 
    ${EX10}_api/fifo_buffer_list.c 
    ${EX10}_api/gen2_commands.c 
    ${EX10}_api/gen2_tx_command_manager.c 
    ${EX10}_api/legacy_rf_mode_converter.c    
    ${EX10}_api/list_node.c 
    ${EX10}_api/power_transactor.c 
    ${EX10}_api/print_data.c 
    ${EX10}_api/rf_mode_definitions.c 
    ${EX10}_api/sjc_accessor.c 
    ${EX10}_api/trace.c 
    ${EX10}_api/version_info.c

    ${EX10}_modules/ex10_ramp_module_manager.c
    ${EX10}_modules/ex10_antenna_disconnect.c
    ${EX10}_modules/ex10_listen_before_talk.c
    
    ${EX10}_use_cases/ex10_activity_sequence_use_case.c
    ${EX10}_use_cases/ex10_continuous_inventory_power_sweep_use_case.c
    ${EX10}_use_cases/ex10_continuous_inventory_start_use_case.c
    ${EX10}_use_cases/ex10_continuous_inventory_use_case.c
    ${EX10}_use_cases/ex10_inventory_sequence_use_case.c
    ${EX10}_use_cases/ex10_tag_access_use_case.c    

    ${EX10}_regulatory/ex10_default_region_names.c
    ${EX10}_regulatory/ex10_off_time_helpers.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_australia.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_brazil.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_china.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_error.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_etsi_lower.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_etsi_upper.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_fcc.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_hk.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_india.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_indonesia.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_israel.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_japan_916_921_mhz.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_korea.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_malaysia.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_new_zealand.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_peru.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_philippines.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_russia.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_singapore.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_south_africa.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_taiwan.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_thailand.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_uruguay.c
    ${EX10}_regulatory/ex10_regulatory_regions/ex10_regulatory_vietnam.c

    ${EX10}_utils/ex10_command_line.c
    ${EX10}_utils/ex10_inventory_command_line.c
    ${EX10}_utils/ex10_inventory_command_line_helper.c
    ${EX10}_utils/ex10_use_case_example_errors.c

    ${EX10_GEN2X}_api/event_fifo_printer_gen2x.c
    ${EX10_GEN2X}_api/ex10_autoset_modes_gen2x.c
    ${EX10_GEN2X}_api/ex10_inventory_gen2x.c
    ${EX10_GEN2X}_api/rf_mode_definitions_gen2x.c
    ${EX10_GEN2X}_modules/ex10_algo_autoset.c
    ${EX10_GEN2X}_use_cases/ex10_continuous_inventory_use_case_gen2x.c
    ${EX10_GEN2X}_use_cases/ex10_fast_tag_tracking_use_case_gen2x.c
    ${EX10_GEN2X}_use_cases/ex10_tag_access_use_case_gen2x.c
    
)


#########################################################################################################################################
# Yukon SDK
#########################################################################################################################################

set(VERSION_DISPLAY ${VERSION})

option(EX10_ENABLE_PRINT        "Enable/Disable library printfs to stdout" ON)
option(EX10_ENABLE_PRINT_ERR    "Enable/Disable library printfs to stderr" ON)
option(EX10_ENABLE_PRINT_EX     "Enable/Disable example printfs to stdout" ON)
option(EX10_ENABLE_PRINT_EX_ERR "Enable/Disable example printfs to stderr" ON)

if (EX10_ENABLE_PRINT)
    add_definitions(-DEX10_ENABLE_PRINT)
endif()
if (EX10_ENABLE_PRINT_ERR)
    add_definitions(-DEX10_ENABLE_PRINT_ERR)
endif()

if (EX10_ENABLE_PRINT_EX)
    add_definitions(-DEX10_ENABLE_PRINT_EX)
endif()
if (EX10_ENABLE_PRINT_EX_ERR)
    add_definitions(-DEX10_ENABLE_PRINT_EX_ERR)
endif()

if(NOT DEFINED BOARD_TARGET OR "${BOARD_TARGET}" STREQUAL "")
#    set(BOARD_TARGET macos_arm64_usb2spi)
    set(BOARD_TARGET zephyr_rtos)
    message(STATUS "Setting to default BOARD_TARGET     : ${BOARD_TARGET}")
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE OR "${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release")
    message(STATUS "Setting to default CMAKE_BUILD_TYPE : ${CMAKE_BUILD_TYPE}")
endif()

message(STATUS "TOOLCHAIN                   : ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "VERSION                     : ${VERSION_DISPLAY}")
message(STATUS "BOARD_TARGET                : ${BOARD_TARGET}")
message(STATUS "CMAKE_BUILD_TYPE            : ${CMAKE_BUILD_TYPE}")
message(STATUS "LTTNG                       : ${LTTNG}")
message(STATUS "EX10_ENABLE_PRINT           : ${EX10_ENABLE_PRINT}")
message(STATUS "EX10_ENABLE_PRINT_ERR       : ${EX10_ENABLE_PRINT_ERR}")
message(STATUS "EX10_ENABLE_PRINT_EX        : ${EX10_ENABLE_PRINT_EX}")
message(STATUS "EX10_ENABLE_PRINT_EX_ERR    : ${EX10_ENABLE_PRINT_EX_ERR}")

# block() # -- Not available until CMake version 3.26
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE_UPPER)
message(STATUS  "CMAKE_CONFIGURATION_TYPES   : ${CMAKE_CONFIGURATION_TYPES}")
message(STATUS  "C_FLAGS                     : ${CMAKE_C_FLAGS_${BUILD_TYPE_UPPER}}")
message(STATUS  "CXX_FLAGS                   : ${CMAKE_CXX_FLAGS_${BUILD_TYPE_UPPER}}")
message(STATUS  "EXE_LINKER_FLAGS            : ${CMAKE_EXE_LINKER_FLAGS_${BUILD_TYPE_UPPER}}")
message(STATUS  "SHARED_LINKER_FLAGS         : ${CMAKE_SHARED_LINKER_FLAGS_${BUILD_TYPE_UPPER}}")
# endblock()

message(STATUS  "SHARED_LIB_SPECIFIC_LIBRARIES : ${SHARED_LIB_SPECIFIC_LIBRARIES}")
message(STATUS  "EXECUTABLE_SPECIFIC_LIBRARIES : ${EXECUTABLE_SPECIFIC_LIBRARIES}")
message(STATUS  "CMAKE_OBJCOPY                 : ${CMAKE_OBJCOPY}")
message(STATUS  "CMAKE_OBJDUMP                 : ${CMAKE_OBJDUMP}")
message(STATUS  "CMAKE_NM                      : ${CMAKE_NM}")
message(STATUS  "CMAKE_SIZE                    : ${CMAKE_SIZE}")

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/include/ex10_use_cases
    ${PROJECT_SOURCE_DIR}/src/include/ex10_api
    ${PROJECT_SOURCE_DIR}/src/include/ex10_modules
    ${PROJECT_SOURCE_DIR}/src/include/regulatory
    ${PROJECT_SOURCE_DIR}/src/ex10
    ${PROJECT_SOURCE_DIR}/src/board
    ${PROJECT_SOURCE_DIR}/src/board/zephyr_rtos

    ${PROJECT_SOURCE_DIR}/src/include_gen2x
    ${PROJECT_SOURCE_DIR}/src/include_gen2x/ex10_use_cases
    ${PROJECT_SOURCE_DIR}/src/include_gen2x/ex10_api
    ${PROJECT_SOURCE_DIR}/src/include_gen2x/ex10_modules
 )

#add_subdirectory(src/ex10)
#add_subdirectory(src/board/zephyr_rtos)

#target_link_libraries( # Specifies the target library.
#    app
#
#    yukon_board
#    yukon_sdk
#
##        ${EXECUTABLE_SPECIFIC_LIBRARIES}
##
#    # Links the target library to the log library
#    # included in the NDK.
#    ${log-lib}
#)


