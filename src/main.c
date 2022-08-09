/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */



//----------------------------------------------------------------------
// - SECTION - includes, NEED to review line by line which are needed - TMH
//----------------------------------------------------------------------

#include <errno.h>
#include <string.h>
#define LOG_LEVEL 4
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);


#include <zephyr/zephyr.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>

#include <zephyr/drivers/led_strip.h>   // to provide ...

// 2022-08-04 added for simple factoring during early UART tests:
#include "main.h"
#include "development-defines.h"
#include "thread-led.h"

#include "pulse-v1-axis-sel.h"     // to provide AXIS_X and related
#include "readings-set.h"
#include "data-model-stage1.h"



//----------------------------------------------------------------------
// - SECTION - defines
//----------------------------------------------------------------------

// 1000 msec = 1 sec
#define SLEEP_TIME_MS   750 // 1000

// The devicetree node identifier for the "led0" alias.
#define LED0_NODE DT_ALIAS(led0)


#define MARK_CYCLE_LENGTH 6


/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */

#ifdef DEV_0805_KEEP_BLINKY_CODE_ENABLED
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
#endif


#ifdef DEV_0805__WS2812_BRING_UP_WORK_ON_RP2040

// --- DEV BEGIN :: WS2812 test ---
//
// Source file:  ~/projects-sandbox/workspace-for-rpi/zephyr/samples/drivers/led_ws2812/src/main.c

#define STRIP_NODE              DT_ALIAS(led_strip)
#define STRIP_NUM_PIXELS        DT_PROP(DT_ALIAS(led_strip), chain_length)

#define DELAY_TIME K_MSEC(50)

#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }

static const struct led_rgb colors[] = { 
        RGB(0x0f, 0x00, 0x00), /* red */
        RGB(0x00, 0x0f, 0x00), /* green */
        RGB(0x00, 0x00, 0x0f), /* blue */
};

struct led_rgb pixels[STRIP_NUM_PIXELS];

static const struct device *strip = DEVICE_DT_GET(STRIP_NODE);

// --- DEV END :: WS2812 test ---
#endif // DEV_0805__WS2812_BRING_UP_WORK_ON_RP2040



//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

void main(void)
{
    int ret;

    static uint32_t routine_main_iterations = 0;

#if 1 //
    uint32_t count_for_mark_messages = 0;


    char lbuf[SIZE_OF_TEN_BYTES] = { 0 };

    if (!device_is_ready(led.port)) {
        return;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return;
    }


    memset(lbuf, 0, SIZE_OF_TEN_BYTES);

// NEED to capture return value from this routine call:
    thread_led__initialize();

// To support vRMS code tests:
    populate_acc_buf(AXIS_X);


    while (1)
    {
#ifdef DEV_0808__BLINK_FROM_MAIN_NOT_FROM_THREAD
        ret = gpio_pin_toggle_dt(&led);

        if (ret < 0) {
            return;
        }
#endif


// --- DEV BEGIN :: UART stuff ---
        if ( count_for_mark_messages < MARK_CYCLE_LENGTH )
        {
            count_for_mark_messages++;
        }
        else
        {
            count_for_mark_messages = 1;
            memset(lbuf, 0, SIZE_OF_TEN_BYTES);
        }
         
        memset(lbuf, '.', count_for_mark_messages);

        printk("- MARK - ( rpi work 2022-08-08 )");
        printk("%s", lbuf);
        printk("\n\r");


        if ( ( routine_main_iterations % 5 ) == 0 )
        {
            populate_acc_buf(AXIS_X);

            printk("...0809 vRMS code testing underway...\n\r");
            on_event__readings_done__calculate_vrms(AXIS_X);
        }

        routine_main_iterations++;

// --- DEV END :: UART stuff ---

        k_msleep(SLEEP_TIME_MS);
    }
#endif

#ifdef DEV_0805__WS2812_BRING_UP_WORK_ON_RP2040
    size_t cursor = 0, color = 0;
    int rc;

    if (device_is_ready(strip)) {
        LOG_INF("Found LED strip device %s", strip->name);
    } else {
        LOG_ERR("LED strip device %s is not ready", strip->name);
        return;
    }

    LOG_INF("Displaying pattern on strip");
    while (1) {
        memset(&pixels, 0x00, sizeof(pixels));
        memcpy(&pixels[cursor], &colors[color], sizeof(struct led_rgb));
        rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);

        if (rc) {
            LOG_ERR("couldn't update strip: %d", rc);
        }

        cursor++;
        if (cursor >= STRIP_NUM_PIXELS) {
            cursor = 0;
            color++;
            if (color == ARRAY_SIZE(colors)) {
                color = 0;
            }
        }

        k_msleep(SLEEP_TIME_MS);
    }
#endif // DEV_0805__WS2812_BRING_UP_WORK_ON_RP2040

}



// --- EOF ---
