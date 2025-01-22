/*****************************************************************************
 *                  IMPINJ CONFIDENTIAL AND PROPRIETARY                      *
 *                                                                           *
 * This source code is the property of Impinj, Inc. Your use of this source  *
 * code in whole or in part is subject to your applicable license terms      *
 * from Impinj.                                                              *
 * Contact support@impinj.com for a copy of the applicable Impinj license    *
 * terms.                                                                    *
 *                                                                           *
 * (c) Copyright 2023 - 2024 Impinj, Inc. All rights reserved.               *
 *                                                                           *
 *****************************************************************************/

#pragma once

#include "ex10_api/application_registers.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void                   clear_boot_flag(void);
bool                   get_reboot_occurred(void);
void                   print_reset_cause(void);
struct CrashInfoFields print_crash_info(void);

#ifdef __cplusplus
}
#endif
