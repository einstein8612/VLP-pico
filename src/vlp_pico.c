#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    printf("Ready for input\n");

    const int MAX_LINE_LENGTH = 256;
    char buffer[MAX_LINE_LENGTH];
    int index = 0;

    while (true) {
        sleep_ms(1);
    
        // Check if USB is connected before reading input
        if (!stdio_usb_connected()) {
            printf("Waiting for USB...\n");
            continue;
        }

        int c = getchar_timeout_us(0);  // Non-blocking read
        if (c == PICO_ERROR_TIMEOUT) {
            continue;
        }

        printf("Got: %c (%d)\n", c, c);
    }

    return 0;
}
