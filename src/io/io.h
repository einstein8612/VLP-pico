#include <stdbool.h>
#include <stdint.h>

#ifndef IO_H
#define IO_H

typedef struct IncomingPacket
{
    bool eval;
    float leds[36];
} IncomingPacket;

void io_init(void);

int read_packet(uint32_t timeout_us, IncomingPacket *packet);

void write_packet(float x, float y);

#endif // IO_H