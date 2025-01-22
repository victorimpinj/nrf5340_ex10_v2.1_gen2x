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

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

#include "board/spi_driver.h"
#include "ex10_api/ex10_print.h"
#include <assert.h>

// SPI master functionality
const struct device *spi_dev;
static struct k_poll_signal spi_done_sig = K_POLL_SIGNAL_INITIALIZER(spi_done_sig);

// SPI config.

// Automatice CS control is very hard to configure in current SDK so manually controlling it for now.
#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)
const struct gpio_dt_spec GPO_SPI_CS_N = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, e910_spi_cs_gpios);

// Silly Nordic SPI driver does not update it's config if the config pointers match the last thing it knew so we have to swap structures when the config changes.
// This happens when the SPI speed gets changed.
static struct spi_config spi_cfg_0 = {
	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_MODE_CPHA,
	.frequency = 1000000,
	.slave = 0,
};

static struct spi_config spi_cfg_1 = {
	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_MODE_CPHA,
	.frequency = 1000000,
	.slave = 0,
};

static int enabled_spi_config_idx = 0;
struct spi_config *spi_cfg() {
    return enabled_spi_config_idx ? &spi_cfg_1 : &spi_cfg_0;
}

static bool isGpioInitted = false;

static int32_t nrf5340_spi_open(uint32_t clock_freq_hz)
{
	spi_dev = DEVICE_DT_GET(DT_NODELABEL(spiex10));
	if(!device_is_ready(spi_dev)) {
		printk("SPI master device not ready!\n");
        return -1;
	}

    // Init CS GPIO
    if (!isGpioInitted) {
        gpio_pin_configure_dt(&GPO_SPI_CS_N, GPIO_OUTPUT_INACTIVE | GPIO_ACTIVE_LOW);
        isGpioInitted = true;
    }
    gpio_pin_set_dt(&GPO_SPI_CS_N, 0);

    // Switch config structures so driver will re-read it then update speed.
    enabled_spi_config_idx = !enabled_spi_config_idx;
    spi_cfg_0.frequency = clock_freq_hz;
    spi_cfg_1.frequency = clock_freq_hz;

    return 0;
}

static void nrf5340_spi_close(void)
{
}

static int32_t nrf5340_spi_write(const void* tx_buff, size_t length)
{
    int32_t result = length;

    // Perform SPI write operation
    struct spi_buf tx_buf = {
        .buf = tx_buff,
        .len = length,
    };

    struct spi_buf_set tx = {
        .buffers = &tx_buf,
        .count = 1,
    };

	// Start write transaction
    gpio_pin_set_dt(&GPO_SPI_CS_N, 1);
    int error;
#ifdef CONFIG_SPI_ASYNC
    error = spi_write_signal(spi_dev, spi_cfg(), &tx, &spi_done_sig);
	if(error != 0){
		printk("SPI write error: %i\n", error);
		result = -1;
    } else {
        // Wait for transaction to end.
        int spi_signaled, spi_result;
        k_poll_signal_check(&spi_done_sig, &spi_signaled, &spi_result);
        while (spi_signaled == 0) {
            k_usleep(50);
            k_poll_signal_check(&spi_done_sig, &spi_signaled, &spi_result);
        }
        result = spi_result ? -1 : length;
    }
#else
    error = spi_write(spi_dev, spi_cfg(), &tx);	
	if(error != 0){
		printk("SPI write error: %i\n", error);
		result = -1;
    }
#endif /* CONFIG_SPI_ASYNC */

    gpio_pin_set_dt(&GPO_SPI_CS_N, 0);
	return result;
}

static int32_t nrf5340_spi_read(void* rx_buff, size_t length)
{
    int32_t result = length;
/*
    uint8_t tx_buff[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    // Perform SPI write operation
    struct spi_buf tx_buf = {
        .buf = &tx_buff,
        .len = length,
    };

    struct spi_buf_set tx = {
        .buffers = &tx_buf,
        .count = 1,
    };
*/

    // Perform SPI read operation
    struct spi_buf rx_buf = {
        .buf = rx_buff,
        .len = length,
    };

    struct spi_buf_set rx = {
        .buffers = &rx_buf,
        .count = 1,
    };

	// Start read transaction
    gpio_pin_set_dt(&GPO_SPI_CS_N, 1);
    int error;
#ifdef CONFIG_SPI_ASYNC
//    error = spi_transceive_signal(spi_dev, spi_cfg(), &tx, &rx, &spi_done_sig);
    error = spi_read_signal(spi_dev, spi_cfg(), &rx, &spi_done_sig);
	if (error != 0){
		printk("SPI read error: %i\n", error);
		result = -1;
    } else {
        // Wait for transaction to end.
        int spi_signaled, spi_result;
        k_poll_signal_check(&spi_done_sig, &spi_signaled, &spi_result);
        while(spi_signaled == 0) {
            k_usleep(50);
            k_poll_signal_check(&spi_done_sig, &spi_signaled, &spi_result);
        }
        result = spi_result ? -1 : length;
    }
#else
    error = spi_read(spi_dev, spi_cfg(), &rx);	
	if (error != 0){
		printk("SPI read error: %i\n", error);
		result = -1;
    }
#endif /* CONFIG_SPI_ASYNC */
    gpio_pin_set_dt(&GPO_SPI_CS_N, 0);
	return result;
}

static struct Ex10SpiDriver const ex10_spi_driver = {
    .spi_open  = nrf5340_spi_open,
    .spi_close = nrf5340_spi_close,
    .spi_write = nrf5340_spi_write,
    .spi_read  = nrf5340_spi_read,
};

struct Ex10SpiDriver const* get_ex10_spi_driver(void)
{
    return &ex10_spi_driver;
}
