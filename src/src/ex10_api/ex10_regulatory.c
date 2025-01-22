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

#include "ex10_api/ex10_regulatory.h"
#include "ex10_api/ex10_print.h"
#include "ex10_regulatory/ex10_regulatory_region.h"

#include <stdlib.h>
#include <string.h>


// IPJ_autogen | gen_c_regions_get_ex10_region_regulatory_list_c {
static struct Ex10RegionRegulatory const* get_region_layer(
    enum Ex10RegionId region_id)
{
    switch (region_id)
    {
        case REGION_FCC:
            return get_ex10_fcc_regulatory();
        case REGION_HK:
            return get_ex10_hk_regulatory();
        case REGION_TAIWAN:
            return get_ex10_taiwan_regulatory();
        case REGION_ETSI_LOWER:
            return get_ex10_etsi_lower_regulatory();
        case REGION_KOREA:
            return get_ex10_korea_regulatory();
        case REGION_MALAYSIA:
            return get_ex10_malaysia_regulatory();
        case REGION_CHINA:
            return get_ex10_china_regulatory();
        case REGION_SOUTH_AFRICA:
            return get_ex10_south_africa_regulatory();
        case REGION_BRAZIL:
            return get_ex10_brazil_regulatory();
        case REGION_THAILAND:
            return get_ex10_thailand_regulatory();
        case REGION_SINGAPORE:
            return get_ex10_singapore_regulatory();
        case REGION_AUSTRALIA:
            return get_ex10_australia_regulatory();
        case REGION_INDIA:
            return get_ex10_india_regulatory();
        case REGION_URUGUAY:
            return get_ex10_uruguay_regulatory();
        case REGION_VIETNAM:
            return get_ex10_vietnam_regulatory();
        case REGION_ISRAEL:
            return get_ex10_israel_regulatory();
        case REGION_PHILIPPINES:
            return get_ex10_philippines_regulatory();
        case REGION_INDONESIA:
            return get_ex10_indonesia_regulatory();
        case REGION_NEW_ZEALAND:
            return get_ex10_new_zealand_regulatory();
        case REGION_JAPAN_916_921_MHZ:
            return get_ex10_japan_916_921_mhz_regulatory();
        case REGION_PERU:
            return get_ex10_peru_regulatory();
        case REGION_ETSI_UPPER:
            return get_ex10_etsi_upper_regulatory();
        case REGION_RUSSIA:
            return get_ex10_russia_regulatory();
        case REGION_NOT_DEFINED:
        case REGION_CUSTOM:
        default:
            break;
    }
    return get_ex10_error_regulatory();
}
// IPJ_autogen }

static void set_region(enum Ex10RegionId        region_id,
                       struct Ex10Region const* region_to_use)
{
    struct Ex10RegionRegulatory const* region_reg = get_region_layer(region_id);
    region_reg->set_region(region_to_use);
}

static struct Ex10Region const* get_region(enum Ex10RegionId region_id)
{
    struct Ex10RegionRegulatory const* region_reg = get_region_layer(region_id);
    return region_reg->get_region();
}

static void get_regulatory_timers(enum Ex10RegionId            region_id,
                                  channel_index_t              channel,
                                  uint32_t                     time_ms,
                                  struct Ex10RegulatoryTimers* timers)
{
    struct Ex10RegionRegulatory const* region_reg = get_region_layer(region_id);
    region_reg->get_regulatory_timers(channel, time_ms, timers);
}

static void regulatory_timer_set_start(enum Ex10RegionId region_id,
                                       channel_index_t   channel,
                                       uint32_t          time_ms)
{
    struct Ex10RegionRegulatory const* region_reg = get_region_layer(region_id);
    region_reg->regulatory_timer_set_start(channel, time_ms);
}

static void regulatory_timer_set_end(enum Ex10RegionId region_id,
                                     channel_index_t   channel,
                                     uint32_t          time_ms)
{
    struct Ex10RegionRegulatory const* region_reg = get_region_layer(region_id);
    region_reg->regulatory_timer_set_end(channel, time_ms);
}

static void regulatory_timer_clear(enum Ex10RegionId region_id)
{
    struct Ex10RegionRegulatory const* region_reg = get_region_layer(region_id);
    region_reg->regulatory_timer_clear();
}

static uint32_t calculate_channel_khz(enum Ex10RegionId region_id,
                                      channel_index_t   channel)
{
    struct Ex10Region const* region = get_region(region_id);
    return region->regulatory_channels.start_freq_khz +
           (channel - 1) * region->regulatory_channels.spacing_khz;
}

static channel_index_t calculate_channel_index(enum Ex10RegionId region_id,
                                               uint32_t          frequency_khz)
{
    struct Ex10Region const* region = get_region(region_id);

    uint32_t const freq_khz_offset =
        frequency_khz - region->regulatory_channels.start_freq_khz;
    uint32_t const channel_index =
        (freq_khz_offset / region->regulatory_channels.spacing_khz) + 1;
    return (channel_index_t)channel_index;
}

static struct Ex10Regulatory const ex10_regulatory = {
    .set_region                 = set_region,
    .get_region                 = get_region,
    .get_regulatory_timers      = get_regulatory_timers,
    .regulatory_timer_set_start = regulatory_timer_set_start,
    .regulatory_timer_set_end   = regulatory_timer_set_end,
    .regulatory_timer_clear     = regulatory_timer_clear,
    .calculate_channel_khz      = calculate_channel_khz,
    .calculate_channel_index    = calculate_channel_index,
};

struct Ex10Regulatory const* get_ex10_regulatory(void)
{
    return &ex10_regulatory;
}
