cmake_minimum_required(VERSION 3.13...3.27)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(third_party/pico-sdk/pico_sdk_init.cmake)

project(vlp_pico C CXX ASM)

# RP2040
set(PICO_BOARD pico)

add_compile_options(-Wall -Winline)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# === TFLM Setup library ===

# Set up TensorFlow Lite Micro include paths
add_subdirectory(third_party/pico-tflmicro)

# === TFLM END ===

# Add the path to the source files
include_directories(src)

# Add the executable. This will build the source file vlp_pico.c
file(GLOB_RECURSE VLP_SOURCES
    src/*.c
    src/*.cpp
)

add_executable(vlp_pico ${VLP_SOURCES})

if(DEBUG_LED)
    target_compile_definitions(vlp_pico PRIVATE DEBUG_LED) # Define DEBUG mode if needed
endif()

pico_enable_stdio_usb(vlp_pico 1)
pico_enable_stdio_uart(vlp_pico 0)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(vlp_pico pico_stdlib)
target_link_libraries(vlp_pico pico-tflmicro)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(vlp_pico)

# Add the path to the source files for tests
enable_testing()
add_subdirectory(tests)