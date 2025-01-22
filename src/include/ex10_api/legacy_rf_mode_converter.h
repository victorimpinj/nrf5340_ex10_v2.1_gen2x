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

#pragma once

#include "ex10_api/rf_mode_definitions.h"


#ifdef __cplusplus
extern "C" {
#endif

#define NUM_LEGACY_MODES 8u

struct LegacyRfModeConverter
{
    uint16_t const     legacy_rf_mode_id_list[NUM_LEGACY_MODES];
    enum RfModes const supported_rf_mode_list[NUM_LEGACY_MODES];
};

/**
 * Maps the given legacy RF mode to a supported RF mode. This is to ensure
 * backwards compatibility with FW 2.0 and earlier versions.
 *
 * @param legacy_rf_mode_id The legacy RF mode ID
 * @return enum RfModes     If the given legacy_rf_mode_id is found in the
 *                          legacy mode id list, return the matching RF mode
 *                          that is supported by the SDK. If not found, return
 *                          back the given legacy_rf_mode_id.
 */
enum RfModes convert_legacy_rf_mode_to_new_rf_mode(uint16_t legacy_rf_mode_id);

#ifdef __cplusplus
}
#endif
