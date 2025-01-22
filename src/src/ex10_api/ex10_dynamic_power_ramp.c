/*****************************************************************************
 *                  IMPINJ CONFIDENTIAL AND PROPRIETARY                      *
 *                                                                           *
 * This source code is the property of Impinj, Inc. Your use of this source  *
 * code in whole or in part is subject to your applicable license terms      *
 * from Impinj.                                                              *
 * Contact support@impinj.com for a copy of the applicable Impinj license    *
 * terms.                                                                    *
 *                                                                           *
 * (c) Copyright 2020 - 2024 Impinj, Inc. All rights reserved.               *
 *                                                                           *
 *****************************************************************************/

#include "board/board_spec.h"

#include "calibration.h"
#include "ex10_api/aggregate_op_builder.h"
#include "ex10_api/application_registers.h"
#include "ex10_api/board_init.h"
#include "ex10_api/event_fifo_printer.h"
#include "ex10_api/ex10_active_region.h"
#include "ex10_api/ex10_dynamic_power_ramp.h"
#include "ex10_api/ex10_ops.h"
#include "ex10_api/ex10_print.h"
#include "ex10_api/ex10_protocol.h"
#include "ex10_api/rf_mode_definitions.h"
#include "ex10_api/trace.h"


static struct PowerDroopCompensationFields droop_comp_on = {
    .enable                   = true,
    .compensation_interval_ms = 25,
    .fine_gain_step_cd_b      = 10,
};

static struct PowerDroopCompensationFields droop_comp_off = {
    .enable                   = false,
    .compensation_interval_ms = 25,
    .fine_gain_step_cd_b      = 10,
};

static bool append_delay_time_us(struct ByteSpan* agg_buffer,
                                 uint32_t         delay_time_us)
{
    struct Ex10AggregateOpBuilder const* agg_builder =
        get_ex10_aggregate_op_builder();

    if (!agg_builder->append_start_timer_op(delay_time_us, agg_buffer))
    {
        return false;
    }
    if (!agg_builder->append_wait_timer_op(agg_buffer))
    {
        return false;
    }
    return true;
}

static bool append_dynamic_ramp_transmit_power(
    struct PowerConfigs* old_power_config,
    struct PowerConfigs* new_power_config,
    struct ByteSpan*     agg_buffer,
    uint16_t             delay_us,
    uint8_t              max_coarse_step_size)
{
    struct Ex10AggregateOpBuilder const* agg_builder =
        get_ex10_aggregate_op_builder();

    const uint8_t starting_coarse_gain = old_power_config->tx_atten;
    const uint8_t final_coarse_gain    = new_power_config->tx_atten;

    if (!agg_builder->append_droop_compensation(&droop_comp_off, agg_buffer))
    {
        return false;
    }

    // start by adding a change to the nominal fine gain
    if (!agg_builder->append_set_tx_fine_gain(new_power_config->tx_scalar,
                                              agg_buffer))
    {
        return false;
    }
    // now add in delay and coarse gain changes up to the required coarse gain
    // in steps of max_coarse_step_size
    uint8_t    coarse_to_use = starting_coarse_gain;
    const bool coarse_pos    = (final_coarse_gain >= starting_coarse_gain);
    while (coarse_to_use != final_coarse_gain)
    {
        if (coarse_pos)
        {
            coarse_to_use += max_coarse_step_size;
            coarse_to_use = (coarse_to_use > final_coarse_gain)
                                ? final_coarse_gain
                                : coarse_to_use;
        }
        else
        {
            // ensure we don't pass 0
            coarse_to_use = (max_coarse_step_size > coarse_to_use)
                                ? 0
                                : coarse_to_use - max_coarse_step_size;
            // ensure we did not pass the final gain
            coarse_to_use = (coarse_to_use < final_coarse_gain)
                                ? final_coarse_gain
                                : coarse_to_use;
        }
        if (!append_delay_time_us(agg_buffer, delay_us) ||
            !agg_builder->append_set_tx_coarse_gain(coarse_to_use, agg_buffer))
        {
            return false;
        }
    }

    if (!agg_builder->append_droop_compensation(&droop_comp_on, agg_buffer))
    {
        return false;
    }

    // Update the DC offset by setting the register and running the ramp up op.
    // If already ramped up, the dc offset will be updated and the op will exit.
    struct ConstByteSpan offset_span = {
        .data   = ((uint8_t const*)&new_power_config->dc_offset),
        .length = sizeof(new_power_config->dc_offset)};
    if (!agg_builder->append_reg_write(
            &dc_offset_reg, &offset_span, agg_buffer) ||
        !agg_builder->append_op_run(SetDcOffsetOp, agg_buffer))
    {
        return false;
    }

    // finally add in a run of closed loop power control
    if (!agg_builder->append_power_control(new_power_config, agg_buffer) ||
        !agg_builder->append_run_sjc(agg_buffer))
    {
        return false;
    }

    return true;
}

static uint16_t get_delay_us(void)
{
    return 0;
}

static uint8_t get_max_coarse_gain_step_size(void)
{
    return 5;
}

static struct Ex10Result update_power(struct PowerConfigs* curr_power_config,
                                      struct ByteSpan*     agg_buffer,
                                      int16_t              new_tx_power_cdbm,
                                      uint32_t             freq_khz,
                                      uint16_t             temp_adc)
{
    struct Ex10ActiveRegion const*       region      = get_ex10_active_region();
    struct Ex10Calibration const*        calibration = get_ex10_calibration();
    struct Ex10AggregateOpBuilder const* agg_builder =
        get_ex10_aggregate_op_builder();
    struct Ex10Ops const* ops = get_ex10_ops();

    // Get configs for the passed power and update the configs passed in.
    bool const temp_comp_enabled =
        get_ex10_board_spec()->temperature_compensation_enabled(temp_adc);
    struct PowerConfigs new_power_config =
        calibration->get_power_control_params(new_tx_power_cdbm,
                                              false,
                                              freq_khz,
                                              temp_adc,
                                              temp_comp_enabled,
                                              region->get_rf_filter());

    if (!append_dynamic_ramp_transmit_power(curr_power_config,
                                            &new_power_config,
                                            agg_buffer,
                                            get_delay_us(),
                                            get_max_coarse_gain_step_size()))
    {
        // There was an issue in appending to the aggregate op buffer. Since
        // nothing was run, the configs are not updated.
        return make_ex10_sdk_error(Ex10ModuleDynamicPowerRamp,
                                   Ex10SdkErrorAggBufferOverflow);
    }


    // Add the exit instruction and set the buffer
    agg_builder->append_exit_instruction(agg_buffer);
    agg_builder->set_buffer(agg_buffer);

    // Run the power change
    struct Ex10Result ex10_result = ops->run_aggregate_op();
    if (ex10_result.error)
    {
        // There was an error in running the aggregate op. Note that the power
        // could be at some intermediate power step, thus the power config is
        // not updated. For further info on where the power actually is, one
        // must check the aggregate op logging to determine which instruction it
        // failed on.
        return ex10_result;
    }
    ops->wait_op_completion();

    // Success, so we update our power config
    *curr_power_config = new_power_config;

    return ex10_result;
}

static struct Ex10DynamicPowerRamp const ex10_dynamic_power_ramp = {
    .append_dynamic_ramp_transmit_power = append_dynamic_ramp_transmit_power,
    .get_delay_us                       = get_delay_us,
    .get_max_coarse_gain_step_size      = get_max_coarse_gain_step_size,
    .update_power                       = update_power,
};

struct Ex10DynamicPowerRamp const* get_ex10_dynamic_power_ramp(void)
{
    return &ex10_dynamic_power_ramp;
}
