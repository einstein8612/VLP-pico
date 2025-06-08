import serial
import struct

import numpy as np

import time

def pack_input(eval_flag, inputs):
    return struct.pack('<B36f', eval_flag, *inputs)

def unpack_output(response_bytes):
    return struct.unpack('<2f', response_bytes)

def main():
    with serial.Serial("/dev/ttyACM0", 9600, timeout=100) as ser:
        now = time.time()
        for _ in range(1000):
            packet = pack_input(1, np.random.normal(size=36).astype(np.float32).tolist())
            ser.write(packet)

            bytes = ser.read_until(expected=b"BIGGER_THAN_8", size=8)
            x,y = unpack_output(bytes)
            print(f"Received: x={x}, y={y}")

        elapsed = time.time() - now
        print(elapsed)

if __name__ == "__main__":
    main()