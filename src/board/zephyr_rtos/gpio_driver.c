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

#define _GNU_SOURCE

#include "../gpio_driver.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include "ex10_api/ex10_print.h"


#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

const struct gpio_dt_spec GPI_IRQ_N = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, e910_irqn_gpios);
const struct gpio_dt_spec GPO_PWR_EN = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, e910_pwren_gpios);
const struct gpio_dt_spec GPO_RESET_N = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, e910_resetn_gpios);
const struct gpio_dt_spec GPO_ENABLE = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, e910_enable_gpios);
const struct gpio_dt_spec GPIO_READY_INT_N = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, e910_readyn_gpios);


// Protects access to callback.
K_MUTEX_DEFINE(irq_n_callback_lock);
extern struct k_mutex irq_n_callback_lock;

// IRQ callback function.
static void (*irq_n_cb)(void) = NULL;

// We only init the GPIO IRQ ints one time..
static volatile bool is_irq_gpio_initialized = false;

// IRQ enabled flag.
static volatile bool is_irq_enabled = false;

// When set to false, inhibit the IRQ_N monitor thread from calling the
// registered callback function pointer irq_n_cb.
// This value is set to true when the register_irq_callback() function
// successfully starts the IRQ_N monitor thread.
// This value is set to false when deregister_irq_callback() is called.
// The interface functions irq_monitor_callback_enable() and
// irq_monitor_callback_is_enabled() provides external access to this value.
static volatile bool irq_monitor_callback_enable_flag = false;

// For counting unhandled Yukon IRQs.
static volatile int unhandled_irq_count = 0;

// Semaphore for signaling interrupt handling thread when irq GPIO goes low. 
K_SEM_DEFINE(yukon_irq_sem, 0, 1);

static void irq_gpio_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    if (irq_monitor_callback_enable_flag && irq_n_cb) {
        unhandled_irq_count++;
        k_sem_give(&yukon_irq_sem);
    }
}

// Monitors IRQ line status and calls callbacks when enabled.
static void irq_handler_thread(void *arg1, void *arg2, void *arg3) {
    while (1) {
        k_sem_take(&yukon_irq_sem, K_FOREVER);

        // Skip callback if IRQ not active or enabled.
        //if (gpio_pin_get_dt(&GPI_IRQ_N) != 0) continue; // This is the preferred approach but it seems that (at this time) setting the GPIO up for interrupts disables it as an input so it always returns 0.
        if (!unhandled_irq_count) continue; // so we use a volatile variable to count IRQs instead of the line above....
        unhandled_irq_count = 0;
        
        // IRQ is active, run callback if present and enabled.
        k_mutex_lock(&irq_n_callback_lock, K_FOREVER);
        if (irq_n_cb && irq_monitor_callback_enable_flag) {
            irq_n_cb();
        }
        k_mutex_unlock(&irq_n_callback_lock);
    }
}

// GPIO IRQ handler thread, runs forever.
K_THREAD_DEFINE(irq_thread_id, CONFIG_MAIN_STACK_SIZE, irq_handler_thread, NULL, NULL, NULL, -5, K_ESSENTIAL, 0);
extern const k_tid_t irq_thread_id; 

static void gpio_cleanup(void)
{
//    gpio_pin_configure_dt(&GPI_IRQ_N, GPIO_INPUT);
    gpio_pin_configure_dt(&GPO_PWR_EN, GPIO_INPUT);
    gpio_pin_configure_dt(&GPO_RESET_N, GPIO_INPUT);
    gpio_pin_configure_dt(&GPO_ENABLE, GPIO_INPUT);
    gpio_pin_configure_dt(&GPIO_READY_INT_N, GPIO_INPUT);
}

/// @todo PI-29685 Ex10GpioDriver.initialize(), consider removing parameters
/// There is only one proper set of values that can be used here:
/// (PWR_EN: deassert false, ENABLE: deassert false, RESET_N: deassert true)
static void gpio_initialize(bool board_power_on, bool ex10_enable, bool reset)
{
    gpio_pin_configure_dt(&GPO_PWR_EN, GPIO_INPUT | GPIO_OUTPUT_INACTIVE | GPIO_ACTIVE_HIGH);
    gpio_pin_configure_dt(&GPO_RESET_N, GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_LOW);
    gpio_pin_configure_dt(&GPO_ENABLE, GPIO_INPUT | GPIO_OUTPUT_INACTIVE | GPIO_ACTIVE_HIGH);
//    gpio_pin_configure_dt(&GPIO_READY_INT_N, GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_LOW);
    gpio_pin_configure_dt(&GPIO_READY_INT_N, GPIO_INPUT);

    if (!is_irq_gpio_initialized) {
        is_irq_gpio_initialized = true;

        // Set up GPIO IRQ callback.
        gpio_pin_configure_dt(&GPI_IRQ_N, GPIO_INPUT);

        static struct gpio_callback gpio_cb;
        gpio_init_callback(&gpio_cb, irq_gpio_callback, BIT(GPI_IRQ_N.pin));
        gpio_add_callback_dt(&GPI_IRQ_N, &gpio_cb);
        gpio_pin_interrupt_configure_dt(&GPI_IRQ_N, GPIO_INT_EDGE_FALLING);
    }
}

static void irq_enable_func(bool enable)
{
    is_irq_enabled = enable;
    if (enable) {
        k_mutex_unlock(&irq_n_callback_lock);
    } else {
        k_mutex_lock(&irq_n_callback_lock, K_FOREVER);
    }
}

static void set_board_power(bool power_on)
{
    gpio_pin_set_dt(&GPO_PWR_EN, power_on);
}

static bool get_board_power(void)
{
    const bool is_powered = gpio_pin_get_dt(&GPO_PWR_EN);
    return is_powered;
}

static void set_ex10_enable(bool enable)
{
    gpio_pin_set_dt(&GPO_ENABLE, enable);
}

static bool get_ex10_enable(void)
{
    const bool is_enabled = gpio_pin_get_dt(&GPO_ENABLE);
    return is_enabled;
}

static int register_irq_callback(void (*cb_func)(void))
{

    int error_code = 0;
    k_mutex_lock(&irq_n_callback_lock, K_FOREVER);
    if (irq_n_cb == NULL)
    {
        irq_n_cb = cb_func;
        irq_monitor_callback_enable_flag = true;
        unhandled_irq_count = 0;
        //k_sem_give(&yukon_irq_sem);
    }
    else
    {
        ex10_eprintf("error: %s: already registered\n", __func__);
        error_code = EBUSY;
    }

    k_mutex_unlock(&irq_n_callback_lock);
    return error_code;
}

static int deregister_irq_callback(void)
{
    int error_code = 0;
    // Note: This function is called multiple times during board teardown;
    // i.e. double deregistration. gpio_cleanup() will call this function
    // regardless of state to insure gpio driver resource release.
    // Don't return an error due to irq_n_cb == NULL.
    k_mutex_lock(&irq_n_callback_lock, K_FOREVER);
    if (irq_n_cb != NULL)
    {
        irq_monitor_callback_enable_flag = false;
        irq_n_cb                         = NULL;
        unhandled_irq_count = 0;
    }
    k_mutex_unlock(&irq_n_callback_lock);
    return error_code;
}

static void irq_monitor_callback_enable(bool enable)
{
    k_mutex_lock(&irq_n_callback_lock, K_FOREVER);
    unhandled_irq_count = 0;
    irq_monitor_callback_enable_flag = enable;
    k_mutex_unlock(&irq_n_callback_lock);
    if (enable) {
        //k_sem_give(&yukon_irq_sem);
    }
}

static bool irq_monitor_callback_is_enabled(void)
{
    k_mutex_lock(&irq_n_callback_lock, K_FOREVER);
    bool const enable = irq_monitor_callback_enable_flag;
    k_mutex_unlock(&irq_n_callback_lock);
    return enable;
}

static bool thread_is_irq_monitor(void)
{
    k_tid_t tid = k_current_get();
    return tid == irq_thread_id;
}

static void gpio_release_all_lines(void)
{
    gpio_cleanup();
}

static void assert_ready_n(void)
{
    gpio_pin_configure_dt(&GPIO_READY_INT_N, GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_LOW);
    gpio_pin_set_dt(&GPIO_READY_INT_N, 1);
}

static void release_ready_n(void)
{
    gpio_pin_configure_dt(&GPIO_READY_INT_N, GPIO_INPUT);
}

static void assert_reset_n(void)
{
    gpio_pin_set_dt(&GPO_RESET_N, 1);
}

static void deassert_reset_n(void)
{
    gpio_pin_set_dt(&GPO_RESET_N, 0);
}

static void reset_device(void)
{
    assert_reset_n();
    k_msleep(10);
    deassert_reset_n();
}

static int ready_n_pin_get(void)
{
    int pinval = gpio_pin_get_dt(&GPIO_READY_INT_N);
    return pinval != 0;
}

static int busy_wait_ready_n(uint32_t timeout_ms)
{
    uint64_t start_ms = k_uptime_get();
    bool timed_out = false;
    while (!ready_n_pin_get())
    {
        if ((k_uptime_get() - start_ms) >= timeout_ms) {
            printk("Wait for ready_n timed out!\n");
            return -1;
        }
//        k_usleep(1);
    }

    return 0;
}

static bool get_test_pin_level(uint8_t pin_no)
{
    return false;  // not a valid pin_no
}

static size_t debug_pin_get_count(void)
{
    return 0;
}

static bool debug_pin_get(uint8_t pin_idx)
{
    return false;
}

static void debug_pin_set(uint8_t pin_idx, bool value)
{
}

static void debug_pin_toggle(uint8_t pin_idx)
{
}

static size_t led_pin_get_count(void)
{
    return 0;
}

static bool led_pin_get(uint8_t pin_idx)
{
    switch(pin_idx) {
        case 0:
        case 1:

    }
    return true;
}

static void led_pin_set(uint8_t pin_idx, bool value)
{
}

static void led_pin_toggle(uint8_t pin_idx)
{
    led_pin_set(pin_idx, !led_pin_get(pin_idx));
}

static struct Ex10GpioDriver const ex10_gpio_driver = {
    .gpio_initialize                 = gpio_initialize,
    .gpio_cleanup                    = gpio_cleanup,
    .set_board_power                 = set_board_power,
    .get_board_power                 = get_board_power,
    .set_ex10_enable                 = set_ex10_enable,
    .get_ex10_enable                 = get_ex10_enable,
    .register_irq_callback           = register_irq_callback,
    .deregister_irq_callback         = deregister_irq_callback,
    .irq_monitor_callback_enable     = irq_monitor_callback_enable,
    .irq_monitor_callback_is_enabled = irq_monitor_callback_is_enabled,
    .irq_enable                      = irq_enable_func,
    .thread_is_irq_monitor           = thread_is_irq_monitor,
    .assert_reset_n                  = assert_reset_n,
    .deassert_reset_n                = deassert_reset_n,
    .release_ready_n                 = release_ready_n,
    .assert_ready_n                  = assert_ready_n,
    .reset_device                    = reset_device,
    .busy_wait_ready_n               = busy_wait_ready_n,
    .ready_n_pin_get                 = ready_n_pin_get,
    .get_test_pin_level              = get_test_pin_level,
    .debug_pin_get_count             = debug_pin_get_count,
    .debug_pin_get                   = debug_pin_get,
    .debug_pin_set                   = debug_pin_set,
    .debug_pin_toggle                = debug_pin_toggle,
    .led_pin_get_count               = led_pin_get_count,
    .led_pin_get                     = led_pin_get,
    .led_pin_set                     = led_pin_set,
    .led_pin_toggle                  = led_pin_toggle,
};

struct Ex10GpioDriver const* get_ex10_gpio_driver(void)
{
    return &ex10_gpio_driver;
}
