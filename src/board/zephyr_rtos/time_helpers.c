/*****************************************************************************
 *                  IMPINJ CONFIDENTIAL AND PROPRIETARY                      *
 *                                                                           *
 * This source code is the property of Impinj, Inc. Your use of this source  *
 * code in whole or in part is subject to your applicable license terms      *
 * from Impinj.                                                              *
 * Contact support@impinj.com for a copy of the applicable Impinj license    *
 * terms.                                                                    *
 *                                                                           *
 * (c) Copyright 2020 - 2023 Impinj, Inc. All rights reserved.               *
 *                                                                           *
 *****************************************************************************/

#include "board/time_helpers.h"
#include <zephyr/kernel.h>

/*
#include <assert.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
*/


static uint32_t ex10_time_now(void)
{
    return k_uptime_get_32();
}

static uint32_t ex10_time_elapsed(uint32_t start_time)
{
    uint32_t const time = ex10_time_now();

    // If we rolled over, calculate the time from the start time to
    // max uint32_t and then add the time from 0 to time.
    uint32_t const time_elapsed = (time < start_time)
                                      ? (time + (UINT32_MAX - start_time))
                                      : (time - start_time);

    return time_elapsed;
}

static void ex10_busy_wait_ms(uint32_t msec_to_wait)
{
    uint64_t end_time_ms = k_uptime_get() + msec_to_wait;
    while (k_uptime_get() < end_time_ms)
    {
    }
}

// This suspends the caller from execution (at least) msec_to_wait
static void ex10_wait_ms(uint32_t msec_to_wait)
{
    k_msleep(msec_to_wait);
}

static struct Ex10TimeHelpers ex10_time_helpers = {
    .time_now     = ex10_time_now,
    .time_elapsed = ex10_time_elapsed,
    .busy_wait_ms = ex10_busy_wait_ms,
    .wait_ms      = ex10_wait_ms,
};

struct Ex10TimeHelpers* get_ex10_time_helpers(void)
{
    return &ex10_time_helpers;
}
