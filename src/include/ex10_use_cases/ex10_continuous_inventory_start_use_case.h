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

#include "ex10_api/application_register_definitions.h"
#include "ex10_api/event_fifo_packet_types.h"
#include "ex10_api/ex10_continuous_inventory_common.h"
#include "ex10_api/ex10_inventory.h"
#include "ex10_api/rf_mode_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @struct Ex10ContinuousInventoryStartUseCase
 * The continuous inventory start use case interface. This use case works wraps
 * the continuous inventory use case with a couple differences. It allows a more
 * extensive interface for different variables to be passed in. It also starts
 * continuous inventory and immediately returns. The normal continuous inventory
 * use case sits in a while loop within the use case until a stop condition is
 * met. This use case kick off inventory and returns assuming the user will deal
 * with the event fifo and any other interactiosn which may be required.
 */
struct Ex10ContinuousInventoryStartUseCase
{
    /**
     * Initialize the Ex10ContinuousInventoryStartUseCase object.
     * This must be called after the Ex10 core has been initialized
     *
     */
    struct Ex10Result (*init)(void);

    /**
     * Release any resources used by the
     * Ex10ContinuousInventoryStartUseCase object.
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
     * Pass through the tag read extended packet enable to the continuous
     * inventory use case.
     *
     * @param enable_extended_packet If set to false, extended packets will be
     * disabled, and vice versa.
     */
    void (*enable_tag_read_extended_packet)(bool enable_extended_packet);

    /**
     * Used to return the reason that continuous inventory stopped.
     * @return The StopReason.
     */
    enum StopReason (*get_continuous_inventory_stop_reason)(void);

    /**
     * Run inventory rounds continuously until the specified
     * stop conditions are met. Returns to the user while using registered
     * interrupts to continue inventory
     *
     * @param params @see struct Ex10ContinuousInventoryUseCaseParameters
     * @return Info about any encountered errors.
     */
    struct Ex10Result (*continuous_inventory)(
        struct InventoryRoundControlFields*   inventory_config,
        struct InventoryRoundControl_2Fields* inventory_config_2,
        struct StopConditions const*          stop_conditions,
        uint8_t                               antenna,
        enum RfModes                          rf_mode,
        int16_t                               tx_power_cdbm,
        bool                                  send_selects,
        bool                                  dual_target);
};

struct Ex10ContinuousInventoryStartUseCase const*
    get_ex10_continuous_inventory_start_use_case(void);

#ifdef __cplusplus
}
#endif
