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

#include <stdbool.h>
#include <string.h>

#include "ex10_api/ex10_inventory.h"
#include "ex10_api/ex10_macros.h"
#include "ex10_api/ex10_protocol.h"

#include "include_gen2x/ex10_api/application_register_definitions_gen2x.h"
#include "include_gen2x/ex10_api/application_registers_gen2x.h"
#include "include_gen2x/ex10_api/ex10_inventory_gen2x.h"

static struct Ex10Result run_inventory(
    struct InventoryRoundControlFields const*   inventory_config,
    struct InventoryRoundControl_2Fields const* inventory_config_2,
    struct ScanCommandControlFields const*      scan_control,
    struct ScanIdCommandControlFields const*    scan_id_control,
    bool                                        send_selects)
{
    struct RegisterInfo const* const regs[] = {&scan_command_control_reg,
                                               &scan_id_command_control_reg};

    void const* buffers[] = {scan_control, scan_id_control};

    struct Ex10Result result =
        get_ex10_protocol()->write_multiple(regs, buffers, ARRAY_SIZE(regs));
    if (result.error)
    {
        return result;
    }
    return get_ex10_inventory()->run_inventory(
        inventory_config, inventory_config_2, send_selects);
}

static struct Ex10Result start_inventory(
    uint8_t                                     antenna,
    enum RfModes                                rf_mode,
    int16_t                                     tx_power_cdbm,
    struct InventoryRoundControlFields const*   inventory_config,
    struct InventoryRoundControl_2Fields const* inventory_config_2,
    struct ScanCommandControlFields const*      scan_control,
    struct ScanIdCommandControlFields const*    scan_id_control,
    bool                                        send_selects)
{
    struct RegisterInfo const* const regs[] = {&scan_command_control_reg,
                                               &scan_id_command_control_reg};

    void const* buffers[] = {scan_control, scan_id_control};

    struct Ex10Result result =
        get_ex10_protocol()->write_multiple(regs, buffers, ARRAY_SIZE(regs));
    if (result.error)
    {
        return result;
    }
    return get_ex10_inventory()->start_inventory(antenna,
                                                 rf_mode,
                                                 tx_power_cdbm,
                                                 inventory_config,
                                                 inventory_config_2,
                                                 send_selects);
}


static bool inventory_halted(void)
{
    return get_ex10_inventory()->inventory_halted();
}

static enum StopReason ex10_result_to_continuous_inventory_error(
    struct Ex10Result ex10_result)
{
    return get_ex10_inventory()->ex10_result_to_continuous_inventory_error(
        ex10_result);
}

static void enable_crc5_and_crc5plus(bool enable)
{
    struct Gen2XFeaturesControlFields const feature_data = {
        .accept_crc5_and_crc5_plus = enable, .rfu = 0};
    get_ex10_protocol()->write(&gen2_x_features_control_reg, &feature_data);
}

static const struct Ex10InventoryGen2X ex10_inventory_gen2x = {
    .run_inventory    = run_inventory,
    .start_inventory  = start_inventory,
    .inventory_halted = inventory_halted,
    .ex10_result_to_continuous_inventory_error =
        ex10_result_to_continuous_inventory_error,
    .enable_crc5_and_crc5plus = enable_crc5_and_crc5plus,
};

const struct Ex10InventoryGen2X* get_ex10_inventory_gen2x(void)
{
    return &ex10_inventory_gen2x;
}
