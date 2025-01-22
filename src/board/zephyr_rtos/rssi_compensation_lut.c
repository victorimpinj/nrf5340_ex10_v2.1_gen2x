/*****************************************************************************
 *                  IMPINJ CONFIDENTIAL AND PROPRIETARY                      *
 *                                                                           *
 * This source code is the property of Impinj, Inc. Your use of this source  *
 * code in whole or in part is subject to your applicable license terms      *
 * from Impinj.                                                              *
 * Contact support@impinj.com for a copy of the applicable Impinj license    *
 * terms.                                                                    *
 *                                                                           *
 * (c) Copyright 2022 - 2024 Impinj, Inc. All rights reserved.               *
 *                                                                           *
 *****************************************************************************/

#include "board/zephyr_rtos/rssi_compensation_lut.h"
#include "ex10_api/ex10_print.h"


// IPJ_autogen | board_rssi_compensation_lut {
#define NUM_MODES 34u

// clang-format off
static uint16_t const rf_modes[NUM_MODES] = {
     102,  103,  104,  120,  123,  124,  125,  126,  141,  146,  147,  148,
     185,  202,  203,  205,  222,  223,  224,  225,  226,  241,  244,  285,
     302,  323,  324,  325,  326,  342,  343,  344,  345,  382,
};
// clang-format on

static uint16_t const rf_mode_to_rx_mode[NUM_MODES] = {
    RX_MODE_6_IDX,  RX_MODE_6_IDX,  RX_MODE_4_IDX,  RX_MODE_12_IDX,
    RX_MODE_11_IDX, RX_MODE_12_IDX, RX_MODE_11_IDX, RX_MODE_11_IDX,
    RX_MODE_17_IDX, RX_MODE_16_IDX, RX_MODE_18_IDX, RX_MODE_18_IDX,
    RX_MODE_21_IDX, RX_MODE_5_IDX,  RX_MODE_5_IDX,  RX_MODE_0_IDX,
    RX_MODE_11_IDX, RX_MODE_11_IDX, RX_MODE_11_IDX, RX_MODE_51_IDX,
    RX_MODE_51_IDX, RX_MODE_17_IDX, RX_MODE_16_IDX, RX_MODE_21_IDX,
    RX_MODE_6_IDX,  RX_MODE_12_IDX, RX_MODE_11_IDX, RX_MODE_11_IDX,
    RX_MODE_11_IDX, RX_MODE_17_IDX, RX_MODE_16_IDX, RX_MODE_18_IDX,
    RX_MODE_18_IDX, RX_MODE_21_IDX,
};

/**
 * RSSI Compensation Look Up Table
 * These variables are look up tables used for RSSI compensation.
 *
 * These values are derived empirically but do not vary significantly from
 * part to part as they are digital offsets. These values, combined with
 * calibration data are used in unison to compensate for RSSI.
 */
static struct RssiCompensationLut rssi_compensation = {
    .num_modes          = NUM_MODES,
    .rf_modes           = rf_modes,
    .rf_mode_to_rx_mode = rf_mode_to_rx_mode,

    // The 5 calibration modes we recommend for the reference design board.
    // These 5 modes must meet the following criteria for optimal RSSI
    // calibration: For all modes we recommend picking modes with M > 1 to
    // support SKUs E310 and E510 which cannot handle FM0 modes
    //   drm_250 - DRM, BLF 250 kHz, M > 1
    //   drm_320 - DRM, BLF 320 kHz, M > 1
    //   non_drm_160 - Non-DRM, BLF 160 kHz, M > 1
    //   non_drm_320 - Non-DRM, BLF 320 kHz, M > 1
    //   non_drm_640 - Non-DRM, BLF 640 kHz, M > 1
    //
    // Recommended modes for calibration for each SKU, (see pc_cal_example.py):
    //   E910, E710 - 146, 141, 185, 123, 124
    //   E510 (No M=1 modes) - 146, 141, 185, 123, 124
    //   E310 (No BLF > 320 kHz) - 146, 141, 185, 123
    //
    // If the DRM status changes for any of the following modes, they must be
    // replaced.
    //
    // The modes must be listed in increasing order of BLF. This will be checked
    // at runtime using the function check_sorted_by_blf which will fail if the
    // requirement is not met.
    //
    // For ideal performance, modes should be selected so that the BLFs of the
    // modes span the passband of the DRM and Non-DRM filters.
    //
    // If a part was calibrated before FW 2.1 using modes that are no longer
    // supported, they will be remapped to the following existing modes.
    // 1 -> 124
    // 3 -> 123
    // 5 -> 146
    // 7 -> 146
    // 11 -> 125
    // 12 -> 125
    // 13 -> 185
    // 15 -> 147
    // No re-calibration of these parts is necessary.

    // DRM filter calibration modes
    .drm_cal_modes =
        {
            {.mode_id = 146, .blf = 250},
            {.mode_id = 141, .blf = 320},
        },

    // Non-DRM filter calibration modes
    .non_drm_cal_modes =
        {
            {.mode_id = 185, .blf = 160},
            {.mode_id = 123, .blf = 320},
            {.mode_id = 124, .blf = 640},
        },

    .rx_mode_digital_correction =
        {
            // RX Mode 0
            -149,
            // RX Mode 4
            49,
            // RX Mode 5
            10,
            // RX Mode 6
            131,
            // RX Mode 11
            1,
            // RX Mode 12
            84,
            // RX Mode 16
            -33,
            // RX Mode 17
            20,
            // RX Mode 18
            108,
            // RX Mode 21
            1,
            // RX Mode 23
            58,
            // RX Mode 44
            20,
            // RX Mode 46
            83,
            // RX Mode 47
            1,
            // RX Mode 48
            -32,
            // RX Mode 49
            1,
            // RX Mode 50
            108,
            // RX Mode 51
            48,
            // RX Mode 52
            48,
        },
    .rx_modes_blf_khz =
        {
            // RX Mode 0
            50,
            // RX Mode 4
            320,
            // RX Mode 5
            426,
            // RX Mode 6
            640,
            // RX Mode 11
            320,
            // RX Mode 12
            640,
            // RX Mode 16
            250,
            // RX Mode 17
            320,
            // RX Mode 18
            640,
            // RX Mode 21
            160,
            // RX Mode 23
            320,
            // RX Mode 44
            320,
            // RX Mode 46
            640,
            // RX Mode 47
            320,
            // RX Mode 48
            250,
            // RX Mode 49
            160,
            // RX Mode 50
            640,
            // RX Mode 51
            426,
            // RX Mode 52
            426,
        },
    .lbt_digital_correction = -37,
    .lbt_cordic_freq_shift  = 200,
};

struct RssiCompensationLut const* get_ex10_rssi_compensation(void)
{
    return &rssi_compensation;
}
// IPJ_autogen }
