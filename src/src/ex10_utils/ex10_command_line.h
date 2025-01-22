/*****************************************************************************
 *                  IMPINJ CONFIDENTIAL AND PROPRIETARY                      *
 *                                                                           *
 * This source code is the property of Impinj, Inc. Your use of this source  *
 * code in whole or in part is subject to your applicable license terms      *
 * from Impinj.                                                              *
 * Contact support@impinj.com for a copy of the applicable Impinj license    *
 * terms.                                                                    *
 *                                                                           *
 * (c) Copyright 2022 - 2024 Impinj, Inc. All rights reserved.               *
 *                                                                           *
 *****************************************************************************/

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ex10_api/ex10_result.h"

#ifdef __cplusplus
extern "C" {
#endif

enum Verbosity
{
    SILENCE             = 0,
    SIMPLE              = 1,
    PRINT_SCOPED_EVENTS = 2,
    PRINT_EVERYTHING    = 3,
};

enum Ex10CommandLineArgumentType
{
    Ex10CommandLineArgumentUninitialized = 0,
    Ex10CommandLineArgumentChar,
    Ex10CommandLineArgumentInt32,
    Ex10CommandLineArgumentUint32,
    Ex10CommandLineArgumentFloat,
    Ex10CommandLineArgumentBool,
    Ex10CommandLineArgumentSetTrue,
    Ex10CommandLineArgumentSetFalse,
    Ex10CommandLineArgumentString,
};

union Ex10CommandLineArgumentValue {
    char        character;
    int32_t     int32;
    uint32_t    uint32;
    float       floater;
    bool        boolean;
    char const* string;
};

#define MAX_SPECIFIER_COUNT ((size_t)(2u))
#define MAX_SPECIFIER_LENGTH ((size_t)(24u))

struct Ex10CommandLineArgument
{
    // An array of strings, which when encountered will trigger this argument
    // node to be parsed. char const* const specifiers[] = {"--region", "-r"};
    // When either of these strings are encountered, the following argument will
    // be parsed as a region name.
    char const* specifiers[MAX_SPECIFIER_COUNT];

    /// The number of strings in the specifiers array.
    size_t specifiers_size;

    // Specify how the command line argument should be parsed.
    enum Ex10CommandLineArgumentType type;

    /// If non-NULL, then provides a description for use by
    /// ex10_print_command_line_usage().
    char const* description;
};

struct Ex10CommandLineArgumentNode
{
    struct Ex10CommandLineArgument      arg;
    bool                                is_parsed;
    union Ex10CommandLineArgumentValue  parsed_value;
    struct Ex10CommandLineArgumentNode* next_node;
};

struct Ex10CommandLineParser
{
    /**
     * Clears all the command line nodes from the linked list managed within
     * Ex10CommandLineArgumentNode. Only the help and verbosity level nodes will
     * remain.
     */
    void (*ex10_clear_all_command_line)(void);

    /**
     * Append a new command line node to the linked list managed within
     * Ex10CommandLineArgumentNode.
     *
     * @param new_node New command line argument node to add to the linked list.
     */
    void (*ex10_append_command_line_node)(
        struct Ex10CommandLineArgumentNode* new_node);

    /**
     * Parse the command line arguments from main().
     *
     * Each node in the arguments array specifies how a command line argument is
     * to be parsed into this linked list.  If a command line specifier is
     * encountered, then the associated node in this linked list is set to the
     * parsed value.  If no value is found which matches the specifier, then the
     * default value is used (i.e. the value element within the node is
     * unchanged).
     *
     * @param argc            The number of arguments to parse.
     * @param argv            The argument list passed into main.
     */
    struct Ex10Result (*ex10_parse_command_line)(int                argc,
                                                 char const* const* argv);

    /**
     * Eprint the specifier name and description of all the arguments in
     * the Ex10CommandLineArgumentNode linked list.
     */
    void (*ex10_eprint_command_line_usage)(void);

    /**
     * Eprint the specifier name and parsed value of all the arguments in
     * the Ex10CommandLineArgumentNode linked list.
     */
    void (*ex10_eprint_command_line_values)(void);

    /**
     * A getter for the help requested command line.
     *
     * @retval true If the user requested the help command.
     */
    bool (*ex10_command_line_help_requested)(void);

    /**
     * A getter for the verbosity level command line.
     *
     * @return enum Verbosity The verbosity level.
     */
    enum Verbosity (*ex10_command_line_verbosity)(void);
};

struct Ex10CommandLineParser const* get_ex10_command_line_parser(void);

#ifdef __cplusplus
}
#endif
