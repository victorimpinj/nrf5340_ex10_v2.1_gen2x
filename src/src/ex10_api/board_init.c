/*****************************************************************************
 *                  IMPINJ CONFIDENTIAL AND PROPRIETARY                      *
 *                                                                           *
 * This source code is the property of Impinj, Inc. Your use of this source  *
 * code in whole or in part is subject to your applicable license terms      *
 * from Impinj.                                                              *
 * Contact support@impinj.com for a copy of the applicable Impinj license    *
 * terms.                                                                    *
 *                                                                           *
 * (c) Copyright 2020 - 2024 Impinj, Inc. All rights reserved.               *
 *                                                                           *
 *****************************************************************************/

#include <stdlib.h>

#include "ex10_api/board_init.h"
#include "ex10_api/board_init_core.h"

#include "board/driver_list.h"
#include "board/fifo_buffer_pool.h"
#include "board/uart_helpers.h"
#include "calibration.h"
#include "ex10_api/ex10_helpers.h"
#include "ex10_api/ex10_regulatory.h"
#include "ex10_api/ex10_rf_power.h"
#include "ex10_api/power_transactor.h"

void ex10_board_gpio_init(struct Ex10GpioInterface const* gpio_if)
{
    ex10_core_board_gpio_init(gpio_if);
}

struct Ex10Result ex10_bootloader_board_setup(uint32_t spi_clock_hz)
{
    return ex10_bootloader_core_board_setup(spi_clock_hz);
}

void ex10_typical_board_uart_setup(enum AllowedBpsRates bitrate)
{
    struct Ex10DriverList const* driver_list = get_ex10_board_driver_list();
    driver_list->uart_if.open(bitrate);
    get_ex10_uart_helper()->init(driver_list);
}

void ex10_bootloader_board_teardown(void)
{
    ex10_bootloader_core_board_teardown();
}

void ex10_typical_board_uart_teardown(void)
{
    get_ex10_uart_helper()->deinit();
    get_ex10_board_driver_list()->uart_if.close();
}
