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

#include "ex10_api/ex10_result.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A simple init for showcasing basic functionality.
 */
struct Ex10Result ex10_simple_example_init(bool simple_callbacks);

#ifdef __cplusplus
}
#endif
