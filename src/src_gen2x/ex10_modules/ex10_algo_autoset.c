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
#include "board/ex10_osal.h"
#include "calibration.h"
#include "ex10_api/application_registers.h"
#include "ex10_api/ex10_active_region.h"
#include "ex10_api/ex10_macros.h"
#include "ex10_api/ex10_print.h"
#include "ex10_api/ex10_result.h"
#include "ex10_api/ex10_rf_power.h"
#include "ex10_api/ex10_utils.h"
#include "ex10_api/version_info.h"
#include "ex10_modules/ex10_ramp_module_manager.h"
#include "ex10_use_cases/ex10_activity_sequence_use_case.h"

#include "include_gen2x/ex10_api/application_register_definitions_gen2x.h"
#include "include_gen2x/ex10_api/application_registers_gen2x.h"
#include "include_gen2x/ex10_modules/ex10_algo_autoset.h"


enum IIState
{
    GEN2X_PREFERENCE_STATE,
    NORMAL_STATE,
};

static struct Ex10ActivitySequence activity_sequence;

static struct Ex10ActivitySequence* get_activity_sequence(void)
{
    return &activity_sequence;
}

static void set_activity_sequence(struct Ex10ActivitySequence act_seq_in)
{
    activity_sequence.count               = act_seq_in.count;
    activity_sequence.sequence_activities = act_seq_in.sequence_activities;
}

static struct Ex10Result setup_basic_activity_sequence(
    enum AutoSetModeIdGen2X           mode_id,
    uint8_t                           antenna,
    int16_t                           tx_power_cdbm,
    uint8_t                           target,
    enum InventoryRoundControlSession session,
    uint8_t                           initial_q)
{
    // --------------------------------------------------------
    // Create inventory configs which make up the activity sequence
    // --------------------------------------------------------
    static struct InventoryRoundConfigBasic
        inventory_configs[AUTOSET_RF_MODE_COUNT_GEN2X];

    struct AutoSetRfModesGen2X const* autoset_rf_modes =
        get_ex10_autoset_modes_gen2x()->get_autoset_rf_modes_gen2x(mode_id);
    if (autoset_rf_modes == NULL)
    {
        return make_ex10_sdk_error(Ex10ModuleModuleManager,
                                   Ex10SdkErrorNullPointer);
    }

    // Initialize AutoSet for A targets:
    struct Ex10Result ex10_result =
        get_ex10_autoset_modes_gen2x()->init_autoset_basic_inventory_sequence(
            &inventory_configs[0u],
            AUTOSET_RF_MODE_COUNT_GEN2X,
            autoset_rf_modes,
            antenna,
            tx_power_cdbm,
            target,
            session);
    if (ex10_result.error)
    {
        return ex10_result;
    }

    for (size_t index = 0u; index < ARRAY_SIZE(inventory_configs); ++index)
    {
        inventory_configs[index].inventory_config.initial_q = initial_q;
    }

    // --------------------------------------------------------
    // Fill the passed activity sequence with the inventory configs
    // --------------------------------------------------------
    static struct SequenceActivity
        sequence_activities[AUTOSET_RF_MODE_COUNT_GEN2X];

    for (size_t index = 0u; index < ARRAY_SIZE(inventory_configs); ++index)
    {
        sequence_activities[index].type_id = SEQUENCE_INVENTORY_ROUND_CONFIG;
        sequence_activities[index].config =
            (struct InventoryRoundConfigBasic*)&inventory_configs[index];
    }

    activity_sequence.count               = ARRAY_SIZE(sequence_activities);
    activity_sequence.sequence_activities = sequence_activities;

    return ex10_result;
}

static void set_state(enum IIState new_state)
{
    // This particular machine does not require current state, but is here to
    // suggest an outline for future state machines
    switch (new_state)
    {
        case GEN2X_PREFERENCE_STATE:
            // Re-load the same sequence in to start over from the beginning
            get_ex10_activity_sequence_use_case()->set_activity_sequence(
                get_activity_sequence());
            break;
        case NORMAL_STATE:
            // Do nothing and let the sequence carry on as defined
            break;
        default:
            break;
    }
}

static void pre_activity_algorithm_callback(
    struct ActivityCallbackInfo* sequence_info,
    struct Ex10Result*           ex10_result)
{
    if (!sequence_info->first_activity)
    {
        size_t const next_seq_iter = sequence_info->sequence_iter;
        size_t const prev_seq_iter =
            (next_seq_iter == 0) ? sequence_info->activity_sequence->count - 1
                                 : next_seq_iter - 1;

        if (prev_seq_iter == SEQUENCE_INVENTORY_ROUND_CONFIG)
        {
            struct LastTxRampDownTimeMsFields ramp_down_ms;
            *ex10_result = get_ex10_protocol()->read(
                &last_tx_ramp_down_time_ms_reg, &ramp_down_ms);
            if (ex10_result->error)
            {
                return;
            }

            static uint32_t previous_stored_ramp_down_ms = 0;
            if (ramp_down_ms.time_ms != previous_stored_ramp_down_ms)
            {
                // Must have encountered a ramp down due to regulatory since the
                // previous round
                set_state(GEN2X_PREFERENCE_STATE);
                previous_stored_ramp_down_ms = ramp_down_ms.time_ms;
            }
        }
    }
}

static struct Ex10Result init(void)
{
    get_ex10_activity_sequence_use_case()->register_pre_activity_callback(
        pre_activity_algorithm_callback);

    // Setup the Scan and ScanID configurations
    struct RegisterInfo const* const regs[] = {&scan_command_control_reg,
                                               &scan_id_command_control_reg};

    struct ScanCommandControlFields scan_control = {
        .n          = 0xF,
        .code       = (uint8_t)CodeAntipodal,
        .crypto     = false,
        .cr         = CrRN16,
        .protection = ProtectionCRC5,
        .id         = IdFull,
        .rfu        = 0};

    struct ScanIdCommandControlFields scan_id_control = {
        .scan_id_enable = false,
        .app_size       = AppSizeRfu,
        .Reserved0      = 0,
        .app_id         = 0};

    void const* buffers[] = {&scan_control, &scan_id_control};

    struct Ex10Result result =
        get_ex10_protocol()->write_multiple(regs, buffers, ARRAY_SIZE(regs));
    if (result.error)
    {
        return result;
    }

    // No need to initialize state since this simplistic state machine does not
    // track current state
    return make_ex10_success();
}

static struct Ex10Result deinit(void)
{
    // Registering callbacks to NULL overrides the previous instance
    get_ex10_activity_sequence_use_case()->register_pre_activity_callback(NULL);
    return make_ex10_success();
}

static const struct Ex10AlgoAutoset ex10_algo_autoset = {
    .init                          = init,
    .deinit                        = deinit,
    .get_activity_sequence         = get_activity_sequence,
    .set_activity_sequence         = set_activity_sequence,
    .setup_basic_activity_sequence = setup_basic_activity_sequence,
};

const struct Ex10AlgoAutoset* get_ex10_algo_autoset(void)
{
    return &ex10_algo_autoset;
}
