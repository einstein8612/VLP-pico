# VLP-Pico

A pico-specific build for the most optimal VLP system, as found in my [VLP simulation](https://github.com/einstein8612/VLP). Proper documentation is TBA.


## Getting started
```bash
$ mkdir third_party
$ cd third_party
$ git clone --recurse-submodules https://github.com/raspberrypi/pico-sdk.git
$ git clone --recurse-submodules https://github.com/raspberrypi/pico-tflmicro.git
$ git clone --recurse-submodules https://github.com/einstein8612/ransac-line.git
```

```bash
$ ./build.sh
```