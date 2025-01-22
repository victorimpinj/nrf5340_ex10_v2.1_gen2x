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

#include "board/board_spec.h"
#include "calibration.h"

#include "ex10_api/application_registers.h"
#include "ex10_api/board_init_core.h"
#include "ex10_api/ex10_event_fifo_queue.h"
#include "ex10_api/ex10_power_modes.h"
#include "ex10_api/ex10_rf_power.h"
#include "ex10_api/ex10_simple_example_init.h"
#include "ex10_api/ex10_utils.h"
#include "ex10_modules/ex10_antenna_disconnect.h"
#include "ex10_modules/ex10_ramp_module_manager.h"


/**
 * No interrupts are handled apart from processing EventFifo packets.
 *
 * @param irq_status Unused
 * @return bool      true enables EventFifo packet call backs
 *                   on the event_fifo_handler() function.
 */
static bool ex10_simple_interrupt_handler(
    struct InterruptStatusFields irq_status)
{
    (void)irq_status;
    return true;
}

/**
 * Called by the interrupt handler thread when there is a fifo related
 * interrupt. This node contains the current info from the Event Fifo
 * Buffer. There is nothing we need to do with the fifo buffer data,
 * so the FifoBufferNode can be immediately pushed onto the
 * event_fifo_list.
 * This is normally where use cases would implement specific logic
 * based on the packets received.
 */
static void ex10_simple_fifo_data_handler(
    struct FifoBufferNode* fifo_buffer_node)
{
    get_ex10_event_fifo_queue()->list_node_push_back(fifo_buffer_node);
}

struct Ex10Result ex10_simple_example_init(bool simple_callbacks)
{
    // must be initialized before the interrupt handlers
    // are optionally enabled (if simple_callbacks is true)
    get_ex10_event_fifo_queue()->init();

    struct Ex10Protocol const* ex10_protocol = get_ex10_protocol();
    if (simple_callbacks)
    {
        ex10_protocol->unregister_fifo_data_callback();
        ex10_protocol->unregister_interrupt_callback();

        struct Ex10Result ex10_result =
            ex10_protocol->register_fifo_data_callback(
                ex10_simple_fifo_data_handler);
        if (ex10_result.error)
        {
            return ex10_result;
        }

        struct InterruptMaskFields const interrupt_mask = {
            .op_done                 = true,
            .halted                  = true,
            .event_fifo_above_thresh = true,
            .event_fifo_full         = true,
            .inventory_round_done    = true,
            .halted_sequence_done    = true,
            .command_error           = true,
            .aggregate_op_done       = true,
        };
        ex10_result = ex10_protocol->register_interrupt_callback(
            interrupt_mask, ex10_simple_interrupt_handler);
        if (ex10_result.error)
        {
            return ex10_result;
        }
    }
    struct Ex10Result ex10_result = ex10_set_default_gpio_setup();
    if (ex10_result.error)
    {
        return ex10_result;
    }

    get_ex10_gen2_tx_command_manager()->init();
    get_ex10_power_modes()->init();

    uint16_t temperature_adc = INT16_MAX;
    ex10_result =
        get_ex10_rf_power()->measure_and_read_adc_temperature(&temperature_adc);
    if (ex10_result.error)
    {
        return ex10_result;
    }

    get_ex10_calibration()->init(ex10_protocol);

    ex10_result = ex10_protocol->set_event_fifo_threshold(0);
    if (ex10_result.error)
    {
        return ex10_result;
    }

    // Register the reader post ramp up callback to occur after cw_on calls
    struct Ex10AntennaDisconnect const* antenna_disconnect =
        get_ex10_antenna_disconnect();
    antenna_disconnect->init();

    return ex10_result;
}
