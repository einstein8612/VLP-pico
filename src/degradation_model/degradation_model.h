#ifndef DEGRADATION_MODEL_H
#define DEGRADATION_MODEL_H

#include "../data/lambertian.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_SAMPLES 50            // Maximum number of samples to store for each LED
#define BUFFER_SIZE_LEDS (36 * MAX_SAMPLES)     // 36 LEDs * 50 samples per LED
#define BUFFER_SIZE_POSITIONS (2 * MAX_SAMPLES) // 2D * 50 samples per LED

/**
 * @brief Adds a new RSS sample and its corresponding position to the internal buffer.
 *
 * This function stores the provided RSS sample array and the 2D position (x, y).
 * When enough samples are collected (MAX_SAMPLES), it triggers an update of the
 * internal degradation scalars using a RANSAC-based fitting process.
 *
 * @param sample A float array of 36 RSS values for the current sample.
 * @param x The x-coordinate of the current sample's position.
 * @param y The y-coordinate of the current sample's position.
 * @return true if the sample was added and scalars were updated, false if not enough samples yet.
 */
bool add_sample(float sample[TX_POSITIONS_COUNT], float x, float y);

/**
 * @brief Returns a pointer to the current array of degradation scalars.
 *
 * These scalars represent the degradation factors for each LED position,
 * adjusted based on the collected samples.
 *
 * @return A pointer to a float array containing the degradation scalars.
 * The size of this array is TX_POSITIONS_COUNT.
 */
float *get_scalars();

#ifdef __cplusplus
}
#endif

#endif // DEGRADATION_MODEL_H