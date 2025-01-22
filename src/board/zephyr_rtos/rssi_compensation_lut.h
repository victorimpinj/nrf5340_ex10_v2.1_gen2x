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

#pragma once

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * RSSI Compensation Look Up Table
 * These variables are look up tables used for RSSI compensation.
 *
 * These values are derived empirically but do not vary significantly from
 * part to part as they are digital offsets. These values, combined with
 * calibration data are used in unison to compensate for RSSI.
 */

/**
 * @enum RxModeIndex
 */
enum RxModeIndex
{
    RX_MODE_0_IDX  = 0,
    RX_MODE_4_IDX  = 1,
    RX_MODE_5_IDX  = 2,
    RX_MODE_6_IDX  = 3,
    RX_MODE_11_IDX = 4,
    RX_MODE_12_IDX = 5,
    RX_MODE_16_IDX = 6,
    RX_MODE_17_IDX = 7,
    RX_MODE_18_IDX = 8,
    RX_MODE_21_IDX = 9,
    RX_MODE_23_IDX = 10,
    RX_MODE_44_IDX = 11,
    RX_MODE_46_IDX = 12,
    RX_MODE_47_IDX = 13,
    RX_MODE_48_IDX = 14,
    RX_MODE_49_IDX = 15,
    RX_MODE_50_IDX = 16,
    RX_MODE_51_IDX = 17,
    RX_MODE_52_IDX = 18,
};

/**
 * The number of Rx modes supported by the Impinj Reader Chip.
 * Each RF mode consists of a Tx and Rx mode.
 */
#define NUM_RX_MODES ((size_t)19)

/**
 * The number of RF modes used for calibration.
 */
#define NUM_DRM_CAL_MODES ((size_t)2)
#define NUM_NON_DRM_CAL_MODES ((size_t)3)

/**
 * @struct CalMode
 *
 * This structure encapsulates the mode information relevent for calibration -
 * mode ID and backscatter link frequency (BLF)
 */
struct CalMode
{
    uint16_t mode_id;  // Mode ID
    uint16_t blf;      // Backscatter link frequency (BLF)
};


/**
 * @struct RssiCompensationLut
 *
 * The RSSI Compensation Look Up Table is used to interpolate/extrapolate
 * all RF modes based on the Rx baseband filter(i.e. Dense Reader Mode BPF or
 * non-DRM HPF) and the Backscatter Link Frequency (BLF).
 */
struct RssiCompensationLut
{
    uint16_t const num_modes;

    /// The supported rf modes
    uint16_t const* rf_modes;

    /// the RX modes that each rf mode corresponds to
    uint16_t const* rf_mode_to_rx_mode;

    /// BLF in kHz for each Rx mode.
    uint16_t const rx_modes_blf_khz[NUM_RX_MODES];

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
    struct CalMode const drm_cal_modes[NUM_DRM_CAL_MODES];
    struct CalMode const non_drm_cal_modes[NUM_NON_DRM_CAL_MODES];

    /// Every Rx mode's rssi digital frequency response.
    int16_t const rx_mode_digital_correction[NUM_RX_MODES];

    /// LBT digital correction.
    /// This value was derived empirically.
    int16_t const lbt_digital_correction;

    /// LBT cordic frequency shift
    uint16_t const lbt_cordic_freq_shift;
};

struct RssiCompensationLut const* get_ex10_rssi_compensation(void);

#ifdef __cplusplus
}
#endif
