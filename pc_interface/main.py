import serial
import struct

import time

input_example = [0.21015011,0.10497452,0.029676337,0.011930144,0.0,0.0,0.6672459238000796,0.20419017627480446,0.053542458,0.016145056,0.0,0.0,0.45816955,0.1737651,0.039164636,0.014616314,0.0,0.0,0.112191476,0.06503443,0.025773967,0.0,0.0,0.0,0.027632952,0.019319404,0.013114345,0.0,0.0,0.0,0.0122044245,0.0,0.0,0.0,0.0,0.0]
eval_flag = 1


def pack_input(eval_flag, inputs):
    return struct.pack('<B36f', eval_flag, *inputs)

def unpack_output(response_bytes):
    return struct.unpack('<2f', response_bytes)

def main():
    packet = pack_input(eval_flag, input_example)

    with serial.Serial("/dev/ttyACM0", 9600, timeout=100) as ser:
        now = time.time()
        for _ in range(1000):
            ser.write(packet)

            bytes = ser.read_until(size=8)
            x,y = unpack_output(bytes)
            print(f"Received: x={x}, y={y}")

        elapsed = time.time() - now
        print(elapsed)

if __name__ == "__main__":
    main()