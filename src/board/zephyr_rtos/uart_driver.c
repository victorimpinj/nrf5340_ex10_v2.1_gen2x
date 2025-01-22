/*****************************************************************************
 *                  IMPINJ CONFIDENTIAL AND PROPRIETARY                      *
 *                                                                           *
 * This source code is the property of Impinj, Inc. Your use of this source  *
 * code in whole or in part is subject to your applicable license terms      *
 * from Impinj.                                                              *
 * Contact support@impinj.com for a copy of the applicable Impinj license    *
 * terms.                                                                    *
 *                                                                           *
 * (c) Copyright 2020 - 2023 Impinj, Inc. All rights reserved.               *
 *                                                                           *
 *****************************************************************************/

/*
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
*/
#include "board/uart_driver.h"
//#include "board/uart_helpers.h"
//#include "ex10_api/ex10_print.h"

static int32_t uart_open(enum AllowedBpsRates bitrate)
{
    return 0;
}

static void uart_close(void)
{
}

static int32_t uart_write(const void* tx_buff, size_t length)
{
    return 0;
}

static int32_t uart_read(void* rx_buff, size_t length)
{
    return (int32_t)0;
}

static struct Ex10UartDriver const ex10_uart_driver = {
    .uart_open  = uart_open,
    .uart_close = uart_close,
    .uart_write = uart_write,
    .uart_read  = uart_read,
};

struct Ex10UartDriver const* get_ex10_uart_driver(void)
{
    return &ex10_uart_driver;
}
