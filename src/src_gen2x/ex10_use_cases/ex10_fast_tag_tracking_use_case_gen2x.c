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

#include "ex10_api/ex10_print.h"

#include "ex10_api/application_register_field_enums.h"
#include "ex10_api/bit_span.h"
#include "ex10_api/ex10_continuous_inventory_common.h"
#include "ex10_api/ex10_macros.h"
#include "ex10_api/ex10_protocol.h"
#include "ex10_api/ex10_select_commands.h"

#include "include_gen2x/ex10_use_cases/ex10_continuous_inventory_use_case_gen2x.h"
#include "include_gen2x/ex10_use_cases/ex10_fast_tag_tracking_use_case_gen2x.h"

#include "include_gen2x/ex10_api/application_register_field_enums_gen2x.h"
#include "include_gen2x/ex10_api/event_fifo_packet_types_gen2x.h"

struct FastTagTrackingState
{
    /// The callback to notify the subscriber of a new packet.
    void (*packet_subscriber_callback)(struct EventFifoPacket const*,
                                       struct Ex10Result*);
    bool            force_full_round;
    bool            switch_to_full_round;
    bool            stop_inventory;
    enum StopReason stop_reason;
    size_t          fast_round_count;
    size_t          round_count;
    size_t          tag_count;
    uint16_t        last_new_stored_crc;
    struct BitSpan  known_bitspan;
    struct BitSpan  new_bitspan;
};

static struct FastTagTrackingState fast_tag_tracking_state = {
    NULL,                         // callback pointer
    false,                        // force_full_round,
    false,                        // switch to full round,
    false,                        // stop_inventory
    SRNone,                       // inventory stop reason
    0,                            // fast round count
    0,                            // round count
    0,                            // tag_count
    0x0000,                       // last new stored crc
    {.data = NULL, .length = 0},  // known bitspan
    {.data = NULL, .length = 0}   // new bitspan
};

static uint32_t              start_time_us = 0;
static struct StopConditions stop_conditions;

enum Ex10FastTagTrackingResultCode
{
    Ex10ApplicationSuccess = 0,
    Ex10FastTagTrackingNewTag,
    Ex10HostStopRequested,
};

static void set_inventory_timer_start(void)
{
    start_time_us = get_ex10_ops()->get_device_time();
}

static struct Ex10Result init(void)
{
    // This uses the continuous inventory use case interrupt handlers
    // The callback will be injected as needed by the fast tag tracking
    // inventory
    return get_ex10_continuous_inventory_use_case_gen2x()->init();
}

static struct Ex10Result deinit(void)
{
    return get_ex10_continuous_inventory_use_case_gen2x()->deinit();
}

static void register_packet_subscriber_callback(
    void (*callback)(struct EventFifoPacket const*, struct Ex10Result*))
{
    fast_tag_tracking_state.packet_subscriber_callback = callback;
}

static void dummy_enable_packet_filter(bool enable_filter)
{
    // should not be used, it should be pointed at the continuous
    // inventory use case's function in the get_ below
    (void)enable_filter;
}

static enum StopReason get_fast_tag_tracking_stop_reason(void)
{
    return fast_tag_tracking_state.stop_reason;
}

static void force_full_round(void)
{
    fast_tag_tracking_state.force_full_round = true;
}

static void stop_inventory(void)
{
    // tell our stop condition checker and our callback
    // to tell the continuous inventory use case to stop.
    fast_tag_tracking_state.stop_inventory = true;
}

static struct StopConditions update_stop_conditions(void)
{
    struct StopConditions updated_stop = {.max_duration_us      = 0,
                                          .max_number_of_rounds = 0,
                                          .max_number_of_tags   = 0};

    if (stop_conditions.max_duration_us != 0)
    {
        uint32_t elapsed_time =
            get_ex10_ops()->get_device_time() - start_time_us;
        updated_stop.max_duration_us =
            stop_conditions.max_duration_us - elapsed_time;
    }
    if (stop_conditions.max_number_of_rounds != 0)
    {
        updated_stop.max_number_of_rounds =
            stop_conditions.max_number_of_rounds -
            fast_tag_tracking_state.round_count;
    }
    if (stop_conditions.max_number_of_tags != 0)
    {
        updated_stop.max_number_of_tags = stop_conditions.max_number_of_tags -
                                          fast_tag_tracking_state.tag_count;
    }

    return updated_stop;
}

static bool check_stop_conditions(uint32_t timestamp_us)
{
    // If the reason is already set, we return so as to retain the original stop
    // reason
    if (fast_tag_tracking_state.stop_reason != SRNone)
    {
        return true;
    }

    if (stop_conditions.max_number_of_rounds > 0u)
    {
        if (fast_tag_tracking_state.round_count >=
            stop_conditions.max_number_of_rounds)
        {
            fast_tag_tracking_state.stop_reason = SRMaxNumberOfRounds;
            return true;
        }
    }
    if (stop_conditions.max_number_of_tags > 0u)
    {
        if (fast_tag_tracking_state.tag_count >=
            stop_conditions.max_number_of_tags)
        {
            fast_tag_tracking_state.stop_reason = SRMaxNumberOfTags;
            return true;
        }
    }
    if (stop_conditions.max_duration_us > 0u)
    {
        // Packet before start checks for packets which occurred before the
        // continuous inventory round was started.
        bool const     packet_before_start = (start_time_us > timestamp_us);
        uint32_t const elapsed_us =
            (packet_before_start)
                ? ((UINT32_MAX - start_time_us) + timestamp_us + 1)
                : (timestamp_us - start_time_us);
        if (elapsed_us >= stop_conditions.max_duration_us)
        {
            fast_tag_tracking_state.stop_reason = SRMaxDuration;
            return true;
        }
    }
    if (fast_tag_tracking_state.stop_inventory)
    {
        fast_tag_tracking_state.stop_reason = SRHost;
        return true;
    }
    return false;
}

static struct Ex10Result push_fast_tag_tracking_mode_packet(
    uint32_t                     time_us,
    enum FastTagTrackingInvState state)
{
    struct Ex10FastTagTrackingStateChangeGen2X state_change = {
        .fast_tag_tracking_state = (uint8_t)state,
        .packet_rfu1             = 0,
        .packet_rfu2             = 0};

    // use the static data for our custom message

    struct EventFifoPacket const summary_packet = {
        .packet_type = Custom,
        .us_counter  = time_us,
        .static_data = (union PacketData const*)&state_change,
        .static_data_length =
            sizeof(struct Ex10FastTagTrackingStateChangeGen2X),
        .dynamic_data        = NULL,
        .dynamic_data_length = 0u,
        .is_valid            = true,
    };

    bool const trigger_irq = true;
    return get_ex10_protocol()->insert_fifo_event(trigger_irq, &summary_packet);
}


static struct Ex10Result make_ex10_customer_reason(
    enum Ex10FastTagTrackingResultCode fast_tag_tracking_result_code)
{
    struct Ex10Result const ex10_result = {
        .error         = false,
        .customer      = true,
        .rfu           = 0,
        .module        = Ex10ModuleApplication,
        .result_code   = {.raw = (uint8_t)fast_tag_tracking_result_code},
        .device_status = {.raw = 0}};

    return ex10_result;
}

static void set_bitfield_bit(struct BitSpan bit_span, uint16_t bit)
{
    const size_t index = bit / 8;
    const size_t shift = bit % 8;
    bit_span.data[index] |= (0x01 << shift);
}

static bool check_bitfield_set(struct BitSpan bit_span, uint16_t bit)
{
    const int     index = bit / 8;
    const int     shift = bit % 8;
    const uint8_t value = bit_span.data[index] & (0x01 << shift);
    return (value != 0);
}

static void clear_bitfield(struct BitSpan bit_span)
{
    // Convert this to a 32-bit write just for speed
    uint32_t* word = (uint32_t*)bit_span.data;
    for (size_t i = 0; i < bit_span.length / (sizeof(uint32_t) * 8); i++)
    {
        *word++ = 0;
    }
}

static void full_epc_callback(struct EventFifoPacket const* packet,
                              struct Ex10Result*            result_ptr)
{
    *result_ptr = make_ex10_success();

    // with this callback we are building the known bitfield
    if (packet->packet_type == TagReadExtended)
    {
        struct TagReadExtendedGen2X const* tag_read_extended =
            ((struct TagReadExtendedGen2X const*)&packet->static_data
                 ->tag_read_extended);
        if (tag_read_extended->id != IdFull ||
            tag_read_extended->cr != CrStoredCRC)
        {
            // We are supposed to be getting full EPCs in this callback!
            *result_ptr = make_ex10_sdk_error(Ex10ModuleUseCase,
                                              Ex10SdkErrorBadParamValue);
            return;
        }
        // Only the bottom 16 bits of the cr_value are valid in this case.
        set_bitfield_bit(fast_tag_tracking_state.known_bitspan,
                         (tag_read_extended->cr_value & 0xFFFF));
    }
    else if (packet->packet_type == TagRead)
    {
        // We are not supposed to be getting TagReads!
        *result_ptr =
            make_ex10_sdk_error(Ex10ModuleUseCase, Ex10SdkErrorBadParamValue);
        return;
    }
    // else nothing

    if (fast_tag_tracking_state.packet_subscriber_callback != NULL)
    {
        // we are passing this though blindly because we don't know
        // if they have turned off the packet filter so we pass everything.
        fast_tag_tracking_state.packet_subscriber_callback(packet, result_ptr);

        if (fast_tag_tracking_state.stop_inventory)
        {
            // if the use requested a stop we override the result pointer
            // from the callback with our own request to stop
            *result_ptr = make_ex10_customer_reason(Ex10HostStopRequested);
        }
    }
}


static struct Ex10Result run_full_epc_inventory(
    uint32_t                                          device_time,
    struct Ex10FastTagTrackingUseCaseParametersGen2X* params)
{
    clear_bitfield(fast_tag_tracking_state.known_bitspan);

    // run only 1 round to fill the known EPCs and ignore
    // the stop other stop conditions for a full epc round.
    struct StopConditions full_epc_stop_conditions = {
        .max_duration_us      = 0,
        .max_number_of_rounds = 1,
        .max_number_of_tags   = 0,
    };


    // Setup for the Full EPC round to gather the known bitfield
    struct Ex10ContinuousInventoryUseCaseParametersGen2X ciucpg = {
        .antenna         = params->antenna,
        .rf_mode         = params->rf_mode,
        .tx_power_cdbm   = params->tx_power_cdbm,
        .initial_q       = params->initial_q,
        .session         = params->session,
        .target          = 0,
        .select          = params->select,
        .send_selects    = true,
        .stop_conditions = &full_epc_stop_conditions,
        .dual_target     = false,
        .crypto          = false,
        .code            = CodeAntipodal,
        .cr              = CrStoredCRC,
        .cr_protection   = ProtectionCRC5,
        .id              = IdFull,
        .scan_id_enable  = false,
        .app_size        = 0,
        .app_id          = 0};


    // set the full EPC callback
    struct Ex10ContinuousInventoryUseCaseGen2X const* ciucg =
        get_ex10_continuous_inventory_use_case_gen2x();
    ciucg->register_packet_subscriber_callback(&full_epc_callback);
    // send a message to the callback that we are starting an inventory round
    push_fast_tag_tracking_mode_packet(device_time, FullEpcMode);
    struct Ex10Result ex10_result = ciucg->continuous_inventory(&ciucpg);
    if (ex10_result.error)
    {
        return ex10_result;
    }

    return make_ex10_success();
}

static void fast_inventory_callback(struct EventFifoPacket const* packet,
                                    struct Ex10Result*            result_ptr)
{
    *result_ptr           = make_ex10_success();
    bool new_tag_detected = false;

    if (packet->packet_type == TagReadExtended)
    {
        struct TagReadExtendedGen2X const* tag_read_extended =
            ((struct TagReadExtendedGen2X const*)&packet->static_data
                 ->tag_read_extended);
        if (tag_read_extended->id != IdNoAckResponse ||
            tag_read_extended->cr != CrStoredCRC)
        {
            // We are supposed to be getting CRs in this callback!
            *result_ptr = make_ex10_sdk_error(Ex10ModuleUseCase,
                                              Ex10SdkErrorBadParamValue);
            return;
        }

        fast_tag_tracking_state.tag_count++;
        uint16_t cr_value = (uint16_t)(tag_read_extended->cr_value & 0xFFFF);
        if (check_bitfield_set(fast_tag_tracking_state.known_bitspan,
                               cr_value) == false)
        {
            if (check_bitfield_set(fast_tag_tracking_state.new_bitspan,
                                   cr_value))
            {
                // It is in the new bitfield so we assume it is not a phantom.
                // End the fast inventory early and have the application decide
                // what it wants to do
                fast_tag_tracking_state.last_new_stored_crc  = cr_value;
                fast_tag_tracking_state.switch_to_full_round = true;
                new_tag_detected                             = true;
                // set the result pointer to customer to let get the continuous
                // inventory to stop.
                *result_ptr =
                    make_ex10_customer_reason(Ex10FastTagTrackingNewTag);
                // and we do not return so that the new stored crc gets reported
                // to the callback
            }
            else
            {
                // it is not in the new bitfield so we should add it and
                // continue inventory
                set_bitfield_bit(fast_tag_tracking_state.new_bitspan, cr_value);
                // but not report it to the example so we return early
                return;
            }
        }
        // else
        // it is in the known bitfield so we let the
        // callback report it at the end of this function.
    }
    else if (packet->packet_type == ContinuousInventorySummary)
    {
        fast_tag_tracking_state.fast_round_count++;
        if (fast_tag_tracking_state.fast_round_count == 10)
        {
            // Every 10 rounds, clear out the new tag bits
            // as they didn't come back.  They are probably phantom
            // noise that we don't want it to accumulate over time.
            clear_bitfield(fast_tag_tracking_state.new_bitspan);
            fast_tag_tracking_state.fast_round_count = 0;
        }
    }
    else if (packet->packet_type == TagRead)
    {
        // We are not supposed to be getting TagReads!
        *result_ptr =
            make_ex10_sdk_error(Ex10ModuleUseCase, Ex10SdkErrorBadParamValue);
        return;
    }
    else if (packet->packet_type == ContinuousInventorySummary)
    {
        fast_tag_tracking_state.round_count++;
    }
    // else nothing

    if (fast_tag_tracking_state.packet_subscriber_callback != NULL)
    {
        // we are passing this though blindly because we don't know
        // if they have turned off the packet filter so we pass everything
        fast_tag_tracking_state.packet_subscriber_callback(packet, result_ptr);
        // if the callback declared it an error we just pass it back though

        if (fast_tag_tracking_state.stop_inventory)
        {
            // if the application requested a stop we override the result
            // pointer from the callback with our own request to stop
            *result_ptr = make_ex10_customer_reason(Ex10HostStopRequested);
        }
        else if (fast_tag_tracking_state.force_full_round)
        {
            // if the host requested that we change to a full round.
            *result_ptr = make_ex10_customer_reason(Ex10FastTagTrackingNewTag);
        }
    }

    // now we check to see if the a new tag was detected so we should
    // run a full EPC round.. (note that if the customer requested it
    // or any reason that will override our decision)
    if (result_ptr->error == false && result_ptr->customer == false &&
        new_tag_detected)
    {
        *result_ptr = make_ex10_customer_reason(Ex10FastTagTrackingNewTag);
    }
}

static struct Ex10Result run_fast_inventory(
    uint32_t                                          device_time,
    struct Ex10FastTagTrackingUseCaseParametersGen2X* params)
{
    // update the inventory stop conditions as this is may
    // not be the first time this inventory round was run
    struct StopConditions fast_stop_conditions = update_stop_conditions();

    // Setup for fast inventory responses (ID == None and CR == StoredCRC)
    // start the dual target in B (1) because we assume that the fast
    // inventory round pushed them all to B
    struct Ex10ContinuousInventoryUseCaseParametersGen2X ciucpg = {
        .antenna         = params->antenna,
        .rf_mode         = params->rf_mode,
        .tx_power_cdbm   = params->tx_power_cdbm,
        .initial_q       = params->initial_q,
        .session         = params->session,
        .target          = 1,
        .select          = params->select,
        .send_selects    = false,
        .stop_conditions = &fast_stop_conditions,
        .dual_target     = true,
        .crypto          = false,
        .code            = CodeAntipodal,
        .cr              = CrStoredCRC,
        .cr_protection   = ProtectionCRC5,
        .id              = IdNoAckResponse,
        .scan_id_enable  = false,
        .app_size        = 0,
        .app_id          = 0};

    // set the fast inventory callback
    struct Ex10ContinuousInventoryUseCaseGen2X const* ciucg =
        get_ex10_continuous_inventory_use_case_gen2x();
    ciucg->register_packet_subscriber_callback(&fast_inventory_callback);
    // send a message to the callback that we are starting an inventory round
    push_fast_tag_tracking_mode_packet(device_time, FastInventoryMode);
    return ciucg->continuous_inventory(&ciucpg);
}

static struct Ex10Result fast_tag_tracking_inventory(
    struct Ex10FastTagTrackingUseCaseParametersGen2X* params)
{
    if (params == NULL || params->stop_conditions == NULL ||
        params->known_stored_crcs.data == NULL ||
        params->new_stored_crcs.data == NULL)
    {
        return make_ex10_sdk_error(Ex10ModuleUseCase, Ex10SdkErrorNullPointer);
    }

    if (params->known_stored_crcs.length != SIZE_OF_BITFIELD ||
        params->new_stored_crcs.length != SIZE_OF_BITFIELD)
    {
        return make_ex10_sdk_error(Ex10ModuleUseCase,
                                   Ex10SdkErrorBadParamLength);
    }

    set_inventory_timer_start();

    struct Ex10SelectCommands const* select_commands =
        get_ex10_select_commands();
    const ssize_t select_command_index_A =
        select_commands->set_select_session_command(target_A, params->session);
    if (select_command_index_A < 0)
    {
        return make_ex10_sdk_error(Ex10ModuleUseCase,
                                   Ex10ErrorGen2BufferLength);
    }
    select_commands->enable_select_command((size_t)select_command_index_A);

    stop_conditions                       = *params->stop_conditions;
    fast_tag_tracking_state.known_bitspan = params->known_stored_crcs;
    fast_tag_tracking_state.new_bitspan   = params->new_stored_crcs;

    // enable the extended tag read to expose the CR (StoredCRC) values
    get_ex10_continuous_inventory_use_case_gen2x()
        ->enable_tag_read_extended_packet(true);

    struct Ex10Result ex10_result = make_ex10_success();

    // clear the known epc bitfield and build a new one
    if (params->build_stored_crcs)
    {
        ex10_result = run_full_epc_inventory(start_time_us, params);
        if (ex10_result.error)
        {
            return ex10_result;
        }
    }

    if (params->clear_new_stored_crcs)
    {
        clear_bitfield(fast_tag_tracking_state.new_bitspan);
    }
    uint32_t device_time = get_ex10_ops()->get_device_time();
    while (check_stop_conditions(device_time) == false)
    {
        // run fast inventory
        ex10_result = run_fast_inventory(device_time, params);
        if (ex10_result.error)
        {
            // something bad happended
            return ex10_result;
        }

        if (fast_tag_tracking_state.switch_to_full_round ||
            fast_tag_tracking_state.force_full_round)
        {
            // clear out the new tag bitfield
            clear_bitfield(fast_tag_tracking_state.new_bitspan);
            // if the customer flag is set, then we should do a full inventory
            device_time = get_ex10_ops()->get_device_time();
            ex10_result = run_full_epc_inventory(device_time, params);
            if (ex10_result.error)
            {
                return ex10_result;
            }
            // reset the full round or switch requests
            fast_tag_tracking_state.force_full_round     = false;
            fast_tag_tracking_state.switch_to_full_round = false;
        }
        device_time = get_ex10_ops()->get_device_time();
    }

    return ex10_result;
}

static struct Ex10FastTagTrackingUseCaseGen2X
    ex10_fast_tag_tracking_use_case_gen2x = {
        .init   = init,
        .deinit = deinit,
        .register_packet_subscriber_callback =
            register_packet_subscriber_callback,
        .enable_packet_filter              = dummy_enable_packet_filter,
        .get_fast_tag_tracking_stop_reason = get_fast_tag_tracking_stop_reason,
        .force_full_round                  = force_full_round,
        .stop_inventory                    = stop_inventory,
        .clear_bitfield                    = clear_bitfield,
        .fast_tag_tracking_inventory       = fast_tag_tracking_inventory,
};

struct Ex10FastTagTrackingUseCaseGen2X const*
    get_ex10_fast_tag_tracking_use_case_gen2x(void)
{
    // the functionality for these functions is just to pass
    // it through to the continuous inventory use case, so we
    // populate this use cases pointers with those at runtime
    struct Ex10ContinuousInventoryUseCase const* ciuc =
        get_ex10_continuous_inventory_use_case();

    // a pointer to the struct with a shorter name
    // for readability
    struct Ex10FastTagTrackingUseCaseGen2X* fttucg =
        &ex10_fast_tag_tracking_use_case_gen2x;

    fttucg->enable_packet_filter = ciuc->enable_packet_filter;

    return &ex10_fast_tag_tracking_use_case_gen2x;
}
