#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#include "model/model.h"
#include "io/io.h"

int main() {
    io_init();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    if (load_model() != kTfLiteOk) {
        return 1;
    }

    while (!stdio_usb_connected()) {
        sleep_ms(100); // Wait for USB connection
    }

    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    while (true) {
        sleep_ms(1); // Sleep to avoid busy-waiting
    
        // Read a packet with a timeout
        IncomingPacket packet;
        int result = read_packet(100, &packet);
        if (result == PICO_ERROR_TIMEOUT) {
            continue;
        }

        float x, y;
        if (predict(packet.leds, &x, &y) != kTfLiteOk) {
            printf("Failed to run inference\n");
            continue;
        }

        if (packet.eval) {
            write_packet(x, y);
            continue;
        }

        // If not evaluating, do led degradation logic
    }

    return 0;
}
