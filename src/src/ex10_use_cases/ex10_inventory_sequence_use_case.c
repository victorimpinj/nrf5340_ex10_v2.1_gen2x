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

#include "ex10_api/application_registers.h"
#include "ex10_api/event_fifo_packet_types.h"
#include "ex10_api/event_fifo_printer.h"
#include "ex10_api/event_packet_parser.h"
#include "ex10_api/ex10_active_region.h"
#include "ex10_api/ex10_boot_health.h"
#include "ex10_api/ex10_event_fifo_queue.h"
#include "ex10_api/ex10_inventory.h"
#include "ex10_api/ex10_macros.h"
#include "ex10_api/ex10_ops.h"
#include "ex10_api/ex10_print.h"
#include "ex10_api/ex10_protocol.h"
#include "ex10_api/ex10_result.h"
#include "ex10_api/ex10_rf_power.h"
#include "ex10_api/ex10_utils.h"
#include "ex10_api/fifo_buffer_list.h"
#include "ex10_api/gen2_tx_command_manager.h"

#include "ex10_modules/ex10_ramp_module_manager.h"

#include "ex10_use_cases/ex10_activity_sequence_use_case.h"
#include "ex10_use_cases/ex10_inventory_sequence_use_case.h"


#define MAX_INVENTORY_SEQUENCE_COUNT 0xFF


/**
 * @struct InventorySequenceState
 * Progress through a requested sequence of inventory rounds.
 */
struct InventorySequenceState
{
    /// Points to the client supplied inventory sequences struct.
    /// - Set when Ex10InventorySequenceUseCase.run_inventory_sequence() is
    ///   called by the client.
    /// - Read by IRQ_N monitor thread context within fifo_data_handler()
    ///   call chain and the publish_packets() function.
    struct InventoryRoundSequence const* inventory_sequence;
};

static struct InventorySequenceState inventory_state;

static struct Ex10Result init(void)
{
    return get_ex10_activity_sequence_use_case()->init();
}

static struct Ex10Result deinit(void)
{
    return get_ex10_activity_sequence_use_case()->deinit();
}

static void register_packet_subscriber_callback(
    void (*packet_subscriber_callback)(struct EventFifoPacket const*,
                                       struct Ex10Result*))
{
    get_ex10_activity_sequence_use_case()->register_packet_subscriber_callback(
        packet_subscriber_callback);
}

static void enable_packet_filter(bool enable_filter)
{
    get_ex10_activity_sequence_use_case()->enable_packet_filter(enable_filter);
}

static struct InventoryRoundSequence const* get_inventory_sequence(void)
{
    return inventory_state.inventory_sequence;
}

static struct InventoryRoundConfigBasic const* get_inventory_round(void)
{
    return get_ex10_activity_sequence_use_case()->get_inventory_round();
}

static struct Ex10Result run_inventory_sequence(
    struct InventoryRoundSequence const* inventory_sequence)
{
    if (inventory_sequence == NULL || inventory_sequence->configs == NULL)
    {
        return make_ex10_sdk_error(Ex10ModuleUseCase, Ex10SdkErrorNullPointer);
    }

    // A max sequence size is enforced to allow for translation to the sequence
    // event paradigm (by not having variable array size). If the sequence
    // events are used directly, this max size is not needed.
    if (inventory_sequence->count == 0u ||
        inventory_sequence->count >= MAX_INVENTORY_SEQUENCE_COUNT)
    {
        return make_ex10_sdk_error(Ex10ModuleUseCase,
                                   Ex10SdkErrorBadParamValue);
    }

    // With this interface, the type must be inventory config only
    if (inventory_sequence->type_id != INVENTORY_ROUND_CONFIG_BASIC)
    {
        return make_ex10_sdk_error(Ex10ModuleUseCase,
                                   Ex10SdkErrorBadParamValue);
    }

    inventory_state.inventory_sequence = inventory_sequence;

    // Translate to the new interface of sequence events
    struct SequenceActivity sequence_activities[MAX_INVENTORY_SEQUENCE_COUNT];
    for (uint8_t idx = 0; idx < inventory_sequence->count; idx++)
    {
        sequence_activities[idx].type_id = SEQUENCE_INVENTORY_ROUND_CONFIG;
        sequence_activities[idx].config =
            &(((const struct InventoryRoundConfigBasic*)
                   inventory_sequence->configs)[idx]);
    }

    struct Ex10ActivitySequence const activity_sequence = {
        .sequence_activities = sequence_activities,
        .count               = inventory_sequence->count,
    };

    return get_ex10_activity_sequence_use_case()->run_activity_sequence(
        &activity_sequence);
}

static struct Ex10InventorySequenceUseCase ex10_inventory_sequence_use_case = {
    .init                                = init,
    .deinit                              = deinit,
    .register_packet_subscriber_callback = register_packet_subscriber_callback,
    .enable_packet_filter                = enable_packet_filter,
    .get_inventory_sequence              = get_inventory_sequence,
    .get_inventory_round                 = get_inventory_round,
    .run_inventory_sequence              = run_inventory_sequence,
};

struct Ex10InventorySequenceUseCase const* get_ex10_inventory_sequence_use_case(
    void)
{
    return &ex10_inventory_sequence_use_case;
}
