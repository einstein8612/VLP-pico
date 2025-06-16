import os

if len(os.sys.argv) == 1:
    print("Usage: python torch2tflite.py <input_file>")
    exit(1)

INPUT_FILE = os.sys.argv[1]

# Import necessary libraries

import torch
import torch.nn as nn

import tensorflow as tf
from tensorflow.keras import layers, models

import numpy as np


# Define PyTorch model
class BottleneckBlock(nn.Module):
    def __init__(self, dim, factor):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(dim, int(dim * factor)),
            nn.ReLU(),
            nn.Linear(int(dim * factor), dim)
        )
        self.activation = nn.ReLU()

    def forward(self, x):
        return self.activation(self.net(x) + x)  # residual

class MLPResNet(nn.Module):
    def __init__(self):
        super().__init__()
        self.norm = NormalizeInput()

        self.entry = nn.Sequential(
            nn.Linear(36, 256),
            nn.ReLU()
        )

        self.res_block1 = BottleneckBlock(256, 0.25)
        self.res_block2 = BottleneckBlock(256, 0.25)

        self.out = nn.Sequential(
            nn.Linear(256, 2)
        )

    def forward(self, x):
        x = self.norm(x)
        x = self.entry(x)
        x = self.res_block1(x)
        x = self.res_block2(x)
        return self.out(x)

class NormalizeInput(nn.Module):
    def forward(self, x: torch.Tensor) -> torch.Tensor:
        return x / (x.norm(dim=1, keepdim=True) + 1e-8)

model = MLPResNet()

print("Defined PyTorch model")

# Define Keras model
class BottleneckBlock(layers.Layer):
    def __init__(self, dim, factor, name_prefix=""):
        super().__init__()
        hidden_dim = int(dim * factor)
        self.net = models.Sequential([
            layers.Dense(hidden_dim, name=f'{name_prefix}_dense1'),
            layers.Activation('relu'),
            layers.Dense(dim, name=f'{name_prefix}_dense2')
        ])
        self.activation = layers.Activation('relu')

    def get_layer(self, name):
        return self.net.get_layer(name)

    def call(self, x):
        return self.activation(self.net(x) + x)

class MLPResNet(tf.keras.Model):
    def __init__(self):
        super().__init__()
        self.entry = models.Sequential([
            layers.Dense(256, input_shape=(36,), name='linear1'),
            layers.ReLU()
        ], name='entry')

        self.res_block1 = BottleneckBlock(256, 0.25, name_prefix="res1")
        self.res_block2 = BottleneckBlock(256, 0.25, name_prefix="res2")

        self.out = layers.Dense(2, name='linear3')

    def call(self, x):
        # x = self.norm(x) # Remember to normalize input in C code
        x = self.entry(x)
        x = self.res_block1(x)
        x = self.res_block2(x)
        return self.out(x)

model_tf = MLPResNet()

print("Defined Keras model")

# Define layer names and corresponding PyTorch keys
layer_names = ['entry.linear1', 'bottleneck_block.res1_dense1', 'bottleneck_block.res1_dense2', 'bottleneck_block_1.res2_dense1', 'bottleneck_block_1.res2_dense2', 'linear3']
pt_layer_keys = ['entry.0', 'res_block1.net.0', 'res_block1.net.2', 'res_block2.net.0', 'res_block2.net.2', 'out.0']

# Triggers layer builds
with tf.device('/CPU:0'):
    model_tf(tf.zeros((1, 36)))

state_dict = torch.load(INPUT_FILE, map_location="cpu")

model.load_state_dict(state_dict)

for tf_name, pt_idx in zip(layer_names, pt_layer_keys):
    W = state_dict[f'{pt_idx}.weight'].numpy().T  # [in, out] for TF
    b = state_dict[f'{pt_idx}.bias'].numpy()

    walk = model_tf.get_layer(tf_name.split('.')[0])
    for part in tf_name.split('.')[1:]:
        walk = walk.get_layer(part)

    walk.set_weights([W, b])

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
    # Load your CSV once
    data = np.loadtxt("pc_interface/test.csv", delimiter=",", skiprows=1, dtype=np.float32)[:, 2:]

    for row in data:
        noise = np.random.normal(0, 0.005, size=row.shape)  # adjust stddev as needed
        row_noisy = row + noise

        row_noisy = row_noisy / (np.linalg.norm(row_noisy) + 1e-8)
        yield [tf.convert_to_tensor([row_noisy], dtype=tf.float32)]

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