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

#include "board/ex10_osal.h"
#include "ex10_api/ex10_macros.h"
#include "ex10_regulatory/ex10_off_time_helpers.h"
#include "ex10_regulatory/ex10_regulatory_region.h"


// clang-format off
// IPJ_autogen | gen_c_regions_ex10_regulatory | region_name:ETSI_LOWER {
static channel_index_t const etsi_lower_usable_channels[] = {4, 7, 10, 13};

static const struct Ex10Region region = {
    .region_id = REGION_ETSI_LOWER,
    .regulatory_timers =
        {
            .nominal_ms          = 3800,
            .extended_ms         = 3980,
            .regulatory_ms       = 4000,
            .off_same_channel_ms = 100,
        },
    .regulatory_channels =
        {
            .start_freq_khz = 865100,
            .spacing_khz    = 200,
            .count          = 4,
            .usable         = etsi_lower_usable_channels,
            .usable_count   =
                (channel_size_t)ARRAY_SIZE(etsi_lower_usable_channels),
            .random_hop     = false,
        },
    .pll_divider    = 120,
    .rf_filter      = LOWER_BAND,
    .max_power_cdbm = 3000,
};
// IPJ_autogen }
// clang-format on

static const struct Ex10Region* region_ptr = &region;

static void set_region(struct Ex10Region const* region_to_use)
{
    region_ptr = (region_to_use == NULL) ? &region : region_to_use;
}

static struct Ex10Region const* get_region(void)
{
    return region_ptr;
}

static void get_regulatory_timers(channel_index_t              channel_index,
                                  uint32_t                     time_ms,
                                  struct Ex10RegulatoryTimers* timers)
{
    const uint16_t default_off_time =
        region_ptr->regulatory_timers.off_same_channel_ms;
    *timers = region_ptr->regulatory_timers;

    struct Ex10OffTimeHelpers const* oth = get_ex10_off_time_helpers();

    // If start is greater than end, CW is currently on
    if (oth->get_channel_index_start(channel_index) >
        oth->get_channel_index_end(channel_index))
    {
        // If CW is on, we have not exceeded regulatory and conclude no off_time
        // needed.
        timers->off_same_channel_ms = 0;
    }
    else
    {
        // If CW is off, check whether total channel time has exceeded
        // regulatory and if off time has been observed. Note we check against
        // nominal, not regulatory for margin.
        const uint32_t time_since_off =
            time_ms - oth->get_channel_index_end(channel_index);
        const bool off_observed =
            oth->get_off_time_observed(time_since_off, timers);
        uint16_t const total_time =
            (uint16_t)oth->get_channel_index_total(channel_index);
        const bool total_exceeded = total_time >= timers->nominal_ms;

        if (off_observed)
        {
            // No off-time and all other timers are reset
            timers->off_same_channel_ms = 0;
        }
        else
        {
            if (total_exceeded == true)
            {
                // Need to observe off-time. Other timers are reset
                timers->off_same_channel_ms =
                    default_off_time - (uint16_t)time_since_off;
            }
            else
            {
                // Off-time not needed, but other timers need adjusted
                timers->off_same_channel_ms = 0;

                timers->nominal_ms =
                    (timers->nominal_ms > total_time)
                        ? (timers->nominal_ms - (uint16_t)total_time)
                        : 1;
                timers->extended_ms =
                    (timers->extended_ms > total_time)
                        ? (timers->extended_ms - (uint16_t)total_time)
                        : 1;
                timers->regulatory_ms =
                    (timers->regulatory_ms > total_time)
                        ? (timers->regulatory_ms - (uint16_t)total_time)
                        : 1;
            }
        }
    }
}

static void regulatory_timer_set_start(channel_index_t channel_index,
                                       uint32_t        time_ms)
{
    get_ex10_off_time_helpers()->set_channel_index_start(
        channel_index, time_ms, &region_ptr->regulatory_timers);
}

static void regulatory_timer_set_end(channel_index_t channel_index,
                                     uint32_t        time_ms)
{
    get_ex10_off_time_helpers()->set_channel_index_end(
        channel_index, time_ms, &region_ptr->regulatory_timers);
}

static void regulatory_timer_clear(void)
{
    get_ex10_off_time_helpers()->clear_off_time();
}

static struct Ex10RegionRegulatory const ex10_default_regulatory = {
    .set_region                 = set_region,
    .get_region                 = get_region,
    .get_regulatory_timers      = get_regulatory_timers,
    .regulatory_timer_set_start = regulatory_timer_set_start,
    .regulatory_timer_set_end   = regulatory_timer_set_end,
    .regulatory_timer_clear     = regulatory_timer_clear,
};

struct Ex10RegionRegulatory const* get_ex10_etsi_lower_regulatory(void)
{
    return &ex10_default_regulatory;
}
