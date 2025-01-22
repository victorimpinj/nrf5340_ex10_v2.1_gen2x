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

#include <stddef.h>
#include <stdint.h>

#include "ex10_api/ex10_activity_sequence.h"
#include "ex10_api/ex10_result.h"
#include "include_gen2x/ex10_api/ex10_autoset_modes_gen2x.h"

#ifdef __cplusplus
extern "C" {
#endif


struct Ex10AlgoAutoset
{
    /**
     * Initialize the module to set up the activity sequence and register the
     * pre-activity callback
     */
    struct Ex10Result (*init)(void);

    /**
     *  De-initialize the module and unregister the pre-event callback
     */
    struct Ex10Result (*deinit)(void);

    /**
     *  Returns the activity sequence required of the algorithm from the module.
     * Since this is a pointer to the sequence, it is owned by the module. The
     * sequence is not setup before calling the module init function, but can be
     * grabbed at any time.
     */
    struct Ex10ActivitySequence* (*get_activity_sequence)(void);

    /**
     * Sets a custom activity sequence for the module to use. This is meant for
     * more complex usage when users want to use the ex10_algo_autoset module
     * algorithm with a specific sequence.
     */
    void (*set_activity_sequence)(struct Ex10ActivitySequence act_seq_in);

    /**
     * Sets up a general single target activity sequence.
     */
    struct Ex10Result (*setup_basic_activity_sequence)(
        enum AutoSetModeIdGen2X           mode_id,
        uint8_t                           antenna,
        int16_t                           tx_power_cdbm,
        uint8_t                           target,
        enum InventoryRoundControlSession session,
        uint8_t                           initial_q);
};

const struct Ex10AlgoAutoset* get_ex10_algo_autoset(void);

#ifdef __cplusplus
}
#endif
