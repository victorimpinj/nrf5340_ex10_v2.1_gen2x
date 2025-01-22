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
 * Definitions and descriptions of the Ex10 Gen2X RF modes
 */

#include <stddef.h>

#include "ex10_api/rf_mode_definitions.h"
#include "include_gen2x/ex10_api/rf_mode_definitions_gen2x.h"

// clang-format off
// IPJ_autogen | generate_host_api_rf_modes_info | visibility:impinj_gen2x {
static const struct RfModeInfo rf_mode_infos_gen2x[] = {
    {.rf_mode = mode_4123,
     .blf_khz     = 320,
     .miller      = 2,
     .lpf_bw_khz  = 600,
     .tx_mod      = PR_ASK,
     .pie         = 2.0,
     .tari_us     = 20.0},
    {.rf_mode = mode_4124,
     .blf_khz     = 640,
     .miller      = 2,
     .lpf_bw_khz  = 1200,
     .tx_mod      = PR_ASK,
     .pie         = 2.0,
     .tari_us     = 7.5},
    {.rf_mode = mode_4141,
     .blf_khz     = 320,
     .miller      = 4,
     .lpf_bw_khz  = 400,
     .tx_mod      = PR_ASK,
     .pie         = 2.0,
     .tari_us     = 20.0},
    {.rf_mode = mode_4146,
     .blf_khz     = 250,
     .miller      = 4,
     .lpf_bw_khz  = 390,
     .tx_mod      = PR_ASK,
     .pie         = 2.0,
     .tari_us     = 20.0},
    {.rf_mode = mode_4148,
     .blf_khz     = 640,
     .miller      = 4,
     .lpf_bw_khz  = 800,
     .tx_mod      = PR_ASK,
     .pie         = 1.5,
     .tari_us     = 7.5},
    {.rf_mode = mode_4185,
     .blf_khz     = 160,
     .miller      = 8,
     .lpf_bw_khz  = 300,
     .tx_mod      = PR_ASK,
     .pie         = 2.0,
     .tari_us     = 20.0},
    {.rf_mode = mode_4222,
     .blf_khz     = 320,
     .miller      = 2,
     .lpf_bw_khz  = 600,
     .tx_mod      = PR_ASK,
     .pie         = 2.0,
     .tari_us     = 20.0},
    {.rf_mode = mode_4241,
     .blf_khz     = 320,
     .miller      = 4,
     .lpf_bw_khz  = 400,
     .tx_mod      = PR_ASK,
     .pie         = 2.0,
     .tari_us     = 20.0},
    {.rf_mode = mode_4244,
     .blf_khz     = 250,
     .miller      = 4,
     .lpf_bw_khz  = 390,
     .tx_mod      = PR_ASK,
     .pie         = 2.0,
     .tari_us     = 20.0},
    {.rf_mode = mode_4285,
     .blf_khz     = 160,
     .miller      = 8,
     .lpf_bw_khz  = 300,
     .tx_mod      = PR_ASK,
     .pie         = 2.0,
     .tari_us     = 20.0},
    {.rf_mode = mode_4323,
     .blf_khz     = 640,
     .miller      = 2,
     .lpf_bw_khz  = 1200,
     .tx_mod      = PR_ASK,
     .pie         = 2.0,
     .tari_us     = 7.5},
    {.rf_mode = mode_4324,
     .blf_khz     = 320,
     .miller      = 2,
     .lpf_bw_khz  = 600,
     .tx_mod      = PR_ASK,
     .pie         = 2.0,
     .tari_us     = 20.0},
    {.rf_mode = mode_4342,
     .blf_khz     = 320,
     .miller      = 4,
     .lpf_bw_khz  = 400,
     .tx_mod      = PR_ASK,
     .pie         = 2.0,
     .tari_us     = 20.0},
    {.rf_mode = mode_4343,
     .blf_khz     = 250,
     .miller      = 4,
     .lpf_bw_khz  = 390,
     .tx_mod      = PR_ASK,
     .pie         = 2.0,
     .tari_us     = 20.0},
    {.rf_mode = mode_4345,
     .blf_khz     = 640,
     .miller      = 4,
     .lpf_bw_khz  = 800,
     .tx_mod      = PR_ASK,
     .pie         = 1.5,
     .tari_us     = 7.5},
    {.rf_mode = mode_4382,
     .blf_khz     = 160,
     .miller      = 8,
     .lpf_bw_khz  = 300,
     .tx_mod      = PR_ASK,
     .pie         = 2.0,
     .tari_us     = 20.0},
};
// IPJ_autogen }
// clang-format on

static const struct RfModeInfo* find_rf_mode(uint16_t rf_mode)
{
    const size_t num_elements =
        sizeof(rf_mode_infos_gen2x) / sizeof(struct RfModeInfo);
    for (size_t index = 0; index < num_elements; index++)
    {
        if (rf_mode_infos_gen2x[index].rf_mode == rf_mode)
        {
            return &rf_mode_infos_gen2x[index];
        }
    }
    return NULL;
}

bool get_is_gen2x(uint16_t rf_mode)
{
    const struct RfModeInfo* rf_mode_info = find_rf_mode(rf_mode);

    if (rf_mode_info == NULL)
    {
        return false;
    }
    return (rf_mode_info->rf_mode >= EX10_GEN2X_MODE_ID_MIN &&
            rf_mode_info->rf_mode <= EX10_GEN2X_MODE_ID_MAX);
}

int get_blf_khz_gen2x(uint16_t rf_mode)
{
    const struct RfModeInfo* rf_mode_info = find_rf_mode(rf_mode);

    if (rf_mode_info == NULL)
    {
        return -1;
    }
    return rf_mode_info->blf_khz;
}

int get_miller_gen2x(uint16_t rf_mode)
{
    const struct RfModeInfo* rf_mode_info = find_rf_mode(rf_mode);

    if (rf_mode_info == NULL)
    {
        return -1;
    }
    return rf_mode_info->miller;
}

int get_lpf_bw_khz_gen2x(uint16_t rf_mode)
{
    const struct RfModeInfo* rf_mode_info = find_rf_mode(rf_mode);

    if (rf_mode_info == NULL)
    {
        return -1;
    }
    return rf_mode_info->lpf_bw_khz;
}

int get_tx_mod_gen2x(uint16_t rf_mode)
{
    const struct RfModeInfo* rf_mode_info = find_rf_mode(rf_mode);

    if (rf_mode_info == NULL)
    {
        return -1;
    }
    return rf_mode_info->tx_mod;
}

float get_pie_gen2x(uint16_t rf_mode)
{
    const struct RfModeInfo* rf_mode_info = find_rf_mode(rf_mode);

    if (rf_mode_info == NULL)
    {
        return -1.0;
    }
    return rf_mode_info->pie;
}

float tari_us_gen2x(uint16_t rf_mode)
{
    const struct RfModeInfo* rf_mode_info = find_rf_mode(rf_mode);

    if (rf_mode_info == NULL)
    {
        return -1.0;
    }
    return rf_mode_info->tari_us;
}
