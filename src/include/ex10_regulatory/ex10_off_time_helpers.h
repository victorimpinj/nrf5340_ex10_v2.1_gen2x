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

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ex10_api/ex10_regulatory.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct Ex10OffTimeHelpers
 * Interface to assist in working with off times for regions that require it.
 */
struct Ex10OffTimeHelpers
{
    /**
     * Reterns whether or not the time elapsed is greater than the off time for
     * the passed regulatory timers.
     *
     * @param time_elapsed_ms The time to compare against the off time.
     * @param timers          The regulatory timers for the current region of
     *                        interest. The off time is pulled from here.
     *
     * @return Returns true if time_elapsed is >= the off time.
     */
    bool (*get_off_time_observed)(uint32_t time_elapsed_ms,
                                  struct Ex10RegulatoryTimers const* timers);

    /**
     * Sets the last start time in ms for the supplied channel index.
     *
     * @param channel_index The channel index to use for the regulatory timers.
     * @param time_ms       The current time, which can be used to adjust the
     *                      regulatory timers returned based on the previous
     *                      start and end calls.
     */
    void (*set_channel_index_start)(channel_index_t channel_index,
                                    uint32_t        time_ms,
                                    struct Ex10RegulatoryTimers const* timers);

    /**
     * Gets the last start time in ms for the supplied channel index.
     *
     * @param channel_index The channel index to use for the regulatory timers.
     *
     * @return The time in ms of the last start on the passed channel index.
     */
    uint32_t (*get_channel_index_start)(channel_index_t channel_index);

    /**
     * Sets the last end time in ms for the supplied channel index.
     *
     * @param channel_index The channel index to use for the regulatory timers.
     * @param time_ms       The current time, which can be used to adjust the
     *                      regulatory timers returned based on the previous
     *                      start and end calls.
     */
    void (*set_channel_index_end)(channel_index_t channel_index,
                                  uint32_t        time_ms,
                                  struct Ex10RegulatoryTimers const* timers);

    /**
     * Gets the last end time in ms for the supplied channel index.
     *
     * @param channel_index The channel index to use for the regulatory timers.
     *
     * @return The time in ms of the last end on the passed channel index.
     */
    uint32_t (*get_channel_index_end)(channel_index_t channel_index);

    /**
     * Sets the total time spent on the supplied channel index in ms.
     *
     * @param channel_index The channel index to use for the regulatory timers.
     * @param time_ms       The current time, which can be used to adjust the
     *                      regulatory timers returned based on the previous
     *                      start and end calls.
     */
    void (*set_channel_index_total)(channel_index_t channel_index,
                                    uint32_t        time_ms,
                                    struct Ex10RegulatoryTimers const* timers);

    /**
     * Gets the total time spent on the supplied channel index in ms.
     *
     * @param channel_index The channel index to use for the regulatory timers.
     *
     * @return The total time in ms spent on the passed channel index.
     */
    uint32_t (*get_channel_index_total)(channel_index_t channel_index);

    /**
     * Clears the start, end, and total time arrays in the off time helpers.
     */
    void (*clear_off_time)(void);
};

struct Ex10OffTimeHelpers const* get_ex10_off_time_helpers(void);

#ifdef __cplusplus
}
#endif
