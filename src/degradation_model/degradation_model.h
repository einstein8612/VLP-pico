#ifndef DEGRADATION_MODEL_H
#define DEGRADATION_MODEL_H

#include "../data/lambertian.h"

#define MAX_SAMPLES 50            // Maximum number of samples to store for each LED
#define BUFFER_SIZE_LEDS 1800     // 36 LEDs * 50 samples per LED
#define BUFFER_SIZE_POSITIONS 100 // 2D * 50 samples per LED

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
 */
void add_sample(float sample[TX_POSITIONS_COUNT], int x, int y);

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

#endif // DEGRADATION_MODEL_H