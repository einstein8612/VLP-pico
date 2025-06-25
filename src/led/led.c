#include "led.h"

#include "pico/stdlib.h"

#ifdef PICO_DEFAULT_LED_PIN

#define LED_PIN PICO_DEFAULT_LED_PIN

static bool last_led_state = false;

void led_init(void)
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);
}

void led_set(bool value)
{
    gpio_put(LED_PIN, value);
    last_led_state = value;
}

void led_blink(uint32_t times, uint32_t interval_ms)
{
    interval_ms = interval_ms / 2;
    for (uint32_t i = 0; i < times; i++)
    {
        gpio_put(LED_PIN, true);
        sleep_ms(interval_ms);
        gpio_put(LED_PIN, false);
        sleep_ms(interval_ms);
    }
    gpio_put(LED_PIN, last_led_state); // Restore last state
}

#endif
