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

#include <float.h>
#include <stdlib.h>
#include <string.h>

#include "ex10_api/ex10_print.h"
#include "ex10_use_case_example_errors.h"

#include "ex10_command_line.h"

// clang-format off
static struct Ex10CommandLineArgumentNode node_verbose = {
    .arg = {
        .specifiers      = {"-v", "--verbose"},
        .specifiers_size = 2u,
        .type            = Ex10CommandLineArgumentUint32,
        .description     =
            "Set the verbosity level: 0: silent, 1: print tag packets, "
            "2: simple - ToI process", "3: print all packets"},
    .is_parsed    = false,
    .parsed_value = {.uint32 = SILENCE},
    .next_node    = NULL,
};

static struct Ex10CommandLineArgumentNode node_help = {
    .arg = {
        .specifiers      = {"-h", "--help"},
        .specifiers_size = 2u,
        .type            = Ex10CommandLineArgumentSetTrue,
        .description     = "Print this help message"},
    .is_parsed    = false,
    .parsed_value = {.boolean = false},
    .next_node    = &node_verbose,
};
// clang-format on
static struct Ex10CommandLineArgumentNode* argument_head = &node_help;


static int32_t parse_int32(char const* str, int32_t error_value)
{
    if (str == NULL)
    {
        ex10_ex_eprintf("Argument str == NULL\n");
        return error_value;
    }

    char*          endp  = NULL;
    long int const value = strtol(str, &endp, 0);
    if (*endp != 0)
    {
        ex10_ex_eprintf(
            "Parsing %s as signed integer failed, pos: %c\n", str, *endp);
        return error_value;
    }

    return (int32_t)value;
}

static uint32_t parse_uint32(char const* str, uint32_t error_value)
{
    if (str == NULL)
    {
        ex10_ex_eprintf("Argument str == NULL\n");
        return error_value;
    }

    char*                   endp  = NULL;
    long unsigned int const value = strtoul(str, &endp, 0);
    if (*endp != 0)
    {
        ex10_ex_eprintf(
            "Parsing %s as unsigned integer failed, pos: %c\n", str, *endp);
        return error_value;
    }

    return (uint32_t)value;
}

static float parse_float(char const* str, float error_value)
{
    if (str == NULL)
    {
        ex10_ex_eprintf("Argument str == NULL\n");
        return error_value;
    }

    char*       endp  = NULL;
    float const value = strtof(str, &endp);
    if (*endp != 0)
    {
        ex10_ex_eprintf("Parsing %s as float failed, pos: %c\n", str, *endp);
        return error_value;
    }
    return value;
}

/**
 * Search through the array of specifiers and determine if any of the
 * string specifiers match the parameter passed in.
 *
 * @param argument  A pointer to the argument to search for the specifier match.
 * @param specifier A NULL terminated string to search for in the member
 *                  specifiers array.
 *
 * @return bool true if parameter specifier matches one of the strings in
 *              the specifiers member list.
 *              false if no match was found.
 */
static bool has_specifier(struct Ex10CommandLineArgument const* argument,
                          char const*                           specifier)
{
    for (size_t index = 0u; index < argument->specifiers_size; ++index)
    {
        if (argument->specifiers[index] != NULL)
        {
            if (strncmp(argument->specifiers[index],
                        specifier,
                        MAX_SPECIFIER_LENGTH) == 0)
            {
                return true;
            }
        }
    }
    return false;
}

/**
 * Search through the linked list for a node with the matching specifier.
 *
 * @param specifier The argument specifier
 *
 * @return struct Ex10CommandLineArgumentNode* The argument node with the
 *                                             matching specifier.
 * @return NULL If there is no node with the matching specifier.
 */
static struct Ex10CommandLineArgumentNode* get_command_line_node(
    char const* specifier)
{
    struct Ex10CommandLineArgumentNode* temp_node = argument_head;
    while (temp_node != NULL)
    {
        if (has_specifier(&temp_node->arg, specifier))
        {
            return temp_node;
        }
        temp_node = temp_node->next_node;
    }

    ex10_ex_eprintf("Specifier '%s' not found\n", specifier);
    return NULL;
}

static void ex10_clear_all_command_line(void)
{
    argument_head          = &node_help;
    node_help.next_node    = &node_verbose;
    node_verbose.next_node = NULL;
}

static void ex10_append_command_line_node(
    struct Ex10CommandLineArgumentNode* new_node)
{
    if (argument_head == NULL)
    {
        argument_head = new_node;
        return;
    }

    struct Ex10CommandLineArgumentNode* temp_node = argument_head;
    while (temp_node->next_node != NULL)
    {
        temp_node = temp_node->next_node;
    }

    temp_node->next_node = new_node;
    return;
}

static struct Ex10Result ex10_parse_command_line(int                argc,
                                                 char const* const* argv)
{
    // Note: skipping argv[0]; it is the program being run.
    for (int arg_iter = 1u; arg_iter < argc;)
    {
        struct Ex10CommandLineArgumentNode* arg_node =
            get_command_line_node(argv[arg_iter]);

        if (arg_node != NULL)
        {
            switch (arg_node->arg.type)
            {
                case Ex10CommandLineArgumentUninitialized:
                    // This case should never happen.
                    break;
                case Ex10CommandLineArgumentChar:
                    ++arg_iter;
                    if (arg_iter >= argc)
                    {
                        return make_ex10_app_error(
                            Ex10ApplicationCommandLineMissingParamValue);
                    }
                    arg_node->parsed_value.character = *argv[arg_iter];
                    break;
                case Ex10CommandLineArgumentInt32:
                    ++arg_iter;
                    if (arg_iter >= argc)
                    {
                        return make_ex10_app_error(
                            Ex10ApplicationCommandLineMissingParamValue);
                    }
                    arg_node->parsed_value.int32 =
                        parse_int32(argv[arg_iter], INT32_MAX);
                    if (arg_node->parsed_value.int32 == INT32_MAX)
                    {
                        return make_ex10_app_error(
                            Ex10ApplicationCommandLineBadParamValue);
                    }
                    break;
                case Ex10CommandLineArgumentUint32:
                    ++arg_iter;
                    if (arg_iter >= argc)
                    {
                        return make_ex10_app_error(
                            Ex10ApplicationCommandLineMissingParamValue);
                    }
                    arg_node->parsed_value.uint32 =
                        parse_uint32(argv[arg_iter], UINT32_MAX);
                    if (arg_node->parsed_value.uint32 == UINT32_MAX)
                    {
                        return make_ex10_app_error(
                            Ex10ApplicationCommandLineBadParamValue);
                    }
                    break;
                case Ex10CommandLineArgumentFloat:
                    ++arg_iter;
                    if (arg_iter >= argc)
                    {
                        return make_ex10_app_error(
                            Ex10ApplicationCommandLineMissingParamValue);
                    }
                    arg_node->parsed_value.floater =
                        parse_float(argv[arg_iter], FLT_MAX);
                    if (arg_node->parsed_value.floater >= FLT_MAX - FLT_EPSILON)
                    {
                        return make_ex10_app_error(
                            Ex10ApplicationCommandLineBadParamValue);
                    }
                    break;
                case Ex10CommandLineArgumentBool:
                    ++arg_iter;
                    if (arg_iter >= argc)
                    {
                        return make_ex10_app_error(
                            Ex10ApplicationCommandLineMissingParamValue);
                    }
                    arg_node->parsed_value.int32 =
                        parse_int32(argv[arg_iter], INT32_MAX);
                    if (arg_node->parsed_value.int32 == INT32_MAX)
                    {
                        return make_ex10_app_error(
                            Ex10ApplicationCommandLineBadParamValue);
                    }
                    arg_node->parsed_value.boolean =
                        (arg_node->parsed_value.int32 != 0);
                    break;
                case Ex10CommandLineArgumentSetTrue:
                    arg_node->parsed_value.boolean = true;
                    break;
                case Ex10CommandLineArgumentSetFalse:
                    arg_node->parsed_value.boolean = false;
                    break;
                case Ex10CommandLineArgumentString:
                    ++arg_iter;
                    if (arg_iter >= argc)
                    {
                        return make_ex10_app_error(
                            Ex10ApplicationCommandLineMissingParamValue);
                    }
                    arg_node->parsed_value.string = argv[arg_iter];
                    break;
                default:
                    break;
            }
            arg_node->is_parsed = true;
        }
        else
        {
            return make_ex10_app_error(
                Ex10ApplicationCommandLineUnknownSpecifier);
        }
        ++arg_iter;
    }

    return make_ex10_success();
}

static void ex10_eprint_command_line_usage(void)
{
    struct Ex10CommandLineArgumentNode* temp_node = argument_head;
    while (temp_node != NULL)
    {
        struct Ex10CommandLineArgument const* arg = &temp_node->arg;
        for (size_t spec_index = 0u; spec_index < arg->specifiers_size;
             ++spec_index)
        {
            char const* specifier = arg->specifiers[spec_index];
            if (specifier && *specifier)
            {
                if (spec_index > 0u)
                {
                    ex10_ex_eputs(", ");
                }
                ex10_ex_eputs("%s", specifier);
            }
        }
        ex10_ex_eputs("\n");
        if (arg->description)
        {
            ex10_ex_eputs("    %s", arg->description);
        }
        ex10_ex_eputs("\n");

        temp_node = temp_node->next_node;
    }
}

static void ex10_eprint_command_line_values(void)
{
#if defined(EX10_ENABLE_PRINT_EX_ERR)
    struct Ex10CommandLineArgumentNode* temp_node = argument_head;
    while (temp_node != NULL)
    {
        struct Ex10CommandLineArgument const* arg     = &temp_node->arg;
        int                                   n_write = 0;
        for (size_t spec_index = 0u; spec_index < arg->specifiers_size;
             ++spec_index)
        {
            char const* specifier = arg->specifiers[spec_index];
            if (specifier && *specifier)
            {
                if (spec_index > 0u)
                {
                    n_write += ex10_ex_eputs(", ");
                }
                n_write += ex10_ex_eputs("%s", specifier);
            }
        }

        while (n_write < (int)MAX_SPECIFIER_LENGTH)
        {
            n_write += ex10_ex_eputs(" ");
        }
        ex10_ex_eputs(": ");

        switch (arg->type)
        {
            default:
            case Ex10CommandLineArgumentUninitialized:
                ex10_ex_eputs("undefined");
                break;
            case Ex10CommandLineArgumentInt32:
                ex10_ex_eputs("%d", temp_node->parsed_value.int32);
                break;
            case Ex10CommandLineArgumentUint32:
                ex10_ex_eputs("%d", temp_node->parsed_value.uint32);
                break;
            case Ex10CommandLineArgumentFloat:
                ex10_ex_eputs("%.2f", temp_node->parsed_value.floater);
                break;
            case Ex10CommandLineArgumentBool:
            case Ex10CommandLineArgumentSetTrue:
            case Ex10CommandLineArgumentSetFalse:
                ex10_ex_eputs(
                    "%s", temp_node->parsed_value.boolean ? "true" : "false");
                break;
            case Ex10CommandLineArgumentString:
                ex10_ex_eputs("%s", temp_node->parsed_value.string);
                break;
        }
        ex10_ex_eputs("\n");
        temp_node = temp_node->next_node;
    }
#endif
}

static bool ex10_command_line_help_requested(void)
{
    return node_help.parsed_value.boolean;
}

static enum Verbosity ex10_command_line_verbosity(void)
{
    return (enum Verbosity)node_verbose.parsed_value.uint32;
}

static struct Ex10CommandLineParser ex10_command_line = {
    .ex10_clear_all_command_line      = ex10_clear_all_command_line,
    .ex10_append_command_line_node    = ex10_append_command_line_node,
    .ex10_parse_command_line          = ex10_parse_command_line,
    .ex10_eprint_command_line_usage   = ex10_eprint_command_line_usage,
    .ex10_eprint_command_line_values  = ex10_eprint_command_line_values,
    .ex10_command_line_help_requested = ex10_command_line_help_requested,
    .ex10_command_line_verbosity      = ex10_command_line_verbosity};

struct Ex10CommandLineParser const* get_ex10_command_line_parser(void)
{
    return &ex10_command_line;
}
