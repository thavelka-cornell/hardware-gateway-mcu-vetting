//======================================================================
//
//  Project:  RP2040 Exploration
//
//======================================================================



//----------------------------------------------------------------------
// - SECTION - includes
//----------------------------------------------------------------------

//#include <string.h>
#include <stdint.h>                // to provide define of uint32_t

// Zephyr RTOS related:
#include <zephyr.h>

// Zephyr device and target processor peripheral API headers:
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#include "thread-led.h"
#include "app-return-values.h"




//----------------------------------------------------------------------
// - SECTION - stuff mostly to refactor to better file locations:
//----------------------------------------------------------------------

#define THREAD_LED__PRIORITY (8)
#define MODULE_ID__THREAD_LED "thread-led"
#define SLEEP_TIME__THREAD_LED__MS (3000)


#define LED0_NODE DT_ALIAS(led0)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#warning --- --- --- macro evaluating 'led0' dts alias returns true --- --- ---
#define LED0_LABEL   DT_GPIO_LABEL(LED0_NODE, gpios)
#define LED0_PIN     DT_GPIO_PIN(LED0_NODE, gpios)
#define LED0_FLAGS   DT_GPIO_FLAGS(LED0_NODE, gpios)
#else
#warning --- --- --- macro evaluating 'led0' dts alias returns false --- --- ---
#define LED0_LABEL   ""
#define LED0_PIN     0
#define LED0_FLAGS   0
#endif

#define SLEEP_TIME_MS (5000)





static uint32_t sleep_period__thread_led__fsv;

static const struct device *dev;




//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

#define THREAD_LED__STACK_SIZE 512

K_THREAD_STACK_DEFINE(thread_led__stack_area, THREAD_LED__STACK_SIZE);
struct k_thread thread_led__thread_data;

int thread_led__initialize(void)
{
    uint32_t rstatus = 0;

    k_tid_t thread_led__tid = k_thread_create(&thread_led__thread_data, thread_led__stack_area,
                                            K_THREAD_STACK_SIZEOF(thread_led__stack_area),
                                            thread_led__entry_point,
                                            NULL, NULL, NULL,
                                            THREAD_LED__PRIORITY,
                                            0,
                                            K_MSEC(1000)); // K_NO_WAIT);

    rstatus = k_thread_name_set(thread_led__tid, MODULE_ID__THREAD_LED);
    if ( rstatus == 0 ) { } // avoid compiler warning about unused variable - TMH

    sleep_period__thread_led__fsv = SLEEP_TIME__THREAD_LED__MS;

    return (int)thread_led__tid;
}



void thread_led__entry_point(void* arg1, void* arg2, void* arg3)
{
//    const struct device *dev;
    bool led_is_on = true;
    int ret;

    int main_loop_count = 0;
    uint32_t rstatus = ROUTINE_OK;


    dev = device_get_binding(LED0_LABEL);
    if (dev == NULL) {
        return;
    }

    ret = gpio_pin_configure(dev, LED0_PIN, GPIO_OUTPUT_ACTIVE | LED0_FLAGS);
    if (ret < 0) {
        return;
    }

    while (1)
    {

#if 1
        gpio_pin_set(dev, LED0_PIN, (int)led_is_on);
        led_is_on = !led_is_on;
        k_msleep(SLEEP_TIME_MS);

        printk("- MARK - from LED thread 0808\n\r");
#else
        switch(present_led_task)
        {
// Note - periodic blink case is the original Nordic Semi 'blinky'
//  sample app code, which lights LED at fifty percent duty cycle:
            case PP_THREAD_LED__PERIODIC_BLINK:
		gpio_pin_set(dev, PIN, (int)led_is_on);
		led_is_on = !led_is_on;
		k_msleep(SLEEP_TIME_MS);
                break;

            case PP_THREAD_LED__BLINK_ON_ALERT:
                rstatus = run_user_defined_finite_blink_pattern(PP_THREAD_LED__BLINK_PATTERN_CONTINUE);
                break;

            case PP_THREAD_LED__OFF:
		gpio_pin_set(dev, PIN, false);
                break;

            default:
                break;
        }

// The second call here to k_msleep() provides a minimal yet not
// zero sleep time in case / until we determine what impact is of
// blink patterns with zero or small period defined in them:


// *** DEBUG 0314 BEGIN *** QUESTION where is ten millisecond current spike originating? - TMH

//        if ( present_alert == PP_ALERT__NONE )
        {
            k_msleep(sleep_period__thread_led__fsv);
        }
//        else
//        {
//            k_msleep(SLEEP_PERIOD_OF_TEN_MS);
//        }

// *** DEBUG 0314 END ***


#endif

        main_loop_count++;
    }
}



//----------------------------------------------------------------------
// - SECTION - public API
//----------------------------------------------------------------------

