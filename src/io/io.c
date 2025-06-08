#include "io.h"

#include "pico/stdio.h"

#include <stdio.h>

typedef union
{
    float f;
    uint8_t b[4];
} float_bytes_t;

static void stdio_write_float_le(float value)
{
    float_bytes_t u;
    u.f = value;

    for (int i = 0; i < 4; ++i)
    {
        stdio_putchar_raw(u.b[i]);
    }
}

static int stdio_get_float_le_timeout_us(uint32_t timeout_us, float *value)
{
    float_bytes_t u;
    for (int i = 0; i < 4; ++i)
    {
        int c = stdio_getchar_timeout_us(timeout_us);
        if (c == PICO_ERROR_TIMEOUT)
        {
            return PICO_ERROR_TIMEOUT;
        }
        u.b[i] = (uint8_t)c;
    }
    *value = u.f;
    return PICO_OK;
}

void io_init(void)
{
    // Initialize the IO system
    stdio_init_all();
}

int read_packet(uint32_t timeout_ms, IncomingPacket *packet)
{
    // Read a packet from the IO system
    int eval_char = stdio_getchar_timeout_us(timeout_ms * 1000);
    if (eval_char == PICO_ERROR_TIMEOUT)
    {
        return PICO_ERROR_TIMEOUT;
    }
    bool eval = (eval_char == 1);

    IncomingPacket read_packet;
    read_packet.eval = eval;
    for (int i = 0; i < 36; i++)
    {
        float value;
        int result = stdio_get_float_le_timeout_us(timeout_ms * 1000, &value);
        if (result == PICO_ERROR_TIMEOUT)
        {
            return PICO_ERROR_TIMEOUT;
        }
        read_packet.leds[i] = value;
    }

    *packet = read_packet;
    return PICO_OK;
}

void write_packet(float x, float y)
{
    // Write a packet to the IO system
    stdio_write_float_le(x);
    stdio_write_float_le(y);
    stdio_flush();
}
