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


struct Ex10ContinuousInventoryUseCaseParameters
{
    uint8_t                      antenna;
    enum RfModes                 rf_mode;
    int16_t                      tx_power_cdbm;
    uint8_t                      initial_q;
    uint8_t                      session;
    uint8_t                      target;
    uint8_t                      select;
    bool                         send_selects;
    struct StopConditions const* stop_conditions;
    bool                         dual_target;
};

/**
 * @struct Ex10ContinuousInventoryUseCase
 * The continuous inventory use case interface.
 */
struct Ex10ContinuousInventoryUseCase
{
    /**
     * Initialize the Ex10ContinuousInventoryUseCase object.
     * This must be called after the Ex10 core has been initialized
     *
     */
    struct Ex10Result (*init)(void);

    /**
     * Release any resources used by the
     * Ex10ContinuousInventoryUseCase object.
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
     * auto access actions when it singulates a tag.  If true, then
     * the preconfigured auto access commands will be executed on
     * each tag that is singulated; if false, they will not.  The
     * application must setup the auto access commands before
     * starting the continuous inventory round.
     *
     * @param enable If set to true, auto access is enabled
     *               If set to false, auto access is disabled
     */
    void (*enable_auto_access)(bool enable);

    /**
     * This enables the abort_on_fail flag for the auto access sequence
     * that can be enabled above.  This will cause the auto access sequence
     * to abort an auto access sequence on a sigulated tag if one of the
     * access transactions fails.  In the event of a failure, the inventory
     * round will continue to the next tag.  If this is false, all
     * configured auto access commands will be performed.
     *
     * The application callback is responsible for detecting the failure
     * and to adjust its expectations about what Gen2Transaction packets
     * will be put into the event fifo.
     */
    void (*enable_abort_on_fail)(bool enable);

    /**
     * This enables the fast_id flag for for the inventory op. When
     * enabled, the op will send a fast ID select before beginning
     * inventory.  Sending the fast ID select is independent from
     * the send_selects setting in the parameters passed to the
     * continuous_inventory() function.
     */
    void (*enable_fast_id)(bool enable);

    /**
     * This enables the tag_focus flag for for the inventory op. When
     * enabled, the op will send a tag focus select before beginning
     * inventory.  Sending the tag focus select is independent from
     * the send_selects setting in the parameters passed to the
     * continuous_inventory() function.
     */
    void (*enable_tag_focus)(bool enable);

    /**
     * This enables the use_tag_read_extended flag for the inventory op.
     * When enabled, TagRead event FIFO packet reports will be replaced
     * by TagReadExtended event FIFO packets.
     */
    void (*enable_tag_read_extended_packet)(bool enable);


    /**
     * Used to return the reason that continuous inventory stopped.
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
        struct Ex10ContinuousInventoryUseCaseParameters* params);

    /**
     * Starts the timer tracking how long continuous inventory has been going
     * on. This happens by default when the continuous inventory starts, but can
     * be used to reset time if needed.
     */
    void (*set_inventory_timer_start)(void);

    /**
     * Sets the parameters to be used during continuous inventory.
     *
     * @param inventory_config   @see struct InventoryRoundControlFields
     * @param inventory_config_2 @see struct InventoryRoundControl_2Fields
     * @param stop_conditions    The reasons to end running continuous
     * inventory.
     * @param antenna            The antenna to use.
     * @param rf_mode            The RF mode to use.
     * @param tx_power_cdbm      The transmitter power, in centi-dB.
     * @param send_selects       When set to true the select op is run.
     *                           The user should have already updated the
     *                           needed info in the Gen2 Tx command buffer
     *                           and appropriate control registers.
     * @param dual_target        Whether or not to switch targets between each
     * inventory round.
     * @return                   Info about any encountered errors.
     */
    void (*set_use_case_parameters)(
        struct InventoryRoundControlFields const*   inventory_config,
        struct InventoryRoundControl_2Fields const* inventory_config_2,
        struct StopConditions const*                stop_conditions,
        uint8_t                                     antenna,
        enum RfModes                                rf_mode,
        int16_t                                     tx_power_cdbm,
        bool                                        send_selects,
        bool                                        dual_target);
};

struct Ex10ContinuousInventoryUseCase const*
    get_ex10_continuous_inventory_use_case(void);

#ifdef __cplusplus
}
#endif
