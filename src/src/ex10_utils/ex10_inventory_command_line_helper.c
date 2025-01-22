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

#include "ex10_api/ex10_print.h"

#include "ex10_inventory_command_line_helper.h"


struct Ex10Result ex10_parse_inventory_command_line(
    struct InventoryOptions* inventory_options,
    int                      argc,
    char const* const        argv[])
{
    struct Ex10CommandLineParser const* ex10_cl =
        get_ex10_command_line_parser();
    struct Ex10InventoryCommandLine const* ex10_icl =
        get_ex10_inventory_command_line();

    // Append all the inventory command line nodes to the command line parser
    // linked list.
    ex10_icl->ex10_append_inventory_command_line();

    // Parse command line
    struct Ex10Result ex10_result =
        ex10_cl->ex10_parse_command_line(argc, argv);
    if (ex10_result.error || ex10_cl->ex10_command_line_help_requested())
    {
        ex10_cl->ex10_eprint_command_line_usage();
        return ex10_result;
    }

    ex10_result = ex10_icl->ex10_update_inventory_options(inventory_options);
    ex10_ex_printf("Inventory settings:\n");
    ex10_icl->ex10_print_inventory_options(inventory_options);
    if (ex10_result.error)
    {
        ex10_cl->ex10_eprint_command_line_usage();
        return ex10_result;
    }

    return make_ex10_success();
}

enum Verbosity ex10_inventory_command_line_verbosity(void)
{
    return get_ex10_command_line_parser()->ex10_command_line_verbosity();
}

bool ex10_inventory_command_line_help_requested(void)
{
    return get_ex10_command_line_parser()->ex10_command_line_help_requested();
}
