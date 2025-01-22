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

#include <stddef.h>

#include "ex10_api/ex10_print.h"
#include "ex10_api/legacy_rf_mode_converter.h"


static const struct LegacyRfModeConverter legacy_rf_mode_conversion_table = {
    .legacy_rf_mode_id_list = {1, 3, 5, 7, 11, 12, 13, 15},
    .supported_rf_mode_list = {mode_124,
                               mode_123,
                               mode_141,
                               mode_146,
                               mode_102,
                               mode_125,
                               mode_185,
                               mode_147}};


enum RfModes convert_legacy_rf_mode_to_new_rf_mode(uint16_t legacy_rf_mode)
{
    for (size_t idx = 0; idx < NUM_LEGACY_MODES; idx++)
    {
        if (legacy_rf_mode ==
            legacy_rf_mode_conversion_table.legacy_rf_mode_id_list[idx])
        {
            return legacy_rf_mode_conversion_table.supported_rf_mode_list[idx];
        }
    }

    // Will reach this path if the given legacy_rf_mode is not in the legacy RF
    // Modes list. In this case, return the given legacy_rf_mode.
    return legacy_rf_mode;
}
