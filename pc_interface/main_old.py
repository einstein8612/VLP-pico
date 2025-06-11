import serial
import struct

import numpy as np
import pandas as pd
from tqdm import tqdm

import time

def pack_input(eval_flag, inputs):
    return struct.pack('<B36f', eval_flag, *inputs)

def unpack_output(response_bytes):
    return struct.unpack('<2f', response_bytes)

def main():
    df = pd.read_csv("test.csv")
    ys = df[["x", "y"]]
    xs = df[[f"led_{i}" for i in range(36)]]

    with serial.Serial("/dev/ttyACM0", 9600, timeout=100) as ser:
        now = time.time()
        total_error = 0
        for i in tqdm(range(len(xs))):
            x_normalized = xs.iloc[i] / np.linalg.norm(xs.iloc[i])
            packet = pack_input(1, x_normalized.tolist())
            ser.write(packet)

            bytes = ser.read_until(expected=b"BIGGER_THAN_8", size=8)
            x,y = unpack_output(bytes)
            error = np.linalg.norm(ys.iloc[i] - [x, y])
            total_error += error
        error = total_error / len(xs)
        print(f"Average error: {error}")

        elapsed = time.time() - now
        print(elapsed)

if __name__ == "__main__":
    main()