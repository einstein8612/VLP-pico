#include "degradation_model.h"

#include "../data/data.h"
#include "../data/lambertian.h"

#include "ransac_line.h"
#include <string.h>

float sample_buffer_leds[BUFFER_SIZE_LEDS] = {0.0f};
int sample_buffer_positions[BUFFER_SIZE_POSITIONS] = {0};
int sample_amount = 0;

float scalars[TX_POSITIONS_COUNT] = {1};

static void update_scalars()
{
    sample_amount = 0; // Reset to zero

    float samples[MAX_SAMPLES] = {0};
    float reference_samples[MAX_SAMPLES] = {0};

    for (int i = 0; i < TX_POSITIONS_COUNT; i++)
    {
        for (int j = 0; j < MAX_SAMPLES; j++)
        {
            // Get the reference RSS sample for the current LED position
            float ref = get_augmented_data_for_led(
                sample_buffer_positions[j*2], // Use the first sample's x position
                sample_buffer_positions[j*2+1], // Use the first sample's y position
                i                           // LED index
            );

            samples[i] = sample_buffer_leds[j * TX_POSITIONS_COUNT + i];
            reference_samples[i] = ref;
        }

        // Fit the samples to the reference samples using RANSAC
        scalars[i] *= fit(
            samples, reference_samples, MAX_SAMPLES, 0.1f, 100, 0);
    }
}

void add_sample(float sample[TX_POSITIONS_COUNT], int x, int y)
{
    // Copy the RSS sample to the buffer
    memcpy(&sample_buffer_leds[sample_amount * TX_POSITIONS_COUNT], sample, sizeof(float) * TX_POSITIONS_COUNT);
    // Store the position in the buffer
    sample_buffer_positions[sample_amount * 2] = x;
    sample_buffer_positions[sample_amount * 2 + 1] = y;

    // Increment the sample count
    sample_amount++;

    // Update scalars if we have enough samples
    if (sample_amount < MAX_SAMPLES)
    {
        // Not enough samples to update scalars
        return;
    }
    update_scalars();
}

float *get_scalars()
{
    // Return the current scalars
    return scalars;
}