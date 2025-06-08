#ifndef LED_H
#define LED_H

#include <stdbool.h>
#include <stdint.h>

void led_init(void);
void led_set(bool value);
void led_blink(uint32_t times, uint32_t interval_ms);

#endif
