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

// #include "board/e710_ref_design_gen2x/calibration_lut_gen2x.h"
// #include "board/e710_ref_design/calibration.h"
// #include "board/e710_ref_design_gen2x/ex10_rx_baseband_filter_gen2x.h"
// #include "board/e710_ref_design_gen2x/rssi_compensation_lut_gen2x.h"
#include "calibration_lut_gen2x.h"
#include "calibration.h"
#include "ex10_rx_baseband_filter_gen2x.h"
#include "rssi_compensation_lut_gen2x.h"

void set_calibration_lut_gen2x(void)
{
    struct Ex10Calibration const* calibration = get_ex10_calibration();

    calibration->set_rssi_compensation_lut(get_ex10_rssi_compensation_gen2x());
    calibration->set_rx_baseband_filter(get_ex10_rx_baseband_filter_gen2x());
}
