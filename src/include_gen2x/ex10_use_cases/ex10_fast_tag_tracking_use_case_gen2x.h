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

#include "ex10_api/bit_span.h"

#include "include_gen2x/ex10_api/application_register_field_enums_gen2x.h"
#include "include_gen2x/ex10_api/application_registers_gen2x.h"
#include "include_gen2x/ex10_api/event_fifo_packet_types_gen2x.h"
#include "include_gen2x/ex10_api/rf_mode_definitions_gen2x.h"

#ifdef __cplusplus
extern "C" {
#endif

// bitfield is for 64k bits each to represent
// one of the 16-bit stored CRC values.
#define SIZE_OF_BITFIELD (64 * 1024)

enum FastTagTrackingInvState
{
    FullEpcMode = 0,
    FastInventoryMode,
};

/**
 *@struct This is the static data for a custom event fifo
 * message used to communicate changing fast tag tracking modes to the
 * example/callback layer
 */
struct Ex10FastTagTrackingStateChangeGen2X
{
    uint8_t  fast_tag_tracking_state;
    uint8_t  packet_rfu1;
    uint16_t packet_rfu2;
};

/**
 * @struct These are the use case parameters used to run the fast tag tracking
 * inventory and they are generally passed on to the inventory module
 * to setup the inventory rounds.
 *
 * @param known_stored_crcs is a BitSpan struct that is assumed to be the size
 *                          SIZE_OF_BITFIELD allocated by the example
 *                          which is used by the use case for classifying
 *                          the tags read as known (not new) tags
 * @param build_stored_crc  This is a bool that if set true the use case
 *                          will clear the known_stored_crc bitfield and
 *                          run a full EPC inventory round to build the
 *                          known list
 * @param new_stored_crc    is a BitSpan struct that is assumed to be the size
 *                          SIZE_OF_BITFIELD to keep track tags that were read
 *                          but NOT in the known_stored_crc bitfield.  This
 *                          is used to filter out phantom (noise) tag reads that
 *                          are not real.  (the assumption is that a real new
 *                          tag will be read at least twice)
 * @param clear_new_stored_crc is a bool that if set to true the use case
 *                          will clear the new_stored_crc field when starting
 *                          the fast inventory round.. If false it will accept
 *                          any existing "new" tag flagged.
 */
struct Ex10FastTagTrackingUseCaseParametersGen2X
{
    uint8_t                      antenna;
    enum RfModesGen2X            rf_mode;
    int16_t                      tx_power_cdbm;
    uint8_t                      initial_q;
    uint8_t                      session;
    uint8_t                      select;
    struct StopConditions const* stop_conditions;
    struct BitSpan               known_stored_crcs;
    bool                         build_stored_crcs;
    struct BitSpan               new_stored_crcs;
    bool                         clear_new_stored_crcs;
};

/**
 * @struct Ex10FastTagTrackingUseCaseGen2X
 * The fast tag tracking use case interface.
 */
struct Ex10FastTagTrackingUseCaseGen2X
{
    /**
     * Initialize the Ex10FastTagTrackingUseCaseGen2X object.
     * This must be called after the Ex10 core has been initialized
     * @return struct Ex10Result
     *         Indicates whether the function call passed or failed.
     */
    struct Ex10Result (*init)(void);

    /**
     * Release any resources used by the
     * Ex10FastTagTrackingUseCaseGen2X object.
     * @return struct Ex10Result
     *         Indicates whether the function call passed or failed.
     */
    struct Ex10Result (*deinit)(void);

    /**
     * Register a Callback to subscribe to Ex10 event FIFO
     * packets during the set of inventory sequences.
     *
     * @note This function must be called before calling
     *       continuous_inventory().
     *
     * @param packet_subscriber_callback
     * A pointer to a function that will be called back during the inventory
     * sequence with pointers to packet data. If the callback Ex10Result
     * indicates an error, then the inventory sequence will terminate.
     * If the Ex10Result.customer field is set it will cause the inventory
     * sequence to stop without error.
     */
    void (*register_packet_subscriber_callback)(
        void (*packet_subscriber_callback)(struct EventFifoPacket const*,
                                           struct Ex10Result*));

    /**
     * By default only the TagReadExtended, ContinuousInventorySummary
     * and Custom packet types are sent to the packet subscriber.
     *
     * @param enable_filter If set to false, all packets will be sent to the
     * packet subscriber. If true, the normal behavior of only sending the
     * TagRead and InventoryRoundSummary packets to the subscriber is enforced.
     */
    void (*enable_packet_filter)(bool enable_filter);

    /**
     * Return the reason why the fast tag tracking inventory stopped.
     * @return The StopReason.
     */
    enum StopReason (*get_fast_tag_tracking_stop_reason)(void);

    /**
     * Force the fast tag tracking inventory to stop a fast inventory round and
     * do a full EPC inventory round (and to rebuild the bitfields)
     */
    void (*force_full_round)(void);

    /**
     * Command the fast tag tracking inventory to stop and return to the
     * calling context
     */
    void (*stop_inventory)(void);

    /**
     * A helper function that will clear a bitfield
     */
    void (*clear_bitfield)(struct BitSpan bit_span);
    /**
     * Run inventory rounds continuously until the specified
     * stop conditions are met.
     *
     * @param params @see struct Ex10FastTagTrackingUseCaseParametersGen2X
     * @return Info about any encountered errors.
     */
    struct Ex10Result (*fast_tag_tracking_inventory)(
        struct Ex10FastTagTrackingUseCaseParametersGen2X* params);
};

struct Ex10FastTagTrackingUseCaseGen2X const*
    get_ex10_fast_tag_tracking_use_case_gen2x(void);

#ifdef __cplusplus
}
#endif
