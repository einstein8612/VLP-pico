import os

if len(os.sys.argv) == 1:
    print("Usage: python torch2tflite.py <input_file>")
    exit(1)

INPUT_FILE = os.sys.argv[1]

# Import necessary libraries

import torch
import torch.nn as nn

import tensorflow as tf

import numpy as np


# Define PyTorch model
class NormalizeInput(nn.Module):
    def forward(self, x: torch.Tensor) -> torch.Tensor:
        return x / (x.norm(dim=1, keepdim=True) + 1e-8)

model = nn.Sequential(
    NormalizeInput(),

    nn.Linear(36, 64),
    nn.ReLU6(),

    nn.Linear(64, 32),
    nn.LeakyReLU(0.1),

    nn.Linear(32, 2),
)

print("Defined PyTorch model")

# Define Keras model

model_tf = tf.keras.Sequential([
    tf.keras.layers.Dense(64, activation=tf.nn.relu6, name='linear1', input_shape=(36,)),
    tf.keras.layers.Dense(32, name='linear2'),
    tf.keras.layers.LeakyReLU(alpha=0.1),
    tf.keras.layers.Dense(2, name='linear3'),
])

print("Defined Keras model")

# Define layer names and corresponding PyTorch keys
layer_names = ['linear1', 'linear2', 'linear3']
pt_layer_keys = ['1', '3', '5']

# Triggers layer builds
with tf.device('/CPU:0'):
    model_tf(tf.zeros((1, 36)))

state_dict = torch.load(INPUT_FILE, map_location="cpu")

model.load_state_dict(state_dict)

for tf_name, pt_idx in zip(layer_names, pt_layer_keys):
    W = state_dict[f'{pt_idx}.weight'].numpy().T  # [in, out] for TF
    b = state_dict[f'{pt_idx}.bias'].numpy()
    model_tf.get_layer(tf_name).set_weights([W, b])

model_tf.trainable = False

print("Loaded weights into Keras and Torch model")

x = np.random.rand(1, 36).astype(np.float32)
x_norm = x / (np.linalg.norm(x, axis=1, keepdims=True) + 1e-8)

pt_output = model(torch.tensor(x)).detach().numpy()
with tf.device('/CPU:0'):
    tf_output = model_tf(x_norm)

print("PyTorch output:", pt_output)
print("TensorFlow output:", tf_output)

assert np.allclose(pt_output, tf_output, atol=1e-6), "Outputs are not close enough!"

def representative_dataset():
    tf.random.set_seed(42)
    for _ in range(1000):
        data = tf.random.uniform(shape=(1, 36), dtype=tf.float32)
        yield [data]

# Start converting to TFLite
converter = tf.lite.TFLiteConverter.from_keras_model(model_tf)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.representative_dataset = representative_dataset

# Ensure full integer quantization
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

tflite_model = converter.convert()

# Save
with open("model_int8.tflite", "wb") as f:
    f.write(tflite_model)

def convert_to_c_array(input_file, output_file):
    with open(input_file, "rb") as f:
        data = f.read()

    array_name = input_file.split('/')[-1].replace('.', '_')

    with open(output_file, "w") as f:
        f.write(f"unsigned char {array_name}[] = {{\n")
        for i in range(0, len(data), 12):
            bytes_chunk = data[i:i + 12]
            line = ", ".join(f"0x{byte:02x}" for byte in bytes_chunk)
            if i + 12 < len(data):
                f.write(f"  {line},\n")
            else:
                f.write(f"  {line}\n")
        f.write(f"}};\n")
        f.write(f"unsigned int {array_name}_len = {len(data)};\n")

# Example usage
convert_to_c_array("model_int8.tflite", "model_int8.c")
print("Converted TFLite model to C array format and saved to model_int8.c and model_int8.tflite")