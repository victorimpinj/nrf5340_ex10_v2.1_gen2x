/*****************************************************************************
 *                  IMPINJ CONFIDENTIAL AND PROPRIETARY                      *
 *                                                                           *
 * This source code is the property of Impinj, Inc. Your use of this source  *
 * code in whole or in part is subject to your applicable license terms      *
 * from Impinj.                                                              *
 * Contact support@impinj.com for a copy of the applicable Impinj license    *
 * terms.                                                                    *
 *                                                                           *
 * (c) Copyright 2024 Impinj, Inc. All rights reserved.                      *
 *                                                                           *
 *****************************************************************************/
/**
 * Data structure definition and initialization for Gen2X application registers.
 * This contains the basic register info and a pointer to the register data.
 */

#pragma once

#include <stddef.h>

#include "include_gen2x/ex10_api/application_register_definitions_gen2x.h"

#ifdef __cplusplus
extern "C" {
#endif

// clang-format off
// IPJ_autogen | gen_c_app_ex10_api_reg_instances | visibility:impinj_gen2x {
static struct RegisterInfo const gen2_x_features_control_reg = {
    .name = "Gen2XFeaturesControl",
    .address = 0x1096,
    .length = 0x0001,
    .num_entries = 1,
    .access = ReadWrite,
};
#define GEN2_X_FEATURES_CONTROL_REG_LENGTH  ((size_t)1u)
#define GEN2_X_FEATURES_CONTROL_REG_ENTRIES ((size_t)1u)
static struct RegisterInfo const scan_id_command_control_reg = {
    .name = "ScanIdCommandControl",
    .address = 0x109C,
    .length = 0x0004,
    .num_entries = 1,
    .access = ReadWrite,
};
#define SCAN_ID_COMMAND_CONTROL_REG_LENGTH  ((size_t)4u)
#define SCAN_ID_COMMAND_CONTROL_REG_ENTRIES ((size_t)1u)
static struct RegisterInfo const scan_command_control_reg = {
    .name = "ScanCommandControl",
    .address = 0x10A0,
    .length = 0x0004,
    .num_entries = 1,
    .access = ReadWrite,
};
#define SCAN_COMMAND_CONTROL_REG_LENGTH  ((size_t)4u)
#define SCAN_COMMAND_CONTROL_REG_ENTRIES ((size_t)1u)
// IPJ_autogen }
// clang-format on

#ifdef __cplusplus
}
#endif
