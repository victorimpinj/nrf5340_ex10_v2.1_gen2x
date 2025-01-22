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

#include "board/board_spec.h"
#include "board/ex10_osal.h"
#include "board/time_helpers.h"
#include "calibration.h"

#include "ex10_api/application_registers.h"
#include "ex10_api/event_fifo_packet_types.h"
#include "ex10_api/event_fifo_printer.h"
#include "ex10_api/event_packet_parser.h"
#include "ex10_api/ex10_active_region.h"
#include "ex10_api/ex10_boot_health.h"
#include "ex10_api/ex10_dynamic_power_ramp.h"
#include "ex10_api/ex10_event_fifo_queue.h"
#include "ex10_api/ex10_inventory.h"
#include "ex10_api/ex10_macros.h"
#include "ex10_api/ex10_result.h"
#include "ex10_api/ex10_rf_power.h"
#include "ex10_api/ex10_utils.h"
#include "ex10_api/fifo_buffer_list.h"

#include "ex10_modules/ex10_ramp_module_manager.h"
#include "ex10_use_cases/ex10_activity_sequence_use_case.h"

/**
 * @struct ActivitySequenceState
 * Progress through a requested sequence of activities.
 */
struct ActivitySequenceState
{
    /// Points to the client supplied inventory sequences struct.
    /// - Set when run_activity_sequence() is
    ///   called by the client.
    /// - Read by IRQ_N monitor thread context within fifo_data_handler()
    ///   call chain and the publish_packets() function.
    struct Ex10ActivitySequence const* activity_sequence;

    /// Iteration count through the activity_sequence.sequence_activities array.
    /// - Initialized to zero when run_activity_sequence() is
    ///   called.
    /// - Iterated up to, but not including, Ex10ActivitySequence.count
    ///   during iteration through the sequence of inventory rounds
    ///   within the fifo_data_handler() execution context;
    ///   i.e. The IRQ_N monitor thread.
    volatile size_t activity_iter;

    /// If true, publish all packets.
    /// If false, publish TagRead and InventoryRoundSummary packets.
    bool publish_all_packets;

    /// The callback to notify the subscriber of a new packet.
    /// Typically, this callback is set prior to calling
    /// run_activity_sequence().
    void (*packet_subscriber_callback)(struct EventFifoPacket const*,
                                       struct Ex10Result*);

    /// Registers a callback to take place before each activity in the activty
    /// sequence. This allows for special actions to prepare for activities as
    /// well as the ability to alter the sequence based on info from the
    /// other activities.
    void (*pre_activity_callback)(struct ActivityCallbackInfo*,
                                  struct Ex10Result*);

    /// Registers a callback to take place after each activity in the activty
    /// sequence. This allows for special actions to prepare for activities as
    /// well as the ability to alter the sequence based on info from the
    /// other activities.
    void (*post_activity_callback)(struct ActivityCallbackInfo*,
                                   struct Ex10Result*);

    // The current tx power as set by the sequence. Used for dynamic power
    // updates to move from current power to new power.
    int16_t tx_power_cdbm;

    // The current antenna state as set by the sequence. If CW is on and the
    // user wants to switch antenna this ensures we will ramp down and ramp up
    // on the new antenna.
    uint8_t antenna;

    /// Incremented when an inventory round is started and decremented when
    /// publish packets sees the FIFO packet. This is used to ensure publish
    /// packets does not return before all rounds have finished.
    volatile uint32_t inventory_round_pending;
};

static volatile struct ActivitySequenceState sequence_state;

/// Timeout for waiting for the sequence to complete. If 0, this has no effect.
static uint32_t sequence_timeout_us = 0;

/// A bool used as a mutex to ensure the the main context knows when the post
/// activity callback is in progress if in a different thread.
static volatile bool post_callback_lock = false;
/**
 * Do the ugly work of bounds and type checking and casting to convert the
 * sequence_state.activity_sequence void pointer into a validated
 * InventoryRoundConfigBasic pointer.
 *
 * @param iteration The inventory round iteration value.
 *
 * @return struct InventoryRoundConfigBasic const* A validated
 *         InventoryRoundConfigBasic pointer; NULL if the pointed
 *         to activity is invalid.
 */
static struct InventoryRoundConfigBasic const* get_basic_inventory_round_config(
    size_t iteration)
{
    if (sequence_state.activity_sequence == NULL)
    {
        return NULL;
    }
    if (sequence_state.activity_sequence->sequence_activities[iteration]
            .type_id != SEQUENCE_INVENTORY_ROUND_CONFIG)
    {
        return NULL;
    }
    if (iteration >= sequence_state.activity_sequence->count)
    {
        return NULL;
    }

    struct InventoryRoundConfigBasic const* inventory_basic_configs =
        (struct InventoryRoundConfigBasic const*)
            sequence_state.activity_sequence->sequence_activities[iteration]
                .config;

    return inventory_basic_configs;
}

static struct Ex10Result dynamic_power_change(int16_t  curr_tx_power_cdbm,
                                              int16_t  new_power_cdbm,
                                              uint16_t temperature_adc,
                                              uint32_t curr_frequency_khz)
{
    uint8_t agg_data[AGGREGATE_OP_BUFFER_REG_LENGTH];
    ex10_memzero(agg_data, sizeof(agg_data));
    struct ByteSpan agg_buffer = {.data = agg_data, .length = 0};

    bool const temp_comp_enabled =
        get_ex10_board_spec()->temperature_compensation_enabled(
            temperature_adc);

    struct PowerConfigs curr_power_configs =
        get_ex10_calibration()->get_power_control_params(
            curr_tx_power_cdbm,
            false,
            curr_frequency_khz,
            temperature_adc,
            temp_comp_enabled,
            get_ex10_active_region()->get_rf_filter());

    return get_ex10_dynamic_power_ramp()->update_power(&curr_power_configs,
                                                       &agg_buffer,
                                                       new_power_cdbm,
                                                       curr_frequency_khz,
                                                       temperature_adc);
}

/**
 * Updates the power based on the current and next power levels. If CW is
 * currently ramped, a dynamic power ramp will occur to change the power. If CW
 * is not currently ramped, then a normal power ramp will take place using only
 * the configs of the next_cw_configs passed parameters.
 *
 * @param next_cw_configs    The configs to use for updating or ramping CW.
 *
 * @return struct Ex10Result
 */
static struct Ex10Result update_power_from_sequence(
    struct SequenceCwConfig next_cw_configs)
{
    struct Ex10RfPower const*           ex10_rf_power = get_ex10_rf_power();
    struct Ex10RampModuleManager const* ramp_module_manager =
        get_ex10_ramp_module_manager();

    // Sets rf mode regardless of previous RF mode settings
    struct Ex10Result ex10_result =
        ex10_rf_power->set_rf_mode(next_cw_configs.rf_mode);
    if (ex10_result.error)
    {
        return ex10_result;
    }

    bool cw_is_on = ex10_rf_power->get_cw_is_on();
    if (cw_is_on)
    {
        // If the next antenna does not match the  previous antenna, we need to
        // ramp down so the next ramp can ramp back up with the new antenna.
        if (sequence_state.antenna != next_cw_configs.antenna)
        {
            ex10_result = get_ex10_rf_power()->stop_op_and_ramp_down();
            if (ex10_result.error == true)
            {
                return ex10_result;
            }
            cw_is_on = false;
        }
    }

    // If ramped up, execute a dynamic power ramp, otherwise just ramp to power
    // from CW off. If the power is the same, dynamic power ramp will do
    // nothing.
    if (cw_is_on)
    {
        // We do not read and update the temperature since CW is already on, but
        // we still may need temp comp to figure out the next power params
        uint16_t temperature_adc =
            ramp_module_manager->retrieve_adc_temperature();
        uint32_t curr_frequency_khz =
            get_ex10_active_region()->get_active_channel_khz();

        // Note we pass the current and next power since dynamic power brings us
        // from the current CW power to the next power
        ex10_result = dynamic_power_change(sequence_state.tx_power_cdbm,
                                           next_cw_configs.tx_power_cdbm,
                                           temperature_adc,
                                           curr_frequency_khz);
        if (ex10_result.error)
        {
            return ex10_result;
        }

        ramp_module_manager->store_pre_ramp_variables(next_cw_configs.antenna);
        ramp_module_manager->store_post_ramp_variables(
            next_cw_configs.tx_power_cdbm,
            get_ex10_active_region()->get_next_channel_khz());
    }
    else
    {
        // Update the channel time tracking before kicking off the
        // next inventory round. This will be used to update the
        // regulatory timers if the inventory call needs to ramp up
        // again.
        ex10_result = get_ex10_active_region()->update_channel_time_tracking();
        if (ex10_result.error)
        {
            return ex10_result;
        }

        uint16_t temperature_adc = 0;
        ex10_result =
            ex10_rf_power->measure_and_read_adc_temperature(&temperature_adc);
        if (ex10_result.error == false)
        {
            bool const temp_comp_enabled =
                get_ex10_board_spec()->temperature_compensation_enabled(
                    temperature_adc);

            struct PowerDroopCompensationFields const droop_comp_fields =
                ex10_rf_power->get_droop_compensation_defaults();

            struct CwConfig cw_config;
            ex10_result =
                ex10_rf_power->build_cw_configs(next_cw_configs.antenna,
                                                next_cw_configs.rf_mode,
                                                next_cw_configs.tx_power_cdbm,
                                                temperature_adc,
                                                temp_comp_enabled,
                                                &cw_config);
            if (ex10_result.error)
            {
                return ex10_result;
            }

            ramp_module_manager->store_pre_ramp_variables(
                next_cw_configs.antenna);
            ramp_module_manager->store_post_ramp_variables(
                next_cw_configs.tx_power_cdbm,
                get_ex10_active_region()->get_next_channel_khz());

            ex10_result = ex10_rf_power->cw_on(&cw_config.gpio,
                                               &cw_config.power,
                                               &cw_config.synth,
                                               &cw_config.timer,
                                               &droop_comp_fields);
        }
    }
    // Update the state if everything went as expected
    sequence_state.tx_power_cdbm = next_cw_configs.tx_power_cdbm;
    sequence_state.antenna       = next_cw_configs.antenna;

    return ex10_result;
}

/**
 * Should only be called when the passed iteration maps to an
 * InventoryRoundConfigBasic, aka an inventory round within the
 * activity sequence. This pulls information from that index and
 * start and inventory round. If CW is ramped down, it will be ramped up using
 * the associated parameters in
 * Ex10ActivitySequence.sequence_activities[index].config.
 *
 * @param iteration          The inventory round iteration value.
 *
 * @return struct Ex10Result
 */
static struct Ex10Result start_inventory_from_sequence(size_t iteration)
{
    struct InventoryRoundConfigBasic const* inventory_round_next =
        get_basic_inventory_round_config(iteration);

    if (inventory_round_next)
    {
        struct InventoryRoundControlFields inventory_config =
            inventory_round_next->inventory_config;

        struct InventoryRoundControl_2Fields inventory_config_2 =
            inventory_round_next->inventory_config_2;

        // Reset Q state variables since the inventory is complete.
        inventory_config_2.starting_min_q_count                       = 0u;
        inventory_config_2.starting_max_queries_since_valid_epc_count = 0u;

        // ------------- Power, Antenna, Mode updates -------------
        // Calling update power instead of relying on start inventory ensures
        // that the antenna, rf_mode, and power are updated even if CW is on.
        // start_inventory will not update antenna or power if CW is on.
        struct SequenceCwConfig const next_cw_configs = {
            .antenna       = inventory_round_next->antenna,
            .rf_mode       = inventory_round_next->rf_mode,
            .tx_power_cdbm = inventory_round_next->tx_power_cdbm,
        };
        struct Ex10Result ex10_result =
            update_power_from_sequence(next_cw_configs);
        if (ex10_result.error)
        {
            return ex10_result;
        }

        // ----------------- Start inventory ---------------------
        return get_ex10_inventory()->start_inventory(
            inventory_round_next->antenna,
            inventory_round_next->rf_mode,
            inventory_round_next->tx_power_cdbm,
            &inventory_config,
            &inventory_config_2,
            inventory_round_next->send_selects);
    }
    else
    {
        // Inventory sequencing complete; do nothing.
        return make_ex10_success();
    }
}

static struct Ex10Result perform_activity_action(
    size_t                          iteration,
    enum SequenceActivityConfigType activity_type)
{
    struct Ex10Result ex10_result;
    if (activity_type == SEQUENCE_INVENTORY_ROUND_CONFIG)
    {
        // Signify that a round is currently running so that we don't exit the
        // publish packets loop
        sequence_state.inventory_round_pending++;

        ex10_result = start_inventory_from_sequence(iteration);
    }
    else if (activity_type == SEQUENCE_CONFIG_UNKNOWN)
    {
        ex10_result =
            make_ex10_sdk_error(Ex10ModuleUseCase, Ex10SdkErrorBadParamValue);
    }
    else if (activity_type == SEQUENCE_SELECT_CONFIG)
    {
        struct SequenceSelectConfig const* next_select_configs =
            (struct SequenceSelectConfig const*)
                sequence_state.activity_sequence->sequence_activities[iteration]
                    .config;

        ex10_result = make_ex10_success();

        // Update the enable flags
        if (next_select_configs->enables)
        {
            size_t                                 cmd_index = 0;
            struct Ex10Gen2TxCommandManager const* g2tcm =
                get_ex10_gen2_tx_command_manager();
            g2tcm->write_select_enables(next_select_configs->enables,
                                        next_select_configs->enable_array_size,
                                        &cmd_index);
        }

        // Run the select op
        if (next_select_configs->run_op_flag)
        {
            struct Ex10Ops const* ops = get_ex10_ops();
            ex10_result               = ops->send_select();
            if (ex10_result.error)
            {
                return ex10_result;
            }
            ex10_result = ops->wait_op_completion();
        }
    }
    else if (activity_type == SEQUENCE_POWER_RAMP_CONFIG)
    {
        struct SequenceCwConfig const* next_cw_configs =
            (struct SequenceCwConfig const*)sequence_state.activity_sequence
                ->sequence_activities[iteration]
                .config;

        ex10_result = update_power_from_sequence(*next_cw_configs);
    }
    else if (activity_type == SEQUENCE_CW_OFF_CONFIG)
    {
        ex10_result = get_ex10_rf_power()->stop_op_and_ramp_down();
    }
    else
    {
        ex10_result =
            make_ex10_sdk_error(Ex10ModuleUseCase, Ex10SdkErrorBadParamValue);
    }
    return ex10_result;
}

static struct Ex10Result continue_activity_sequence(void)
{
    // Run through the next activity in the sequence until we come across
    // the next inventory round or finish all activities in the sequence.
    // This maintains control as it runs through selects and power updates
    // which happen between inventory rounds.
    while (sequence_state.activity_iter <
           sequence_state.activity_sequence->count)
    {
        // Ensure there is no op running.
        struct Ex10Result ex10_result = get_ex10_ops()->wait_op_completion();
        if (ex10_result.error)
        {
            return ex10_result;
        }

        /*
        ---------------------------------------------------------------
        Check if the user wants to do anything before the next activity
        ---------------------------------------------------------------
        */
        // NOTE: after this callback, the sequence or iteration may have
        // changed, which is valid
        if (sequence_state.pre_activity_callback != NULL)
        {
            struct ActivityCallbackInfo pre_activity_info = {
                .activity_sequence = sequence_state.activity_sequence,
                .sequence_iter     = sequence_state.activity_iter,
                .first_activity    = false};
            sequence_state.pre_activity_callback(&pre_activity_info,
                                                 &ex10_result);
            if (ex10_result.error)
            {
                return ex10_result;
            }
        }

        /*
        ---------------------------------------------------------------
        Now start the next activity
        ---------------------------------------------------------------
        */
        size_t const next_seq_iter = sequence_state.activity_iter;
        enum SequenceActivityConfigType const next_activity_type =
            sequence_state.activity_sequence->sequence_activities[next_seq_iter]
                .type_id;

        ex10_result =
            perform_activity_action(next_seq_iter, next_activity_type);
        if (ex10_result.error)
        {
            return ex10_result;
        }

        // If the activity was an inventory round we kicked off, we can
        // break out and wait for the next inventory summary
        if (next_activity_type == SEQUENCE_INVENTORY_ROUND_CONFIG)
        {
            return ex10_result;
        }
        else
        {
            post_callback_lock = true;
            // Note this is done before the post callback. We do not want to
            // alter it after in case the callback starts a new sequence or
            // alters state.
            sequence_state.activity_iter++;

            /*
            ---------------------------------------------------------------
            Check if the user wants to do anything after the activity
            ---------------------------------------------------------------
            */
            // NOTE: The callback can alter the sequence or iteration
            // NOTE: this doesn't happen for an inventory round since
            // the round was just started. The post callback happens on round
            // completion.
            if (sequence_state.post_activity_callback != NULL)
            {
                struct ActivityCallbackInfo post_activity_info = {
                    .activity_sequence = sequence_state.activity_sequence,
                    .sequence_iter     = sequence_state.activity_iter,
                    .first_activity    = true};
                sequence_state.post_activity_callback(&post_activity_info,
                                                      &ex10_result);
                if (ex10_result.error)
                {
                    post_callback_lock = false;
                    return ex10_result;
                }
            }
            post_callback_lock = false;
        }
    }
    // If we made it here, we exceeded the sequence count and the sequence
    // has finished
    return make_ex10_success();
}

static struct Ex10Result inventory_summary_switching(
    struct InventoryOpSummaryFields inv_status)
{
    struct InventoryRoundConfigBasic const* inventory_round =
        get_basic_inventory_round_config(sequence_state.activity_iter);
    enum InventorySummaryReason const summary_reason =
        (enum InventorySummaryReason)inv_status.done_reason;
    if (summary_reason == InventorySummaryRegulatory ||
        summary_reason == InventorySummaryTxNotRampedUp)
    {
        struct InventoryRoundControlFields inventory_config =
            inventory_round->inventory_config;

        struct InventoryRoundControl_2Fields inventory_config_2 =
            inventory_round->inventory_config_2;

        // Preserve Q across regulatory Inventory Ops.
        inventory_config.initial_q              = inv_status.final_q;
        inventory_config_2.starting_min_q_count = inv_status.min_q_count;
        inventory_config_2.starting_max_queries_since_valid_epc_count =
            inv_status.queries_since_valid_epc_count;

        return get_ex10_inventory()->start_inventory(
            inventory_round->antenna,
            inventory_round->rf_mode,
            inventory_round->tx_power_cdbm,
            &inventory_config,
            &inventory_config_2,
            inventory_round->send_selects);
    }
    else if (summary_reason == InventorySummaryDone ||
             summary_reason == InventorySummaryHost)
    {
        post_callback_lock = true;

        // If the activity was an inventory, it was not incremented since it was
        // not over. Do so now.
        // Note this is done before the post callback. We do not want to alter
        // it after in case the callback starts a new sequence or alters state.
        sequence_state.activity_iter++;
        /*
            ---------------------------------------------------------------
            Check if the user wants to do anything after the activity
            ---------------------------------------------------------------
            */
        // The inventory round finally ended, thus the activity is over.
        // NOTE: The callback can alter the sequence or iteration
        if (sequence_state.post_activity_callback != NULL)
        {
            struct ActivityCallbackInfo post_activity_info = {
                .activity_sequence = sequence_state.activity_sequence,
                .sequence_iter     = sequence_state.activity_iter,
                .first_activity    = true};
            struct Ex10Result ex10_result = make_ex10_success();
            sequence_state.post_activity_callback(&post_activity_info,
                                                  &ex10_result);
            if (ex10_result.error)
            {
                post_callback_lock = false;
                return ex10_result;
            }
        }
        struct Ex10Result ex10_result = continue_activity_sequence();
        post_callback_lock            = false;
        return ex10_result;
    }
    else if (summary_reason == InventorySummaryEventFifoFull)
    {
        return make_ex10_sdk_error(Ex10ModuleUseCase, Ex10SdkEventFifoFull);
    }
    else if (summary_reason == InventorySummaryLmacOverload)
    {
        return make_ex10_sdk_error(Ex10ModuleUseCase, Ex10SdkLmacOverload);
    }
    else if (summary_reason == InventorySummaryInvalidParam)
    {
        return make_ex10_sdk_error(Ex10ModuleUseCase,
                                   Ex10InventoryInvalidParam);
    }
    else
    {
        // The summary_reason is unknown. Treat it as an error.
    }

    // The summary_reason is unknown. Treat it as an error.
    return make_ex10_sdk_error(Ex10ModuleUseCase,
                               Ex10InventorySummaryReasonInvalid);
}

static void handle_fifo_handler_error(struct Ex10Result ex10_result,
                                      uint32_t          us_counter)
{
    struct FifoBufferNode* result_buffer_node =
        make_ex10_result_fifo_packet(ex10_result, us_counter);

    if (result_buffer_node)
    {
        // The Ex10ResultPacket will be placed into the fifo stream
        // list with full details on the encountered error.
        // Note that the microseconds counter from the
        // InventorySummary packet will be provided in the
        // Ex10Result packet.
        // This is a hint to correlate the Ex10Result packet
        // (created here) with the received InventorySummary
        // packet that triggered the continue inventory
        // operation and encountered this error.
        get_ex10_event_fifo_queue()->list_node_push_back(result_buffer_node);
    }
}

/**
 * In this use case, no interrupts are handled apart from processing EventFifo
 * packets.
 *
 * @param irq_status Unused
 * @return bool      Always return true to enable EventFifo packet call backs
 *                   on to the event_fifo_handler() function.
 */
static bool interrupt_handler(struct InterruptStatusFields irq_status)
{
    if (!irq_status.inventory_round_done)
    {
        return true;
    }

    struct RegisterInfo const* const reg_list[] = {&inventory_op_summary_reg,
                                                   &timestamp_reg};
    struct InventoryOpSummaryFields  inv_status;
    struct TimestampFields           dev_time;
    void*                            buffers[] = {&inv_status, &dev_time};
    struct Ex10Result ex10_result = get_ex10_protocol()->read_multiple(
        reg_list, buffers, ARRAY_SIZE(reg_list));

    uint32_t time_us = dev_time.current_timestamp_us;

    if (ex10_result.error)
    {
        handle_fifo_handler_error(ex10_result, time_us);
    }
    else
    {
        const uint8_t reason = inv_status.done_reason;
        switch (reason)
        {
            case InventorySummaryRegulatory:
                // Since we ended due to inventory, we should update the
                // overshoot compensation before the next ramp up. Note:
                // This is done based on the inventory packet, not the TX
                // Ramp down. If done on the ramp down packet, it's possible
                // we have already ramped back up. This is the only place
                // where the ramp is called. Note: We do not update hw lag
                // time on a user ramp down.
                ex10_result =
                    get_ex10_active_region()->update_timer_overshoot();
                break;
            case InventorySummaryDone:
            case InventorySummaryHost:
            case InventorySummaryUnsupported:
            case InventorySummaryTxNotRampedUp:
                // No special action. Continue continuous inventory.
                break;
            case InventorySummaryEventFifoFull:
                ex10_result = make_ex10_sdk_error(Ex10ModuleUseCase,
                                                  Ex10SdkEventFifoFull);
                break;
            case InventorySummaryInvalidParam:
                ex10_result = make_ex10_sdk_error(Ex10ModuleUseCase,
                                                  Ex10InventoryInvalidParam);
                break;
            case InventorySummaryLmacOverload:
                ex10_result =
                    make_ex10_sdk_error(Ex10ModuleUseCase, Ex10SdkLmacOverload);
                break;
            case InventorySummaryNone:
            default:
                ex10_result = make_ex10_sdk_error(
                    Ex10ModuleUseCase, Ex10InventorySummaryReasonInvalid);
                break;
        }
        if (ex10_result.error == true)
        {
            handle_fifo_handler_error(ex10_result, time_us);
        }
        else
        {
            ex10_result = inventory_summary_switching(inv_status);
            if (ex10_result.error == true)
            {
                handle_fifo_handler_error(ex10_result, time_us);
            }
        }
    }
    return true;
}

// Called by the interrupt handler thread when there is a fifo related
// interrupt.
static void fifo_data_handler(struct FifoBufferNode* fifo_buffer_node)
{
    // The FifoBufferNode must be placed into the reader list after the
    // continuous inventory state is updated within the IRQ_N monitor thread
    // context.
    get_ex10_event_fifo_queue()->list_node_push_back(fifo_buffer_node);
}

static struct Ex10Result init(void)
{
    sequence_state.activity_iter           = 0;
    sequence_state.publish_all_packets     = false;
    sequence_state.inventory_round_pending = 0;

    get_ex10_event_fifo_queue()->init();
    get_ex10_gen2_tx_command_manager()->init();

    // Clear any previous boot flags. The flag is set on each boot. When using
    // this use case, if the flag is ever set after this, it means a reboot has
    // occurred.
    clear_boot_flag();

    struct Ex10Protocol const* ex10_protocol = get_ex10_protocol();
    struct Ex10Result          ex10_result =
        ex10_protocol->register_fifo_data_callback(fifo_data_handler);
    if (ex10_result.error)
    {
        return ex10_result;
    }

    struct InterruptMaskFields const interrupt_mask = {
        .op_done                 = true,
        .halted                  = false,
        .event_fifo_above_thresh = true,
        .event_fifo_full         = true,
        .inventory_round_done    = true,
        .halted_sequence_done    = false,
        .command_error           = false,
        .aggregate_op_done       = false,
    };
    return ex10_protocol->register_interrupt_callback(interrupt_mask,
                                                      interrupt_handler);
}

static struct Ex10Result deinit(void)
{
    struct Ex10Protocol const* ex10_protocol = get_ex10_protocol();
    struct Ex10Result          ex10_result =
        ex10_protocol->unregister_interrupt_callback();
    if (ex10_result.error)
    {
        return ex10_result;
    }

    ex10_protocol->unregister_fifo_data_callback();

    return make_ex10_success();
}

static void register_packet_subscriber_callback(
    void (*packet_subscriber_callback)(struct EventFifoPacket const*,
                                       struct Ex10Result*))
{
    sequence_state.packet_subscriber_callback = packet_subscriber_callback;
}

static void register_pre_activity_callback(void (
    *pre_activity_callback)(struct ActivityCallbackInfo*, struct Ex10Result*))
{
    sequence_state.pre_activity_callback = pre_activity_callback;
}

static void register_post_activity_callback(void (
    *post_activity_callback)(struct ActivityCallbackInfo*, struct Ex10Result*))
{
    sequence_state.post_activity_callback = post_activity_callback;
}

static void enable_packet_filter(bool enable_filter)
{
    sequence_state.publish_all_packets = (enable_filter == false);
}

static struct Ex10ActivitySequence const* get_activity_sequence(void)
{
    return sequence_state.activity_sequence;
}

// this should use the same iter, not the publisher one. it should also create a
// new one of just get current config for all types
static struct InventoryRoundConfigBasic const* get_inventory_round(void)
{
    return get_basic_inventory_round_config(sequence_state.activity_iter);
}

static struct Ex10Result publish_packets(void)
{
    struct Ex10Result ex10_result = make_ex10_success();

    struct Ex10EventFifoQueue const* event_fifo_queue =
        get_ex10_event_fifo_queue();
    struct EventFifoPacket const* packet = NULL;

    uint32_t const sequence_complete_timeout_ms = sequence_timeout_us / 1000;
    uint32_t const start_time = get_ex10_time_helpers()->time_now();

    bool sequence_done = false;
    while (sequence_done == false && ex10_result.error == false)
    {
        bool const timeout_exceeded =
            (sequence_complete_timeout_ms > 0) &&
            (get_ex10_time_helpers()->time_elapsed(start_time) >
             sequence_complete_timeout_ms);
        if (timeout_exceeded)
        {
            if (get_reboot_occurred())
            {
                ex10_result = make_ex10_sdk_error(Ex10ModuleUseCase,
                                                  Ex10UnexpectedDeviceBoot);
            }
            // No unexpected reboot. Signal an unknown timeout error
            ex10_result =
                make_ex10_sdk_error(Ex10ModuleUseCase, Ex10SdkErrorTimeout);
            // Regardless of the timeout reason, we should exit the loop and
            // return.
            break;
        }

        uint32_t const packet_wait_timeout_us = 200u * 1000u;
        event_fifo_queue->packet_wait_with_timeout(packet_wait_timeout_us);
        packet = event_fifo_queue->packet_peek();
        while (packet != NULL)
        {
            if (packet->packet_type == Ex10ResultPacket)
            {
                ex10_result =
                    packet->static_data->ex10_result_packet.ex10_result;

                get_ex10_event_fifo_printer()->print_packets(packet);
                return ex10_result;
            }

            if (packet->packet_type == InventoryRoundSummary)
            {
                // If we are waiting on an inventory round to finish, decrement
                // the counter
                if (sequence_state.inventory_round_pending > 0)
                {
                    sequence_state.inventory_round_pending--;
                }
            }

            if (sequence_state.packet_subscriber_callback != NULL)
            {
                sequence_state.packet_subscriber_callback(packet, &ex10_result);
                // The inventory may be stopped by the client application,
                // without creating an error condition.
                if ((ex10_result.customer == true) ||
                    (ex10_result.result_code.raw != 0u))
                {
                    sequence_done = true;
                }
            }
            event_fifo_queue->packet_remove();
            packet = event_fifo_queue->packet_peek();
        }

        // Will end if:
        // - the post activity callback finished
        // - the activity iterator is over the count
        // - there are no ongoing inventories
        if (post_callback_lock == false &&
            sequence_state.inventory_round_pending == 0 &&
            sequence_state.activity_iter >=
                sequence_state.activity_sequence->count)
        {
            sequence_done = true;
        }
    }

    return ex10_result;
}

static struct Ex10Result set_activity_sequence(
    struct Ex10ActivitySequence const* activity_sequence)
{
    if (activity_sequence == NULL ||
        activity_sequence->sequence_activities == NULL)
    {
        return make_ex10_sdk_error(Ex10ModuleUseCase, Ex10SdkErrorNullPointer);
    }

    if (activity_sequence->count == 0u)
    {
        return make_ex10_sdk_error(Ex10ModuleUseCase,
                                   Ex10SdkErrorBadParamLength);
    }
    sequence_state.activity_sequence       = activity_sequence;
    sequence_state.activity_iter           = 0u;
    sequence_state.inventory_round_pending = 0u;

    return make_ex10_success();
}

static struct Ex10Result run_activity_sequence(
    struct Ex10ActivitySequence const* activity_sequence)
{
    struct Ex10Result ex10_result = set_activity_sequence(activity_sequence);
    if (ex10_result.error)
    {
        return ex10_result;
    }

    struct Ex10Protocol const* ex10_protocol = get_ex10_protocol();

    if (ex10_protocol->is_op_currently_running() == true)
    {
        return make_ex10_sdk_error(Ex10ModuleUseCase, Ex10SdkErrorOpRunning);
    }

    ex10_result = continue_activity_sequence();
    if (ex10_result.error)
    {
        return ex10_result;
    }

    /*
     * Note: even if the client did not register a packet subscriber,
     * continue with the inventory sequence.
     */
    ex10_result = publish_packets();

    return ex10_result;
}

static uint32_t get_sequence_timeout_us(void)
{
    return sequence_timeout_us;
}

static void set_sequence_timeout_us(uint32_t timeout_us)
{
    sequence_timeout_us = timeout_us;
}

static struct Ex10ActivitySequenceUseCase ex10_activity_sequence_use_case = {
    .init                                = init,
    .deinit                              = deinit,
    .register_packet_subscriber_callback = register_packet_subscriber_callback,
    .register_pre_activity_callback      = register_pre_activity_callback,
    .register_post_activity_callback     = register_post_activity_callback,
    .enable_packet_filter                = enable_packet_filter,
    .get_activity_sequence               = get_activity_sequence,
    .get_inventory_round                 = get_inventory_round,
    .set_activity_sequence               = set_activity_sequence,
    .run_activity_sequence               = run_activity_sequence,
    .get_sequence_timeout_us             = get_sequence_timeout_us,
    .set_sequence_timeout_us             = set_sequence_timeout_us,
};

struct Ex10ActivitySequenceUseCase const* get_ex10_activity_sequence_use_case(
    void)
{
    return &ex10_activity_sequence_use_case;
}
