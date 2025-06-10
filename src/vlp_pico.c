#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#include "model/model.h"
#include "io/io.h"
#include "debug.h"

#include "data/data.h"

#include "degradation_model/degradation_model.h"

int main()
{
    io_init();
    DEBUG_LED_INIT();

    if (load_model() != kTfLiteOk)
    {
        return 1;
    }

    while (!stdio_usb_connected())
    {
        sleep_ms(100); // Wait for USB connection
    }

    // Indicate that the program is running
    DEBUG_LED_SET(true);

    while (true)
    {
        sleep_ms(1); // Sleep to avoid busy-waiting

        // Read a packet with a timeout
        IncomingPacket packet;
        int result = read_packet(100, &packet);
        if (result == PICO_ERROR_TIMEOUT)
        {
            continue;
        }

        float x, y;
        if (predict(packet.leds, &x, &y) != kTfLiteOk)
        {
            continue;
        }

        DEBUG_LED_BLINK(5, 100);

        if (packet.eval)
        {
            write_packet(x, y);
            continue;
        }

        // If not evaluating, do led degradation logic
    }

    return 0;
}
