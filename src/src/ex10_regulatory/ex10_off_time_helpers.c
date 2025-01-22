/*****************************************************************************
 *                  IMPINJ CONFIDENTIAL AND PROPRIETARY                      *
 *                                                                           *
 * This source code is the property of Impinj, Inc. Your use of this source  *
 * code in whole or in part is subject to your applicable license terms      *
 * from Impinj.                                                              *
 * Contact support@impinj.com for a copy of the applicable Impinj license    *
 * terms.                                                                    *
 *                                                                           *
 * (c) Copyright 2020 - 2024 Impinj, Inc. All rights reserved.               *
 *                                                                           *
 *****************************************************************************/

#include "ex10_regulatory/ex10_off_time_helpers.h"
#include "board/ex10_osal.h"

// clang-format off

// IPJ_autogen | gen_c_regions_ex10_off_time {

// This is the max number of channels that can be used when storing timings
// for use in off time tracking.
// For all supported regions which use off time, the max number of channels
// needed was used as this definition. If you need support for more channels,
// you can change this number.

#define MAX_NUMBER_CHANNELS 4
// IPJ_autogen }
// clang-format on

static uint32_t channel_last_start[MAX_NUMBER_CHANNELS];
static uint32_t channel_last_end[MAX_NUMBER_CHANNELS];
static uint32_t channel_total_time[MAX_NUMBER_CHANNELS];

static bool get_off_time_observed(uint32_t time_elapsed_ms,
                                  struct Ex10RegulatoryTimers const* timers)
{
    return (time_elapsed_ms >= timers->off_same_channel_ms);
}

static void set_channel_index_start(channel_index_t channel_index,
                                    uint32_t        time_ms,
                                    struct Ex10RegulatoryTimers const* timers)
{
    channel_last_start[channel_index] = time_ms;

    const uint32_t time_since_off = time_ms - channel_last_end[channel_index];

    // If we were not off for the specified off time, then the device did not
    // give enough time to relinquish control of the channel; time since off
    // will thus count towards on time.
    if (get_off_time_observed(time_since_off, timers))
    {
        channel_total_time[channel_index] = 0;
    }
    else
    {
        channel_total_time[channel_index] += time_since_off;
    }
}

static uint32_t get_channel_index_start(channel_index_t channel_index)
{
    return channel_last_start[channel_index];
}

static void set_channel_index_end(channel_index_t channel_index,
                                  uint32_t        time_ms,
                                  struct Ex10RegulatoryTimers const* timers)
{
    (void)timers;
    channel_last_end[channel_index] = time_ms;
    const uint32_t time_on = time_ms - channel_last_start[channel_index];
    channel_total_time[channel_index] += time_on;
}

static uint32_t get_channel_index_end(channel_index_t channel_index)
{
    return channel_last_end[channel_index];
}

static void set_channel_index_total(channel_index_t channel_index,
                                    uint32_t        time_ms,
                                    struct Ex10RegulatoryTimers const* timers)
{
    (void)timers;
    channel_total_time[channel_index] = time_ms;
}

static uint32_t get_channel_index_total(channel_index_t channel_index)
{
    return channel_total_time[channel_index];
}

static void clear_off_time(void)
{
    ex10_memzero(channel_last_start, sizeof(channel_last_start));
    ex10_memzero(channel_last_end, sizeof(channel_last_end));
    ex10_memzero(channel_total_time, sizeof(channel_total_time));
}

static struct Ex10OffTimeHelpers const ex10_off_time_helpers = {
    .get_off_time_observed   = get_off_time_observed,
    .set_channel_index_start = set_channel_index_start,
    .get_channel_index_start = get_channel_index_start,
    .set_channel_index_end   = set_channel_index_end,
    .get_channel_index_end   = get_channel_index_end,
    .set_channel_index_total = set_channel_index_total,
    .get_channel_index_total = get_channel_index_total,
    .clear_off_time          = clear_off_time,
};

struct Ex10OffTimeHelpers const* get_ex10_off_time_helpers(void)
{
    return &ex10_off_time_helpers;
}
