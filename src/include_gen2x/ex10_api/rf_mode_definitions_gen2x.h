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
 * Definitions and descriptions of the Gen2X Ex10 RF modes.
 * Further details on all of the RF modes can be found in section 6.6 of the
 * documentation.
 */

#pragma once

// clang-format off
// IPJ_autogen | gen_c_host_api_rf_modes | visibility:impinj_gen2x {
// A list of all available rf modes for an Ex10 device
enum RfModesGen2X {

    // Available on devices: E310, E510, E710, E910
    mode_4123 = 4123,

    // Available on devices: E510, E710, E910
    mode_4124 = 4124,

    // Available on devices: E310, E510, E710, E910
    mode_4141 = 4141,

    // Available on devices: E310, E510, E710, E910
    mode_4146 = 4146,

    // Available on devices: E510, E710, E910
    mode_4148 = 4148,

    // Available on devices: E310, E510, E710, E910
    mode_4185 = 4185,

    // Available on devices: E310, E510, E710, E910
    mode_4222 = 4222,

    // Available on devices: E310, E510, E710, E910
    mode_4241 = 4241,

    // Available on devices: E310, E510, E710, E910
    mode_4244 = 4244,

    // Available on devices: E310, E510, E710, E910
    mode_4285 = 4285,

    // Available on devices: E510, E710, E910
    mode_4323 = 4323,

    // Available on devices: E310, E510, E710, E910
    mode_4324 = 4324,

    // Available on devices: E310, E510, E710, E910
    mode_4342 = 4342,

    // Available on devices: E310, E510, E710, E910
    mode_4343 = 4343,

    // Available on devices: E510, E710, E910
    mode_4345 = 4345,

    // Available on devices: E310, E510, E710, E910
    mode_4382 = 4382,
};
// IPJ_autogen }
// clang-format on

#define EX10_GEN2X_MODE_ID_MIN 4000
#define EX10_GEN2X_MODE_ID_MAX 4999

bool  get_is_gen2x(uint16_t rf_mode);
int   get_blf_khz_gen2x(uint16_t rf_mode);
int   get_miller_gen2x(uint16_t rf_mode);
int   get_lpf_bw_khz_gen2x(uint16_t rf_mode);
int   get_tx_mod_gen2x(uint16_t);
float get_pie_gen2x(uint16_t rf_mode);
float tari_us_gen2x(uint16_t rf_mode);
