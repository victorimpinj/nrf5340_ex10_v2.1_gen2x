/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>

#include <nrfx_clock.h>

// Rain example entry points
int get_board_version(int argc, char* argv[]);
int spi_test(uint32_t iterations);
int simple_ramping_example();
int continuous_inventory(int min_read_rate);

// RAIN application thread.
static void rain_application_thread(void *arg1, void *arg2, void *arg3) {
	k_thread_name_set(NULL, "rain_app_thread");
	
	// Uncomment the example(s) you want to run here.
	//while(true) {
//		get_board_version(0, 0);
//		spi_test(1000);
//		simple_ramping_example();
		//continuous_inventory(0);
		gen2x_continuous_inventory(0);
		k_msleep(1000);
		printk("finish\n\n");
	//}
}

K_THREAD_STACK_DEFINE(rain_app_stack_area, CONFIG_MAIN_STACK_SIZE);
struct k_thread rain_app_thread_data;

int main(void)
{
	int rc;

	// Optional: System starts at 64MHz. Set sys clock to 128MHz by setting divider of 1x.
	nrfx_clock_divider_set(    NRF_CLOCK_DOMAIN_HFCLK, NRF_CLOCK_HFCLK_DIV_1);
	printk("Starting %s with CPU frequency: %d MHz\n", CONFIG_BOARD, SystemCoreClock/MHZ(1));
	k_msleep(1000);
	// Testing
	k_tid_t my_tid = k_thread_create(&rain_app_thread_data, rain_app_stack_area,
                                 K_THREAD_STACK_SIZEOF(rain_app_stack_area),
                                 rain_application_thread,
                                 NULL, NULL, NULL,
                                 5, 0, K_NO_WAIT);

	k_msleep(1000);
	
	while (true) {
		k_msleep(30000);

#ifdef CONFIG_THREAD_ANALYZER		
		//thread_analyzer_print();
#endif
	}

	return 0;
}
