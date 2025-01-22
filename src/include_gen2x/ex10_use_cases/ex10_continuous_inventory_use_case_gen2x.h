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

#pragma once

#include "ex10_use_cases/ex10_continuous_inventory_use_case.h"

#include "include_gen2x/ex10_api/application_register_field_enums_gen2x.h"
#include "include_gen2x/ex10_api/application_registers_gen2x.h"
#include "include_gen2x/ex10_api/event_fifo_packet_types_gen2x.h"
#include "include_gen2x/ex10_api/rf_mode_definitions_gen2x.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct These are the use case parameters used to run the inventory
 * and they are generally passed on to the inventory module to setup
 * the inventory rounds.  These also inform the wrapper how to setup
 * the ScanCommandControl and ScanIDCommandControl registers for
 * Gen2 extensions modes (they will be ignored by the firmware if
 * the RF Mode is not a Gen2 extension mode)
 */
struct Ex10ContinuousInventoryUseCaseParametersGen2X
{
    uint8_t                                antenna;
    enum RfModesGen2X                      rf_mode;
    int16_t                                tx_power_cdbm;
    uint8_t                                initial_q;
    uint8_t                                session;
    uint8_t                                target;
    uint8_t                                select;
    bool                                   send_selects;
    struct StopConditions const*           stop_conditions;
    bool                                   dual_target;
    bool                                   crypto;
    enum ScanCommandControlCodeGen2X       code;
    enum ScanCommandControlCrGen2X         cr;
    enum ScanCommandControlProtectionGen2X cr_protection;
    enum ScanCommandControlIdGen2X         id;
    bool                                   scan_id_enable;
    enum ScanIdCommandControlAppSizeGen2X  app_size;
    uint32_t                               app_id;
};

/**
 * @struct Ex10ContinuousInventoryUseCaseGen2X
 * The continuous inventory use case interface for Gen2X.
 */
struct Ex10ContinuousInventoryUseCaseGen2X
{
    /**
     * Initialize the Ex10ContinuousInventoryUseCaseGen2X object.
     * This must be called after the Ex10 core has been initialized
     * @return struct Ex10Result
     *         Indicates whether the function call passed or failed.
     */
    struct Ex10Result (*init)(void);

    /**
     * Release any resources used by the
     * Ex10ContinuousInventoryUseCaseGen2X object.
     * @return struct Ex10Result
     *         Indicates whether the function call passed or failed.
     */
    struct Ex10Result (*deinit)(void);

    /**
     * Register a Callback to subscribe to EventFifo packets during the set
     * of inventory sequences.
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
     * By default only the TagRead, TagReadExtended, and InventoryRoundSummary
     * packet types are sent to the packet subscriber.
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
     * each tag that is sinulated; If false they will not.  The
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
     * @param params @see struct Ex10ContinuousInventoryUseCaseParametersGen2X
     * @return Info about any encountered errors.
     */
    struct Ex10Result (*continuous_inventory)(
        struct Ex10ContinuousInventoryUseCaseParametersGen2X* params);
};

struct Ex10ContinuousInventoryUseCaseGen2X const*
    get_ex10_continuous_inventory_use_case_gen2x(void);

#ifdef __cplusplus
}
#endif
