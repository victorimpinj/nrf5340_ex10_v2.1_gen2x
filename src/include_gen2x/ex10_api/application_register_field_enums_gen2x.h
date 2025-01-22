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
/**
 * enums used by the register fields
 */

#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// clang-format off
// IPJ_autogen | gen_c_app_ex10_api_reg_field_enums | visibility:impinj_gen2x {
enum ScanIdCommandControlAppSizeGen2X {
    AppSizeRfu                   = 0x00,
    AppSize24Bits                = 0x01,
    AppSize16Bits                = 0x02,
    AppSize8Bits                 = 0x03,
};

enum ScanCommandControlCodeGen2X {
    CodeRfu                      = 0x00,
    CodeAntipodal                = 0x01,
    CodeCCOneHalf                = 0x02,
    CodeCCThreeQuarters          = 0x03,
};

enum ScanCommandControlCrGen2X {
    CrID32                       = 0x00,
    CrID16                       = 0x01,
    CrStoredCRC                  = 0x02,
    CrRN16                       = 0x03,
};

enum ScanCommandControlProtectionGen2X {
    ProtectionNoProtection       = 0x00,
    ProtectionParity             = 0x01,
    ProtectionCRC5               = 0x02,
    ProtectionCRC5Plus           = 0x03,
};

enum ScanCommandControlIdGen2X {
    IdNoAckResponse              = 0x00,
    IdTMNPlusTSN                 = 0x01,
    IdPart                       = 0x02,
    IdFull                       = 0x03,
};
// IPJ_autogen }
// clang-format on

#ifdef __cplusplus
}
#endif
