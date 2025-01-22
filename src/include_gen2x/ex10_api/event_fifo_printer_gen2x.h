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

#include "ex10_api/event_fifo_printer.h"

#include "include_gen2x/ex10_api/event_fifo_packet_types_gen2x.h"

#ifdef __cplusplus
extern "C" {
#endif

const struct Ex10EventFifoPrinter* get_ex10_event_fifo_printer_gen2x(void);

#ifdef __cplusplus
}
#endif
