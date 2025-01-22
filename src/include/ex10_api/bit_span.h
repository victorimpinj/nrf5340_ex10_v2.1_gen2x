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

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct BitSpan
 * Defines a mutable location of contiguous memory, organized as bits.
 * Each byte in data is utilized, therefore 8 bits from length exist in
 * each byte. The bits and bytes are organized LSB first. This means
 * that bit 0 of byte 0 is the first bit, bit 1 of byte 0 is next, etc.
 */
struct BitSpan
{
    uint8_t* data;    ///< The pointer to beginning of the memory location.
    size_t   length;  ///< The number of bits in the span.
};

/**
 * @struct ConstBitSpan
 * Defines a immutable location of contiguous memory, organized as bits.
 * Each byte in data is utilized, therefore 8 bits from length exist in
 * each byte. The bits and bytes are organized LSB first. This means
 * that bit 0 of byte 0 is the first bit, bit 1 of byte 0 is next, etc.
 */
struct ConstBitSpan
{
    uint8_t const* data;  ///< The pointer to beginning of the memory location.
    size_t         length;  ///< The number of bits in the span.
};

#ifdef __cplusplus
}
#endif
