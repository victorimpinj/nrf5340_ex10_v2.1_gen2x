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

#include "ex10_regulatory/ex10_regulatory_region.h"

#include <stdlib.h>
#include <string.h>


/**
 * A region to use signifying an error to the user.
 */
static const struct Ex10Region error_region = {
    .region_id = REGION_NOT_DEFINED,
    .regulatory_timers =
        {
            .nominal_ms          = 0,
            .extended_ms         = 0,
            .regulatory_ms       = 0,
            .off_same_channel_ms = 0,
        },
    .regulatory_channels =
        {
            .start_freq_khz = 0,
            .spacing_khz    = 0,
            .count          = 0,
            .usable_count   = 0u,
            .usable         = NULL,
            .random_hop     = true,
        },
    .pll_divider    = 0,
    .rf_filter      = UNDEFINED_BAND,
    .max_power_cdbm = 0,
};

static const struct Ex10Region* region_ptr = &error_region;

static void set_region(struct Ex10Region const* region_to_use)
{
    region_ptr = (region_to_use == NULL) ? &error_region : region_to_use;
}

static struct Ex10Region const* get_region(void)
{
    return region_ptr;
}

static void get_regulatory_timers(uint16_t                     channel_index,
                                  uint32_t                     time_ms,
                                  struct Ex10RegulatoryTimers* timers)
{
    (void)channel_index;
    (void)time_ms;
    *timers = region_ptr->regulatory_timers;
}

static void regulatory_timer_set_start(uint16_t channel_index, uint32_t time_ms)
{
    (void)channel_index;
    (void)time_ms;
}

static void regulatory_timer_set_end(uint16_t channel_index, uint32_t time_ms)
{
    (void)channel_index;
    (void)time_ms;
}

static void regulatory_timer_clear(void) {}

static struct Ex10RegionRegulatory const ex10_error_regulatory = {
    .set_region                 = set_region,
    .get_region                 = get_region,
    .get_regulatory_timers      = get_regulatory_timers,
    .regulatory_timer_set_start = regulatory_timer_set_start,
    .regulatory_timer_set_end   = regulatory_timer_set_end,
    .regulatory_timer_clear     = regulatory_timer_clear,
};

struct Ex10RegionRegulatory const* get_ex10_error_regulatory(void)
{
    return &ex10_error_regulatory;
}
