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

#include "ex10_api/ex10_activity_sequence.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct ActivityCallbackInfo
 * When a callback occurs before or after an activity in the activity sequence,
 * data is passed to the handler. This contains all the info needed for the
 * callback. This can grow or shrink based on the needs of the algorithms.
 */
struct ActivityCallbackInfo
{
    // NOTE: const * const. You can change the sequence on the fly, but it
    // should be done through the api
    struct Ex10ActivitySequence const* const activity_sequence;
    size_t                                   sequence_iter;
    bool                                     first_activity;
};

/**
 * @struct Ex10ActivitySequenceUseCase
 * The inventory sequence use case interface.
 */
struct Ex10ActivitySequenceUseCase
{
    /**
     * Initialize the Ex10ActivitySequenceUseCase object.
     * This must be called after the Ex10 core has been initialized.
     */
    struct Ex10Result (*init)(void);

    /**
     * Release any resources used by the Ex10ActivitySequenceUseCase
     * object.
     *
     * @return struct Ex10Result
     *         Indicates whether the function call passed or failed.
     */
    struct Ex10Result (*deinit)(void);

    /**
     * Register a Callback to subscribe to TagRead and InventoryRoundSummary
     * packets during the set of inventory sequences.
     *
     * @note To be used during a sequence, the function must be registered
     *       before calling run_activity_sequence(), aka before sequence start.
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
     * Registers a callback to take place before each activity in the activty
     * sequence. This allows for special actions to prepare for activities as
     * well as the ability to alter the sequence based on info from the previous
     * activities.
     *
     * @param ActivityCallbackInfo.activity_sequence The full sequence being
     * run.
     * @param ActivityCallbackInfo.sequence_iter     The iterator pointing to
     * the next activity in the sequence. The activity which just finished is
     * the iterator minus 1.
     * @param ActivityCallbackInfo.first_activity    A simple bool for whether
     * or not the given activity was the very first run.
     */
    void (*register_pre_activity_callback)(
        void (*pre_activity_callback)(struct ActivityCallbackInfo*,
                                      struct Ex10Result*));

    /**
     * Registers a callback to take place after each activity in the activty
     * sequence. This allows for special actions to prepare for activities as
     * well as the ability to alter the sequence based on info from the previous
     * activities. Note that since the callback can alter the sequence iterator,
     * it is incremented before the callback. The callback should note that it
     * points to the next activity in the sequence.
     *
     * @param ActivityCallbackInfo.activity_sequence The full sequence being
     * run.
     * @param ActivityCallbackInfo.sequence_iter     The iterator pointing to
     * the next activity in the sequence. The activity which just finished is
     * the iterator minus 1.
     * @param ActivityCallbackInfo.first_activity    A simple bool for whether
     * or not the given activity was the very first run.
     */
    void (*register_post_activity_callback)(
        void (*post_activity_callback)(struct ActivityCallbackInfo*,
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
     * Get the inventory sequence that is actively in use.
     *
     * @return struct Ex10ActivitySequence const*
     * A pointer to the inventory sequence that was passed in to the
     * run_activity_sequence() function.
     */
    struct Ex10ActivitySequence const* (*get_activity_sequence)(void);

    /**
     * Get the inventory round that is active within the published packets
     * context.
     *
     * @return struct InventoryRoundConfigBasic const*
     * A pointer to the inventory round that is active within the
     * published packets context.
     */
    struct InventoryRoundConfigBasic const* (*get_inventory_round)(void);

    /**
     * Allows setting of the activity sequence into the use case without running
     * the sequence.
     *
     * @param activity_sequence
     *  A pointer to a filled in struct Ex10ActivitySequence, containing a
     *  valid array of struct InventoryRoundConfigBasic nodes.
     *
     * @return struct Ex10Result An indication of whether a run-time
     *         error has occurred during the initial sequencing of inventories.
     */
    struct Ex10Result (*set_activity_sequence)(
        struct Ex10ActivitySequence const* activity_sequence);

    /**
     * Run a sequence of inventory rounds specified by a container of
     * struct InventoryRoundConfig nodes.
     *
     * If a packet subscriber is registered with the
     * Ex10ActivitySequencePowerSweepUseCase using the
     * register_packet_subscriber_callback() function, then this function will
     * block until the inventory sequence completes. If no packet subscriber
     * is registered, then this function returns once the inventory has started.
     *
     * @param activity_sequence
     *  A pointer to a filled in struct Ex10ActivitySequence, containing a
     *  valid array of struct InventoryRoundConfigBasic nodes.
     *
     * @return struct Ex10Result An indication of whether a run-time
     *         error has occurred during the initial sequencing of inventories.
     */
    struct Ex10Result (*run_activity_sequence)(
        struct Ex10ActivitySequence const* activity_sequence);

    /**
     * Gets the timeout being used for the use case sequence.
     */
    uint32_t (*get_sequence_timeout_us)(void);

    /**
     * Sets the timeout to wait for the use case sequence to finish.
     * If the time is 0, the timeout will be ignored.
     */
    void (*set_sequence_timeout_us)(uint32_t timeout_us);
};

struct Ex10ActivitySequenceUseCase const* get_ex10_activity_sequence_use_case(
    void);

#ifdef __cplusplus
}
#endif
