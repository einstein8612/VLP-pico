import argparse
import json
import os
import struct
from time import time

import matplotlib.pyplot as plt
import numpy as np
import numpy.typing as npt
import serial
from tqdm import tqdm

def calculate_decay_constant(r90_hrs: float):
    decay_k = np.log(1/0.9) / r90_hrs
    return decay_k

R90_MIN = 10000
R90_MAX = 50000

def pack_input(eval_flag, inputs):
    return struct.pack('<B36f', eval_flag, *inputs)

def unpack_output(response_bytes):
    return struct.unpack('<2f', response_bytes)

def generate_test_set(data: npt.NDArray, valid_mask: npt.NDArray, test_set_fraction: float, rng: np.random.Generator) -> tuple[npt.NDArray, npt.NDArray]:
    H, W, leds_n = data.shape

    # Flatten data
    flat_data = data.reshape(W*H, leds_n)

    # Generate LED ids
    led_ids = np.arange(leds_n)[None, :]  # Shape (1, 1, leds_n)
    # Generate random sample indices at each time step
    valid_flat_idxs = np.flatnonzero(valid_mask)

    sample_flat_idxs = rng.choice(valid_flat_idxs, int(H*W*test_set_fraction))

    # Calculate the x and y coordinates of the samples
    ys = sample_flat_idxs // W
    xs = sample_flat_idxs % W
    sample_locs = np.stack((xs, ys), axis=-1)

    # Fetch the samples for each LED at each time step
    # Add a new axis to the sample_flat_idxs to match the shape of led_ids
    sample_flat_idxs = sample_flat_idxs[:, None]

    # Get samples
    # Get the samples for each LED at each timestep with the same sample index. Shape (H*W*fraction, leds_n)
    samples = flat_data[sample_flat_idxs, led_ids]

    return samples, sample_locs

def generate_aged_samples(data: npt.NDArray, valid_mask: npt.NDArray, relative_decay: npt.NDArray, samples_per_timestep: int, rng: np.random.Generator) -> tuple[npt.NDArray, npt.NDArray]:
    timesteps_n, leds_n = relative_decay.shape
    H, W, leds_n = data.shape

    # Flatten data
    flat_data = data.reshape(W*H, leds_n)

    # Generate LED ids
    led_ids = np.arange(leds_n)[None, None, :]  # Shape (1, 1, leds_n)
    # Generate random sample indices at each time step
    valid_flat_idxs = np.flatnonzero(valid_mask)

    sample_flat_idxs = rng.choice(
        valid_flat_idxs, size=(timesteps_n, samples_per_timestep))

    # Calculate the x and y coordinates of the samples
    ys = sample_flat_idxs // W
    xs = sample_flat_idxs % W
    sample_locs = np.stack((xs, ys), axis=-1)

    # Fetch the samples for each LED at each time step
    # Add a new axis to the sample_flat_idxs to match the shape of led_ids
    sample_flat_idxs = sample_flat_idxs[:, :, None]
    sample_flat_idxs = np.broadcast_to(
        sample_flat_idxs, (timesteps_n, samples_per_timestep, leds_n))
    led_ids = np.broadcast_to(
        led_ids, (timesteps_n, samples_per_timestep, leds_n))

    # Age the samples
    # Get the samples for each LED at each timestep with the same sample index. Shape (timesteps, samples_per_timestep, leds_n)
    samples = flat_data[sample_flat_idxs, led_ids]
    # Apply the relative decay to the samples
    aged_samples = samples * relative_decay[:, None, :]

    # Add flickering
    # flickering = rng.choice([0, 1], size=aged_samples.shape, p=[args.flickering_prob, 1 - args.flickering_prob])
    # aged_samples = aged_samples * flickering # Apply flickering to the samples

    return aged_samples, sample_locs

def predict(serial_conn: serial.Serial, inputs: npt.NDArray, eval_flag: int = 1) -> npt.NDArray:
    output = np.zeros((inputs.shape[0], 2), dtype=np.float32)
    for i in range(inputs.shape[0]):
        input_i = inputs[i] / np.linalg.norm(inputs[i])  # Normalize the input

        # Pack the input data
        packed_input = pack_input(eval_flag, input_i.tolist())
        serial_conn.write(packed_input)

        if eval_flag == 1:
            # Read the response from the Pico device
            response_bytes = serial_conn.read_until("LONGER_THAN_8", 8)

            # Unpack the output data
            x, y = unpack_output(response_bytes)
            output[i] = np.array([x, y])

    if eval_flag == 0:
        response_bytes = serial_conn.read_until("LONGER_THAN_8", 8)
        x, y = unpack_output(response_bytes)
        print(f"Model warmup response: x={x}, y={y}")

    return output

def main(args):
    rng = np.random.default_rng(args.seed)

    data = np.load(args.src)
    H, W, leds_n = data.shape
    print(f"Data loaded from {args.src}")

    # Generate valid mask for data
    # Shape (H, W), 1 for valid, 0 for invalid same for all LEDs
    valid_mask = np.ones_like(data[:, :, 0], dtype=float)
    valid_mask[data[:, :, 0] == -1] = 0
    print(f"Generated valid mask ({valid_mask.sum()} valid points)")

    # Generate time steps
    timesteps = np.arange(0, args.time, args.timestep)
    print(f"Generated {len(timesteps)} timesteps")

    r90_hours = rng.integers(R90_MIN, R90_MAX, leds_n)
    decay_ks = calculate_decay_constant(r90_hours)
    # Calculate relative decay for each LED at each time step
    relative_decay = np.exp(-np.outer(timesteps, decay_ks))
    # Add noise to the data
    relative_decay += rng.normal(0, args.std, size=relative_decay.shape)
    print(f"Generated {leds_n} decay constants and their decay scalers")

    aged_samples_X, aged_samples_y = generate_aged_samples(
        data, valid_mask, relative_decay, args.samples_per_timestep, rng)
    aged_samples_y *= 10  # Scale the samples to mm
    print(
        f"Generated {aged_samples_X.shape[0]*aged_samples_X.shape[1]} samples; {aged_samples_X.shape[1]} samples at {aged_samples_X.shape[0]} timesteps")

    test_X, test_y = generate_test_set(data, valid_mask, 0.02, rng)
    test_X, test_y = test_X[:1000], test_y[:1000]
    test_y *= 10  # Scale the test set to mm

    print(f"Connecting to model Pico at {args.pico_serial_port}")
    pico_serial_conn = serial.Serial(args.pico_serial_port, 9600, timeout=100)

    errors = []
    bar = tqdm(enumerate(timesteps), total=len(timesteps))
    for i, t in bar:
        # Warm up the model with samples at this timestep
        aged_samples_X_t, aged_samples_y_t = aged_samples_X[i,
                                                            :, :], aged_samples_y[i, :, :]
        predict(pico_serial_conn, aged_samples_X_t, 0)

        # Test accuracy at this timestep
        decay_scalars = relative_decay[i, :]
        predictions = predict(pico_serial_conn, test_X * decay_scalars, 1)
        average_error = np.linalg.norm(predictions - test_y, axis=1).mean()
        bar.set_description(
            f"Average error at {t} hours: {average_error:.2f} mm")
        errors.append(average_error)
    
    if not args.save:
        return
    
    avg_decay = relative_decay.mean(axis=1)
    min_decay = relative_decay.min(axis=1)
    max_decay = relative_decay.max(axis=1)

    now = int(time())

    os.makedirs(f"./saved_timeseries_run/{now}", exist_ok=True)

    # Save the results
    _, axs = plt.subplots(3, figsize=(10, 15))

    # Plot the errors and cumulative errors
    axs[0].plot(timesteps, errors, label=f"PICO-REAL (Current Run)")
    axs[0].set(xlabel='Time in hours passed', ylabel='Positioning Error (mm)')
    axs[0].set(title='Positioning Error vs. LED Degradation')

    axs[1].plot(timesteps, np.cumsum(errors), label=f"PICO_REAL (Current Run)")
    axs[1].set(xlabel='Time in hours passed', ylabel='Cumulative Positioning Error (mm)')
    axs[1].set(title='Cumulative Positioning Error vs. LED Degradation')

    if args.compare_to:
        # Compare to past runs
        for run in args.compare_to:
            run_results = json.load(open(f"{run}/results.json", "r"))
            run_errors = run_results["errors"]
            if len(run_errors) != len(errors):
                print(f"Skipping {run} due to mismatched length of errors")
                continue
            axs[0].plot(timesteps, run_errors, label=f"{run_results["model"]} (Run {os.path.basename(run)})")
            axs[1].plot(timesteps, np.cumsum(run_errors), label=f"{run_results["model"]} (Run {os.path.basename(run)})")
    
    axs[0].legend()
    axs[1].legend()

    # Plot the decay scalars
    axs[2].plot(timesteps, avg_decay, label="Average Decay")
    axs[2].set(xlabel='Time in hours passed', ylabel='Average Decay Scalar')
    axs[2].set(title='Average Decay Scalar vs. Time')
    axs[2].fill_between(timesteps, min_decay, max_decay, alpha=0.2, label="Min/Max Decay")
    axs[2].legend()

    plt.savefig(f"./saved_timeseries_runs/{now}/graph.png")
    
    results = {
        "task": "PICO-REAL",
        "src": args.src,
        "model": "PICO-REAL",
        "errors": errors,
        "timesteps": timesteps.tolist(),
        "decay_ks": decay_ks.tolist(),
        "avg_decay": avg_decay.tolist(),
        "min_decay": min_decay.tolist(),
        "max_decay": max_decay.tolist(),
        "args": vars(args),
        "total_time": bar.format_dict['elapsed']
    }

    with open(f"./saved_timeseries_run/{now}/results.json", "w") as f:
        json.dump(results, f, indent=4)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Experiment Runner (Hardware)")
    parser.add_argument("--src", type=str, required=True, help="Dataset name")
    parser.add_argument("--save", type=bool, required=False, help="Whether to save the run details", default=True)
    parser.add_argument("--compare-to", type=str, required=False, nargs='*',
                        help="Files of past runs to compare against", default=None)
    parser.add_argument("--pico_serial_port", type=str,
                        required=True, help="Serial port for Pico device", default="/dev/ttyACM0")
    parser.add_argument(
        "--std", help="Standard deviation of noise", type=float, default=0.005
    )
    parser.add_argument(
        "--time", help="Time in hours to age LEDs", type=float, default=50000)
    parser.add_argument("--samples_per_timestep",
                        help="Number of noisy samples per LED per timestep", type=int, default=100)
    parser.add_argument("--timestep", help="Timestep size",
                        type=int, required=True, default=1000)
    parser.add_argument("--flickering_prob", help="Probability of flickering",
                        type=float, required=False, default=0.001)
    parser.add_argument("--device", type=str, required=False,
                        help="Device to use", default="cpu")
    parser.add_argument("--seed", type=int, required=False,
                        help="Seed for randomness", default=42)

    args = parser.parse_args()
    main(args)
