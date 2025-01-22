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

#include "ex10_api/ex10_boot_health.h"
#include <ctype.h>

#include "board/ex10_osal.h"
#include "ex10_api/ex10_macros.h"
#include "ex10_api/ex10_print.h"
#include "ex10_api/ex10_protocol.h"


void clear_boot_flag(void)
{
    struct Ex10BootFlagFields ex10_boot_flag = {.boot_flag = false, .rfu1 = 0};
    get_ex10_protocol()->write(&ex10_boot_flag_reg, &ex10_boot_flag);
}

bool get_reboot_occurred(void)
{
    // Check if the device rebooted into the bootloader
    if (get_ex10_protocol()->get_running_location() == Bootloader)
    {
        return true;
    }
    // Ask the app if it recently booted. The user should have
    // cleared the original boot flag at start, so a set flag means
    // a reboot occurred.
    struct Ex10BootFlagFields ex10_boot_flag;
    get_ex10_protocol()->read(&ex10_boot_flag_reg, &ex10_boot_flag);
    if (ex10_boot_flag.boot_flag)
    {
        return true;
    }
    return false;
}

void print_reset_cause(void)
{
    struct ResetCauseFields reset_cause;
    get_ex10_protocol()->read(&reset_cause_reg, &reset_cause);

    ex10_ex_eprintf(
        "ResetCause: software_reset: %u, watchdog_timeout: %u, lockup: %u, "
        "external_reset: %u",
        reset_cause.software_reset,
        reset_cause.watchdog_timeout,
        reset_cause.lockup,
        reset_cause.external_reset);
}

struct CrashInfoFields print_crash_info(void)
{
    // This data should be given to Impinj for further debug.
    static uint8_t         crash_info_arr[CRASH_INFO_REG_LENGTH];
    struct CrashInfoFields crash_info = {.data = crash_info_arr};
    get_ex10_protocol()->read(&crash_info_reg, &crash_info);
    return crash_info;
}
