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

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ex10_api/event_fifo_packet_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// IPJ_autogen | gen_c_app_ex10_api_fifo_types_gen2x {
// clang-format off
#pragma pack(push, 1)

/**
 * @struct TagReadExtendedGen2X
 * Contains fields from a TagReadExtendedGen2X packet in the EventFifo
*/
struct TagReadExtendedGen2X
{
    uint16_t rssi;
    uint16_t rf_phase_begin;
    uint16_t rf_phase_end;
    uint16_t rx_gain_settings;
    uint8_t type;
    uint8_t tid_offset;
    bool halted_on_tag : 1;
    bool memory_parity_err : 1;
    uint8_t packet_rfu_1 : 6;
    uint8_t packet_rfu_2;
    uint8_t cr : 3;
    uint8_t protection : 3;
    uint8_t packet_rfu_3 : 2;
    uint8_t id : 3;
    uint8_t packet_rfu_4 : 5;
    uint16_t packet_rfu_5;
    uint32_t cr_value;
};
static_assert(sizeof(struct TagReadExtendedGen2X) == sizeof(struct TagReadExtended),
              "Size of TagReadExtendedGen2X packet type incorrect");
static_assert(offsetof(struct TagReadExtendedGen2X, rssi) == offsetof(struct TagReadExtended, rssi),
              "Offset of rssi does not match");
static_assert(offsetof(struct TagReadExtendedGen2X, rf_phase_begin) == offsetof(struct TagReadExtended, rf_phase_begin),
              "Offset of rf_phase_begin does not match");
static_assert(offsetof(struct TagReadExtendedGen2X, rf_phase_end) == offsetof(struct TagReadExtended, rf_phase_end),
              "Offset of rf_phase_end does not match");
static_assert(offsetof(struct TagReadExtendedGen2X, rx_gain_settings) == offsetof(struct TagReadExtended, rx_gain_settings),
              "Offset of rx_gain_settings does not match");
static_assert(offsetof(struct TagReadExtendedGen2X, type) == offsetof(struct TagReadExtended, type),
              "Offset of type does not match");
static_assert(offsetof(struct TagReadExtendedGen2X, tid_offset) == offsetof(struct TagReadExtended, tid_offset),
              "Offset of tid_offset does not match");
static_assert(offsetof(struct TagReadExtendedGen2X, cr_value) == offsetof(struct TagReadExtended, cr_value),
              "Offset of cr_value does not match");

/**
 * @enum TagReadCr
 * The type of the backscattered CR
*/
enum TagReadCr
{
    TagReadCrId32      = 0,
    TagReadCrId16      = 1,
    TagReadCrStoredCrc = 2,
    TagReadCrRn16      = 3,
};

/**
 * @enum TagReadProtection
 * The type of tag Protection to its backscattered CR
*/
enum TagReadProtection
{
    TagReadProtectionNone     = 0,
    TagReadProtectionParity   = 1,
    TagReadProtectionCrc5     = 2,
    TagReadProtectionCrc5Plus = 3,
};

/**
 * @enum TagReadId
 * Tag’s ACK response type in the reply state
*/
enum TagReadId
{
    TagReadIdNone   = 0,
    TagReadIdTmnTsn = 1,
    TagReadIdPart   = 2,
    TagReadIdFull   = 3,
};

#pragma pack(pop)
// clang-format on
// IPJ_autogen }

#ifdef __cplusplus
}
#endif
