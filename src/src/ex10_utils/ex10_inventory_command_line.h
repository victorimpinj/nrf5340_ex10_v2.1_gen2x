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

#pragma once

#include <assert.h>

#include "ex10_api/ex10_autoset_modes.h"
#include "include_gen2x/ex10_api/application_register_field_enums_gen2x.h"

#include "ex10_command_line.h"

#ifdef __cplusplus
extern "C" {
#endif

union InventoryMode {
    enum RfModes       rf_mode_id;
    enum AutoSetModeId autoset_mode_id;
    uint32_t           raw;
};

// If these static assert()s fail,
// then adjust the InventoryMode.raw member type accordingly.
static_assert(sizeof(enum RfModes) <= sizeof(uint32_t), "");
static_assert(sizeof(enum AutoSetModeId) <= sizeof(uint32_t), "");

struct InventoryOptions
{
    char const*                       region_name;
    uint32_t                          read_rate;
    uint8_t                           antenna;
    uint32_t                          frequency_khz;
    bool                              remain_on;
    int16_t                           tx_power_cdbm;
    union InventoryMode               mode;
    char                              target_spec;
    uint8_t                           initial_q;
    enum InventoryRoundControlSession session;
    enum ScanCommandControlCrGen2X          cr;
    enum ScanCommandControlProtectionGen2X  protection;
    enum ScanCommandControlIdGen2X          id;
};

struct Ex10InventoryCommandLine
{
    /**
     * Append all predefined inventory command line argument nodes to the
     * ex10_command_line parser.  This function must be called before executing
     * any other functions within this module.
     */
    void (*ex10_append_inventory_command_line)(void);

    /**
     * This function should be called after the parsing of user input by the
     * ex10_command_line module. The inventory options parameter must be
     * initialized with default values upon being passed in as a parameter.
     * This function updates the given inventory options struct based on the
     * parsed command line arguments and validates it. In case the parsed
     * argument value is invalid, the function will return an ex10_result error.
     *
     * @param options Inventory options. Should be filled with default inventory
     *                option values.
     * @return struct Ex10Result The result of the inventory options validity
     * check.
     */
    struct Ex10Result (*ex10_update_inventory_options)(
        struct InventoryOptions* options);

    /**
     * Print the name and value of the given inventory options.
     *
     * @param options Inventory options to print.
     */
    void (*ex10_print_inventory_options)(struct InventoryOptions* options);
};

struct Ex10InventoryCommandLine const* get_ex10_inventory_command_line(void);

#ifdef __cplusplus
}
#endif
