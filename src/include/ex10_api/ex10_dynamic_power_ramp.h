/*****************************************************************************
 *                  IMPINJ CONFIDENTIAL AND PROPRIETARY                      *
 *                                                                           *
 * This source code is the property of Impinj, Inc. Your use of this source  *
 * code in whole or in part is subject to your applicable license terms      *
 * from Impinj.                                                              *
 * Contact support@impinj.com for a copy of the applicable Impinj license    *
 * terms.                                                                    *
 *                                                                           *
 * (c) Copyright 2020 - 2023 Impinj, Inc. All rights reserved.               *
 *                                                                           *
 *****************************************************************************/

#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Dynamic power ramp changes transmit power of the device while it is
 * already ramped up. This power change from power on rather from a
 * ramped down state requires some special logic.
 */
struct Ex10DynamicPowerRamp
{
    /**
     * Takes a user specified power config for the current power and the new
     * power to achieve. Uses the passed delay and coarse gain step size to
     * append to the passed aggregate op buffer with the operations needed to
     * get from the current power level to the one specified.
     *
     * @param old_power_config     The current power configurations of the
     *                             device.
     * @param new_power_config     The new power configurations to achieve.
     * @param agg_buffer           The aggregate op buffer to append to.
     * @param delay_us             The delay between coarse gain steps.
     *                             @see get_delay_us()
     * @param max_coarse_step_size The maximum step size from one coarse gain to
     *                             the next which can be made. This will
     *                             subdivide the power jump into steps within
     *                             the aggregate op.
     *                             @see get_max_coarse_gain_step_size()
     *
     * @return bool If true, all appends to the aggregate op were successful,
     *              else this returns false. The only error is if there was an
     *              overflow in appending.
     */
    bool (*append_dynamic_ramp_transmit_power)(
        struct PowerConfigs* old_power_config,
        struct PowerConfigs* new_power_config,
        struct ByteSpan*     agg_buffer,
        uint16_t             delay_us,
        uint8_t              max_coarse_step_size);

    /**
     * Returns the board specific delay to use between updating power levels.
     * Dynamic power ramp steps from power to power based on the board specific
     * result from get_max_coarse_gain_step_size(). Since this can lead to
     * multiple steps of the coarse gain value, this is the delay to use
     * between each step.
     *
     * @note: Delay between larger user commanded power stepping is left to the
     * user. This delay is used for sub-steps after a user commands a power
     * change.
     *
     * @note: This value is returned from the board layer and is board specific.
     * @return uint16_t the delay time in microseconds
     */
    uint16_t (*get_delay_us)(void);

    /**
     * Returns the max change in coarse gain value which the device may make at
     * any one power change. This value will subdivide a user specified power
     * change to be sub-divided into multiple steps based on the change needed
     * in coarse gain. Ex: If the user specified power change means changing the
     * coarse gain by 5, and the value returned here is 2, there will be 3
     * steps: +2, +4, +5.
     *
     * @note: This value is returned from the board layer and is board specific.
     * @return uint8_t the max coarse gain step that can be made
     */
    uint8_t (*get_max_coarse_gain_step_size)(void);

    /**
     * Updates the power from the current level given by the passed
     * curr_power_config. The new power is the passed new_power_cdbm which gets
     * translated into a power config within the function. This new power config
     * is set into the passed curr_power_config, thus updating the passed
     * pointer to the new current value.
     * @param curr_power_config    The current power configurations of the
     *                             device.
     * @param agg_buffer           The aggregate op buffer to use for ramping.
     *                             This function relies on an aggregate op to be
     *                             efficient, thus the user must supply
     *                             appropriate memory.
     * @param new_tx_power_cdbm    The new power in cdbm to ramp to.
     * @param freq_khz             The frequency that the device PLL is
     *                             currently locked to.
     * @param temp_adc             The temperature of the ex10_device.
     *
     * @note: If there is an Ex10Result error, the passed curr_power_config will
     *        not be updated, otherwise it is changed to reflect the new power
     * config.
     */
    struct Ex10Result (*update_power)(struct PowerConfigs* curr_power_config,
                                      struct ByteSpan*     agg_buffer,
                                      int16_t              new_tx_power_cdbm,
                                      uint32_t             freq_khz,
                                      uint16_t             temp_adc);
};

const struct Ex10DynamicPowerRamp* get_ex10_dynamic_power_ramp(void);

#ifdef __cplusplus
}
#endif
