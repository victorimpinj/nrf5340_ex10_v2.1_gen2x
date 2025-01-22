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

#include "ex10_api/application_register_definitions.h"
#include "ex10_api/event_fifo_packet_types.h"
#include "ex10_api/ex10_continuous_inventory_common.h"
#include "ex10_api/ex10_inventory.h"
#include "ex10_api/rf_mode_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif


struct Ex10PowerSweepUseCaseParameters
{
    uint8_t                      antenna;
    enum RfModes                 rf_mode;
    uint8_t                      initial_q;
    uint8_t                      session;
    uint8_t                      target;
    uint8_t                      select;
    bool                         send_selects;
    struct StopConditions const* stop_conditions;
    bool                         dual_target;
};

/**
 * @struct Ex10ContinuousInventoryPowerSweepUseCase
 * The continuous inventory use case interface.
 */
struct Ex10ContinuousInventoryPowerSweepUseCase
{
    /**
     * Initialize the Ex10ContinuousInventoryPowerSweepUseCase object.
     * This must be called after the Ex10 core has been initialized
     *
     */
    struct Ex10Result (*init)(void);

    /**
     * Release any resources used by the
     * Ex10ContinuousInventoryPowerSweepUseCase object.
     * @return struct Ex10Result
     *         Indicates whether the function call passed or failed.
     */
    struct Ex10Result (*deinit)(void);

    /**
     * Register a Callback to subscribe to TagRead and InventoryRoundSummary
     * packets during the set of inventory sequences.
     *
     * @note This function must be called before calling
     *       continuous_inventory().
     *
     * @param packet_subscriber_callback
     * A pointer to a function that will be called back during the inventory
     * sequence with pointers to packet data. If the callback Ex10Result
     * indicates an error, then the inventory sequence will terminate.
     */
    void (*register_packet_subscriber_callback)(
        void (*packet_subscriber_callback)(struct EventFifoPacket const*,
                                           struct Ex10Result*));

    /**
     * By default only the TagRead and InventoryRoundSummary packet types are
     * sent to the packet subscriber.
     *
     * @param enable_filter If set to false, all packets will be sent to the
     * packet subscriber. If true, the normal behavior of only sending the
     * TagRead and InventoryRoundSummary packets to the subscriber is enforced.
     */
    void (*enable_packet_filter)(bool enable_filter);

    /**
     * By default the continuous inventory use case does not do
     * auto access actions when it singulates a tag.  If true the
     * the preconfigured auto access commands will be executed on
     * each tag that is singulated, if false they will not.  The
     * application must setup the auto access commands before
     * starting the continuous inventory round.
     *
     * @param enable If set to true, auto access is enabled
     *               If set to false, auto access is disabled
     */
    void (*enable_auto_access)(bool enable);

    /**
     * This enables the abort_on_fail flag for the enable_auto_access
     * function. This will cause the auto access sequence
     * to abort an auto access sequence on a sigulated tag if one of the
     * access transactions fails.  In the event of a failure, the inventory
     * round will continue to the next tag.  If this is false, all
     * configured auto access commands will be performed.
     *
     * The applicaiton callback is responsible for detecting the failure
     * and adjusting its expectations about what Gen2Transaction packets
     * will be put into the event fifo.
     */
    void (*enable_abort_on_fail)(bool enable);

    /**
     * Return the reason that continuous inventory stopped.
     * @return The StopReason.
     */
    enum StopReason (*get_continuous_inventory_stop_reason)(void);

    /**
     * Run inventory rounds continuously until the specified
     * stop conditions are met.
     *
     * @param params @see struct Ex10ContinuousInventoryUseCaseParameters
     * @return Info about any encountered errors.
     */
    struct Ex10Result (*continuous_inventory)(
        struct Ex10PowerSweepUseCaseParameters* params);

    /**
     * Sets the power levels to use during the power sweep. This list of
     * powers is iterated over after the end of each inventory round.
     *
     * @param powers_cdbm A pointer to an array of power levels.
     * @param num_powers  The number of power levels in the referenced array
     */
    void (*set_power_sweep_levels)(int16_t const* powers_cdbm,
                                   uint32_t       num_powers);
};

struct Ex10ContinuousInventoryPowerSweepUseCase const*
    get_ex10_continuous_inventory_power_sweep_use_case(void);

#ifdef __cplusplus
}
#endif
