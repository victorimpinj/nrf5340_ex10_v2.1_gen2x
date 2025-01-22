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

#include "ex10_rx_baseband_filter_gen2x.h"

static void set_drm_status_dummy(enum DrmStatus drm_enable)
{
    (void)drm_enable;
}

static enum DrmStatus get_drm_status_dummy(void)
{
    return (enum DrmStatus)0u;
}

static bool rf_mode_is_drm(enum RfModes rf_mode)
{
    const enum DrmStatus drm_status =
        get_ex10_rx_baseband_filter()->get_drm_status();
    if (drm_status == DrmStatusAuto)
    {
        // IPJ_autogen | board_drm_modes_list | visibility:impinj_gen2x {
        // This list of modes depends on the nature of the DRM filter on the
        // board. For custom board designs, the DRM modes list may need to be
        // manually modified.
        const uint16_t drm_modes_gen2x[] = {
            4141,
            4146,
            4241,
            4244,
            4342,
            4343,
        };
        // IPJ_autogen }

        for (size_t idx = 0;
             idx < sizeof(drm_modes_gen2x) / sizeof(drm_modes_gen2x[0]);
             idx++)
        {
            if (rf_mode == drm_modes_gen2x[idx])
            {
                // DRM on
                return true;
            }
        }
    }

    return get_ex10_rx_baseband_filter()->rf_mode_is_drm(rf_mode);
}

static enum BasebandFilterType choose_rx_baseband_filter(enum RfModes rf_mode)
{
    return rf_mode_is_drm(rf_mode) ? BasebandFilterBandpass
                                   : BasebandFilterHighpass;
}

static struct Ex10RxBasebandFilter ex10_baseband_filter_gen2x = {
    .set_drm_status            = set_drm_status_dummy,
    .get_drm_status            = get_drm_status_dummy,
    .rf_mode_is_drm            = rf_mode_is_drm,
    .choose_rx_baseband_filter = choose_rx_baseband_filter,
};

struct Ex10RxBasebandFilter const* get_ex10_rx_baseband_filter_gen2x(void)
{
    struct Ex10RxBasebandFilter const* filter = get_ex10_rx_baseband_filter();
    struct Ex10RxBasebandFilter* filter_gen2x = &ex10_baseband_filter_gen2x;

    filter_gen2x->set_drm_status = filter->set_drm_status;
    filter_gen2x->get_drm_status = filter->get_drm_status;

    return &ex10_baseband_filter_gen2x;
}
