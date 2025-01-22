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


#include "ex10_api/ex10_inventory.h"
#include "ex10_api/ex10_print.h"
#include "ex10_regulatory/ex10_default_region_names.h"

#include "ex10_inventory_command_line.h"
#include "ex10_use_case_example_errors.h"


// clang-format off
static struct Ex10CommandLineArgumentNode node_region = {
    .arg = {
        .specifiers      = {"-r", "--region"},
        .specifiers_size = 2u,
        .type            = Ex10CommandLineArgumentString,
        .description     =
            "Set the regulatory region; i.e. 'FCC', 'ETSI_LOWER', ..."},
    .is_parsed    = false,
    .parsed_value = {.string = NULL},
    .next_node    = NULL,
};

static struct Ex10CommandLineArgumentNode node_read_rate = {
    .arg = {
        .specifiers      = {"--read_rate"},
        .specifiers_size = 1u,
        .type            = Ex10CommandLineArgumentUint32,
        .description     = "Set the minimum read rate limit, tags/second"},
    .is_parsed    = false,
    .parsed_value = {.uint32 = 0u},
    .next_node    = NULL,
};

static struct Ex10CommandLineArgumentNode node_antenna = {
    .arg = {
        .specifiers      = {"-a", "--antenna"},
        .specifiers_size = 2u,
        .type            = Ex10CommandLineArgumentUint32,
        .description     = "Select an antenna port"},
    .is_parsed    = false,
    .parsed_value = {.uint32 = 0u},
    .next_node    = NULL,
};

static struct Ex10CommandLineArgumentNode node_frequency_khz = {
    .arg = {
        .specifiers      = {"-f", "--frequency_khz"},
        .specifiers_size = 2u,
        .type            = Ex10CommandLineArgumentUint32,
        .description     = "Set the LO frequency in kHz"},
    .is_parsed    = false,
    .parsed_value = {.uint32 = 0u},
    .next_node    = NULL,
};

static struct Ex10CommandLineArgumentNode node_remain_on = {
    .arg = {
        .specifiers      = {"-R", "--remain_on"},
        .specifiers_size = 2u,
        .type            = Ex10CommandLineArgumentSetTrue,
        .description     = "Ignore regulatory timers"},
    .is_parsed    = false,
    .parsed_value = {.boolean = false},
    .next_node    = NULL,
};

static struct Ex10CommandLineArgumentNode node_tx_power_cdbm = {
    .arg = {
        .specifiers      = {"-p", "--tx_power_cdbm"},
        .specifiers_size = 2u,
        .type            = Ex10CommandLineArgumentInt32,
        .description     = "Set the transmitter power in cdBm"},
    .is_parsed    = false,
    .parsed_value = {.int32 = 0},
    .next_node    = NULL,
};

static struct Ex10CommandLineArgumentNode node_mode = {
    .arg = {
        .specifiers      = {"-m", "--mode"},
        .specifiers_size = 2u,
        .type            = Ex10CommandLineArgumentUint32,
        .description     =
            "Set the autoset or RF mode (depends on the example)"},
    .is_parsed    = false,
    .parsed_value = {.uint32 = 0u},
    .next_node    = NULL,
};

static struct Ex10CommandLineArgumentNode node_target = {
    .arg = {
        .specifiers      = {"-t", "--target"},
        .specifiers_size = 2u,
        .type            = Ex10CommandLineArgumentChar,
        .description     =
            "Set the inventory target to run; i.e. 'D' for dual, 'A' or 'B' for single."},
    .is_parsed    = false,
    .parsed_value = {.character = 'D'},
    .next_node    = NULL,
};

static struct Ex10CommandLineArgumentNode node_initial_q = {
    .arg = {
        .specifiers      = {"-q", "--initial_q"},
        .specifiers_size = 2u,
        .type            = Ex10CommandLineArgumentUint32,
        .description     = "Set the inventory initial Q value"},
    .is_parsed    = false,
    .parsed_value = {.uint32 = 0u},
    .next_node    = NULL,
};

static struct Ex10CommandLineArgumentNode node_session = {
    .arg = {
        .specifiers      = {"-s", "--session"},
        .specifiers_size = 2u,
        .type            = Ex10CommandLineArgumentUint32,
        .description     = "Set the inventory session to use"},
    .is_parsed    = false,
    .parsed_value = {.uint32 = 0u},
    .next_node    = NULL,
};
// clang-format on


/**
 * @brief
 *
 * @param target_spec
 * @return true
 * @return false
 */
static bool is_valid_target_spec(char target_spec)
{
    return target_spec == 'A' || target_spec == 'B' || target_spec == 'D';
}

/**
 * @brief
 *
 * @param options
 * @return struct Ex10Result
 */
static struct Ex10Result check_inventory_command_line(
    struct InventoryOptions options)
{
    bool parsed_ok = true;
    if (options.antenna < 1 || options.antenna > 2)
    {
        ex10_ex_eprintf("Invalid antenna: %u\n", options.antenna);
        parsed_ok = false;
    }

    if (options.session > SessionS3)
    {
        ex10_ex_eprintf("Invalid session: %u\n", (uint8_t)options.session);
        parsed_ok = false;
    }

    if (is_valid_target_spec(options.target_spec) == false)
    {
        ex10_ex_eprintf("Invalid target specified: %c\n", options.target_spec);
        parsed_ok = false;
    }

    enum Ex10RegionId const region_id =
        get_ex10_default_region_names()->get_region_id(options.region_name);
    if (region_id == REGION_NOT_DEFINED)
    {
        parsed_ok = false;
        ex10_ex_eprintf("Invalid region specified: %s\n", options.region_name);
    }

    if (parsed_ok == false)
    {
        return make_ex10_app_error(Ex10ApplicationCommandLineBadParamValue);
    }

    return make_ex10_success();
}

static void ex10_append_inventory_command_line(void)
{
    struct Ex10CommandLineParser const* ex10_cl =
        get_ex10_command_line_parser();
    ex10_cl->ex10_append_command_line_node(&node_region);
    ex10_cl->ex10_append_command_line_node(&node_read_rate);
    ex10_cl->ex10_append_command_line_node(&node_antenna);
    ex10_cl->ex10_append_command_line_node(&node_frequency_khz);
    ex10_cl->ex10_append_command_line_node(&node_remain_on);
    ex10_cl->ex10_append_command_line_node(&node_tx_power_cdbm);
    ex10_cl->ex10_append_command_line_node(&node_mode);
    ex10_cl->ex10_append_command_line_node(&node_target);
    ex10_cl->ex10_append_command_line_node(&node_initial_q);
    ex10_cl->ex10_append_command_line_node(&node_session);
}

static struct Ex10Result ex10_update_inventory_options(
    struct InventoryOptions* options)
{
    if (node_region.is_parsed)
    {
        options->region_name = node_region.parsed_value.string;
    }

    if (node_read_rate.is_parsed)
    {
        options->read_rate = node_read_rate.parsed_value.uint32;
    }

    if (node_antenna.is_parsed)
    {
        options->antenna = (uint8_t)node_antenna.parsed_value.uint32;
    }

    if (node_frequency_khz.is_parsed)
    {
        options->frequency_khz = node_frequency_khz.parsed_value.uint32;
    }

    if (node_remain_on.is_parsed)
    {
        options->remain_on = node_remain_on.parsed_value.boolean;
    }

    if (node_tx_power_cdbm.is_parsed)
    {
        options->tx_power_cdbm = (int16_t)node_tx_power_cdbm.parsed_value.int32;
    }

    if (node_mode.is_parsed)
    {
        options->mode.raw = node_mode.parsed_value.uint32;
    }

    if (node_target.is_parsed)
    {
        options->target_spec = node_target.parsed_value.character;
    }

    if (node_initial_q.is_parsed)
    {
        options->initial_q = (uint8_t)node_initial_q.parsed_value.uint32;
    }

    if (node_session.is_parsed)
    {
        options->session =
            (enum InventoryRoundControlSession)node_session.parsed_value.uint32;
    }

    return check_inventory_command_line(*options);
}

static void ex10_print_inventory_options(struct InventoryOptions* options)
{
    ex10_ex_printf("Region      : %s\n", options->region_name);
    ex10_ex_printf("Read rate   : %u tags/s\n", options->read_rate);
    ex10_ex_printf("Antenna     : %u\n", options->antenna);
    ex10_ex_printf("Frequency   : %u kHz\n", options->frequency_khz);
    ex10_ex_printf("Remain on   : %u\n", options->remain_on);
    ex10_ex_printf("Tx power    : %u cdBm\n", options->tx_power_cdbm);
    ex10_ex_printf("Auto/RF mode: %u\n", options->mode.raw);
    ex10_ex_printf("Target      : %c\n", options->target_spec);
    ex10_ex_printf("Session     : %u\n", options->session);
    ex10_ex_printf("Initial Q   : %u\n", options->initial_q);
}

// clang-format off
static struct Ex10InventoryCommandLine ex10_inventory_command_line = {
    .ex10_append_inventory_command_line = ex10_append_inventory_command_line,
    .ex10_update_inventory_options      = ex10_update_inventory_options,
    .ex10_print_inventory_options       = ex10_print_inventory_options,
};
// clang-format on

struct Ex10InventoryCommandLine const* get_ex10_inventory_command_line(void)
{
    return &ex10_inventory_command_line;
}
