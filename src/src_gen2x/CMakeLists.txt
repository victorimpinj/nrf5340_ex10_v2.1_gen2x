#############################################################################
#                  IMPINJ CONFIDENTIAL AND PROPRIETARY                      #
#                                                                           #
# This source code is the property of Impinj, Inc. Your use of this source  #
# code in whole or in part is subject to your applicable license terms      #
# from Impinj.                                                              #
# Contact support@impinj.com for a copy of the applicable Impinj license    #
# terms.                                                                    #
#                                                                           #
# (c) Copyright 2024 Impinj, Inc. All rights reserved.                      #
#                                                                           #
#############################################################################

cmake_minimum_required(VERSION 3.13)

project(ex10_host_library_gen2x
    VERSION ${VERSION}
    DESCRIPTION "Impinj Reader Chip, Gen2X host library"
    LANGUAGES C
)

set(LIBRARY_NAME host_gen2x)

# For CMake 3.13 compatibility, create an intermediate object library first,
# and use its object list to create the static library. This allows the
# $<TARGET_OBJECTS:${LIBRARY_NAME}_objects> to be accessed.
# For CMake 3.15 and later, $<TARGET_OBJECTS:${LIBRARY_NAME}> can be accessed
# without the intermediate  $<TARGET_OBJECTS:${LIBRARY_NAME}_objects>.
add_library(${LIBRARY_NAME}_objects OBJECT)
target_sources(${LIBRARY_NAME}_objects
    PRIVATE
    ex10_api/event_fifo_printer_gen2x.c
    ex10_api/ex10_inventory_gen2x.c
    ex10_api/rf_mode_definitions_gen2x.c
    ex10_api/ex10_autoset_modes_gen2x.c
    ex10_modules/ex10_algo_autoset.c
    ex10_use_cases/ex10_continuous_inventory_use_case_gen2x.c
    ex10_use_cases/ex10_tag_access_use_case_gen2x.c
    ex10_use_cases/ex10_fast_tag_tracking_use_case_gen2x.c
)

add_library(${LIBRARY_NAME} STATIC)

target_sources(${LIBRARY_NAME}
    PRIVATE
    $<TARGET_OBJECTS:${LIBRARY_NAME}_objects>
)
