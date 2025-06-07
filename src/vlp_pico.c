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

    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    while (true) {
        sleep_ms(10); // Sleep to avoid busy-waiting
    
        // Check if USB is connected before reading input
        if (!stdio_usb_connected()) {
            continue;
        }

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

        write_packet(x, y);
    }

    return 0;
}
