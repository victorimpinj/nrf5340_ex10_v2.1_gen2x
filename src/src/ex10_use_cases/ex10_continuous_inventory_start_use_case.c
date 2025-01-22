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

#include "board/board_spec.h"
#include "board/ex10_osal.h"
#include "board/time_helpers.h"

#include "ex10_api/application_register_definitions.h"
#include "ex10_api/application_registers.h"
#include "ex10_api/byte_span.h"
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
#include "ex10_api/ex10_rf_power.h"
#include "ex10_api/fifo_buffer_list.h"
#include "ex10_api/gen2_tx_command_manager.h"

#include "ex10_modules/ex10_ramp_module_manager.h"

#include "ex10_use_cases/ex10_continuous_inventory_start_use_case.h"
#include "ex10_use_cases/ex10_continuous_inventory_use_case.h"


static struct Ex10Result init(void)
{
    // super->init
    struct Ex10Result ex10_result =
        get_ex10_continuous_inventory_use_case()->init();
    if (ex10_result.error)
    {
        return ex10_result;
    }

    // Overrides the interrupt mask to allow finer control.
    struct InterruptMaskFields const interrupt_mask = {
        .op_done                 = true,
        .halted                  = true,
        .event_fifo_above_thresh = true,
        .event_fifo_full         = true,
        .inventory_round_done    = true,
        .halted_sequence_done    = true,
        .command_error           = true,
        .aggregate_op_done       = false,
    };
    return get_ex10_protocol()->write(&interrupt_mask_reg, &interrupt_mask);
}

static struct Ex10Result deinit(void)
{
    return get_ex10_continuous_inventory_use_case()->deinit();
}

static void register_packet_subscriber_callback(
    void (*callback)(struct EventFifoPacket const*, struct Ex10Result*))
{
    get_ex10_continuous_inventory_use_case()
        ->register_packet_subscriber_callback(callback);
}

static void enable_packet_filter(bool enable_filter)
{
    get_ex10_continuous_inventory_use_case()->enable_packet_filter(
        enable_filter);
}

static enum StopReason get_continuous_inventory_stop_reason(void)
{
    return get_ex10_continuous_inventory_use_case()
        ->get_continuous_inventory_stop_reason();
}

static void enable_tag_read_extended_packet(bool enable_extended_packet)
{
    get_ex10_continuous_inventory_use_case()->enable_tag_read_extended_packet(
        enable_extended_packet);
}

static struct Ex10Result continuous_inventory(
    struct InventoryRoundControlFields*   inventory_config,
    struct InventoryRoundControl_2Fields* inventory_config_2,
    struct StopConditions const*          stop_conditions,
    uint8_t                               antenna,
    enum RfModes                          rf_mode,
    int16_t                               tx_power_cdbm,
    bool                                  send_selects,
    bool                                  dual_target)
{
    get_ex10_continuous_inventory_use_case()->enable_tag_focus(
        inventory_config->tag_focus_enable);
    get_ex10_continuous_inventory_use_case()->enable_fast_id(
        inventory_config->fast_id_enable);
    get_ex10_continuous_inventory_use_case()->enable_auto_access(
        inventory_config->auto_access);
    get_ex10_continuous_inventory_use_case()->enable_abort_on_fail(
        inventory_config->abort_on_fail);

    get_ex10_continuous_inventory_use_case()->set_use_case_parameters(
        inventory_config,
        inventory_config_2,
        stop_conditions,
        antenna,
        rf_mode,
        tx_power_cdbm,
        send_selects,
        dual_target);

    get_ex10_continuous_inventory_use_case()->set_inventory_timer_start();

    return get_ex10_inventory()->start_inventory(antenna,
                                                 rf_mode,
                                                 tx_power_cdbm,
                                                 inventory_config,
                                                 inventory_config_2,
                                                 send_selects);
}

// clang-format off
static struct Ex10ContinuousInventoryStartUseCase ex10_continuous_inventory_start_use_case = {
    .init                                 = init,
    .deinit                               = deinit,
    .register_packet_subscriber_callback  = register_packet_subscriber_callback,
    .enable_packet_filter                 = enable_packet_filter,
    .enable_tag_read_extended_packet      = enable_tag_read_extended_packet,
    .continuous_inventory                 = continuous_inventory,
    .get_continuous_inventory_stop_reason = get_continuous_inventory_stop_reason,
};
// clang-format on

struct Ex10ContinuousInventoryStartUseCase const*
    get_ex10_continuous_inventory_start_use_case(void)
{
    return &ex10_continuous_inventory_start_use_case;
}
