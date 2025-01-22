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
 * Field definitions for Gen2X registers in an EX10 device.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "application_register_field_enums_gen2x.h"

#ifdef __cplusplus
extern "C" {
#endif

// clang-format off
// IPJ_autogen | gen_c_app_ex10_api_reg | visibility:impinj_gen2x {
// Structs which break down the fields and sizing within each register
#pragma pack(push, 1)

struct Gen2XFeaturesControlFields {
    bool accept_crc5_and_crc5_plus : 1;
    int8_t rfu : 7;
};

struct ScanIdCommandControlFields {
    bool scan_id_enable : 1;
    uint8_t app_size : 2;
    uint8_t Reserved0 : 5;
    uint32_t app_id : 24;
};

struct ScanCommandControlFields {
    uint8_t n : 4;
    uint8_t code : 2;
    uint8_t cr : 2;
    uint8_t protection : 2;
    uint8_t id : 2;
    bool crypto : 1;
    int32_t rfu : 19;
};


#pragma pack(pop)
// IPJ_autogen }
// clang-format on

#ifdef __cplusplus
}
#endif
