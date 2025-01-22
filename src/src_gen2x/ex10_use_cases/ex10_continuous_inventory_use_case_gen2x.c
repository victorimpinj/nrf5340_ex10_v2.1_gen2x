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

#include "ex10_api/ex10_macros.h"
#include "ex10_api/ex10_protocol.h"

#include "include_gen2x/ex10_api/application_register_field_enums_gen2x.h"
#include "include_gen2x/ex10_use_cases/ex10_continuous_inventory_use_case_gen2x.h"

static struct Ex10Result continuous_inventory(
    struct Ex10ContinuousInventoryUseCaseParametersGen2X* params)
{
    if (params == NULL || params->stop_conditions == NULL)
    {
        return make_ex10_sdk_error(Ex10ModuleUseCase, Ex10SdkErrorNullPointer);
    }

    struct RegisterInfo const* const regs[] = {&scan_command_control_reg,
                                               &scan_id_command_control_reg};

    struct ScanCommandControlFields scan_control = {
        .n          = 0xF,
        .code       = (uint8_t)params->code,
        .cr         = (uint8_t)params->cr,
        .protection = (uint8_t)params->cr_protection,
        .id         = (uint8_t)params->id,
        .crypto     = params->crypto,
        .rfu        = 0};

    struct ScanIdCommandControlFields scan_id_control = {
        .scan_id_enable = params->scan_id_enable,
        .app_size       = (uint8_t)params->app_size,
        .Reserved0      = 0,
        .app_id         = params->app_id};

    void const* buffers[] = {&scan_control, &scan_id_control};

    struct Ex10Result result =
        get_ex10_protocol()->write_multiple(regs, buffers, ARRAY_SIZE(regs));
    if (result.error)
    {
        return result;
    }

    struct Ex10ContinuousInventoryUseCaseParameters ciucp = {
        .antenna         = params->antenna,
        .rf_mode         = (enum RfModes)params->rf_mode,
        .tx_power_cdbm   = params->tx_power_cdbm,
        .initial_q       = params->initial_q,
        .session         = params->session,
        .target          = params->target,
        .select          = params->select,
        .send_selects    = params->send_selects,
        .stop_conditions = params->stop_conditions,
        .dual_target     = params->dual_target};

    return get_ex10_continuous_inventory_use_case()->continuous_inventory(
        &ciucp);
}

// clang-format off
// These pointers are populated at runtime in the get_ call below.
static struct Ex10ContinuousInventoryUseCaseGen2X ex10_continuous_inventory_use_case_gen2x = {
    .init                                 = NULL,
    .deinit                               = NULL,
    .register_packet_subscriber_callback  = NULL,
    .enable_packet_filter                 = NULL,
    .enable_auto_access                   = NULL,
    .enable_abort_on_fail                 = NULL,
    .enable_tag_focus                     = NULL,
    .enable_tag_read_extended_packet      = NULL,
    .continuous_inventory                 = continuous_inventory,
    .get_continuous_inventory_stop_reason = NULL,
};
// clang-format on

struct Ex10ContinuousInventoryUseCaseGen2X const*
    get_ex10_continuous_inventory_use_case_gen2x(void)
{
    // the functionality for these functions is just to pass
    // it through to the continuous inventory use case, so we
    // populate this use cases pointers with those at runtime
    struct Ex10ContinuousInventoryUseCase const* ciuc =
        get_ex10_continuous_inventory_use_case();

    // a pointer to the struct with a shorter name
    // for readability
    struct Ex10ContinuousInventoryUseCaseGen2X* ciucg =
        &ex10_continuous_inventory_use_case_gen2x;

    ciucg->init   = ciuc->init;
    ciucg->deinit = ciuc->deinit;
    ciucg->register_packet_subscriber_callback =
        ciuc->register_packet_subscriber_callback;
    ciucg->enable_packet_filter = ciuc->enable_packet_filter;
    ciucg->enable_auto_access   = ciuc->enable_auto_access;
    ciucg->enable_abort_on_fail = ciuc->enable_abort_on_fail;
    ciucg->enable_tag_focus     = ciuc->enable_tag_focus;
    ciucg->enable_tag_read_extended_packet =
        ciuc->enable_tag_read_extended_packet;
    ciucg->get_continuous_inventory_stop_reason =
        ciuc->get_continuous_inventory_stop_reason;

    return &ex10_continuous_inventory_use_case_gen2x;
}
