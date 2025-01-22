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

#include "include_gen2x/ex10_use_cases/ex10_tag_access_use_case_gen2x.h"

static struct Ex10Result run_inventory(
    struct Ex10TagAccessUseCaseParametersGen2X* params)
{
    if (params == NULL)
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

    struct Ex10TagAccessUseCaseParameters taucp = {
        .antenna       = params->antenna,
        .rf_mode       = (enum RfModes)params->rf_mode,
        .tx_power_cdbm = params->tx_power_cdbm,
        .initial_q     = params->initial_q,
        .session       = params->session,
        .target        = params->target,
        .select        = params->select,
        .send_selects  = params->send_selects};

    return get_ex10_tag_access_use_case()->run_inventory(&taucp);
}


static struct Ex10TagAccessUseCaseGen2X ex10_tag_access_use_case_gen2x = {
    .init                     = NULL,
    .deinit                   = NULL,
    .register_halted_callback = NULL,
    .run_inventory            = run_inventory,
    .execute_access_commands  = NULL,
    .get_fifo_packet          = NULL,
    .remove_fifo_packet       = NULL,
    .remove_halted_packet     = NULL,
    .get_inventory_timeout_us = NULL,
    .set_inventory_timeout_us = NULL,
};

struct Ex10TagAccessUseCaseGen2X const* get_ex10_tag_access_use_case_gen2x(void)
{
    // the functionality for these functions is just to pass
    // it through to the tag access use case, so we
    // populate this use cases pointers with those at runtime
    struct Ex10TagAccessUseCase const* tauc = get_ex10_tag_access_use_case();
    // a pointer to the struct with a shorter name
    // for readability
    struct Ex10TagAccessUseCaseGen2X* taguc = &ex10_tag_access_use_case_gen2x;

    taguc->init                     = tauc->init;
    taguc->deinit                   = tauc->deinit;
    taguc->register_halted_callback = tauc->register_halted_callback;
    taguc->execute_access_commands  = tauc->execute_access_commands;
    taguc->get_fifo_packet          = tauc->get_fifo_packet;
    taguc->remove_fifo_packet       = tauc->remove_fifo_packet;
    taguc->remove_halted_packet     = tauc->remove_halted_packet;
    taguc->get_inventory_timeout_us = tauc->get_inventory_timeout_us;
    taguc->set_inventory_timeout_us = tauc->set_inventory_timeout_us;

    return &ex10_tag_access_use_case_gen2x;
}
