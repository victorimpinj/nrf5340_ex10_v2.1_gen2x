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

#include "ex10_command_line.h"
#include "ex10_inventory_command_line.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Parse the user input command lines and update the new inventory options to
 * the passed-in inventory_options.  The inventory options parameter must be
 * initialized with default values upon being passed in as a parameter.
 * Finally, inventory options are validated and command line usage is printed
 * if any are invalid.
 *
 * @param inventory_options Inventory options filled with default inventory
 *                          options setting. After the command line is parsed,
 *                          it will be filled with new inventory option values.
 * @param argc              The number of command line arguments including the
 *                          name of the program.
 * @param argv              An array that lists all the arguments.
 * @return struct Ex10Result The validty of the inventory_options.
 */
struct Ex10Result ex10_parse_inventory_command_line(
    struct InventoryOptions* inventory_options,
    int                      argc,
    char const* const        argv[]);

/**
 * A getter for the verbosity level command line.
 *
 * @return enum Verbosity The verbosity level.
 */
enum Verbosity ex10_inventory_command_line_verbosity(void);

/**
 * A getter for the help requested command line.
 *
 * @return true The help command was requested from the user.
 * @return false The help command was not requested.
 */
bool ex10_inventory_command_line_help_requested(void);

#ifdef __cplusplus
}
#endif
