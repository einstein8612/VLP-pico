#ifndef DEBUG_H
#define DEBUG_H

#include "led/led.h"

#ifdef DEBUG_LED
#define DEBUG_LED_INIT() led_init()
#define DEBUG_LED_BLINK(times, interval_ms) led_blink(times, interval_ms)
#define DEBUG_LED_SET(value) led_set(value)
#else
#define DEBUG_LED_INIT() ((void)0)
#define DEBUG_LED_BLINK(times, interval_ms) ((void)0)
#define DEBUG_LED_SET(value) ((void)0)
#endif

#endif
