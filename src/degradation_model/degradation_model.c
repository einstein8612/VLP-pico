#include "degradation_model.h"

#include "../data/data.h"
#include "../data/lambertian.h"

#include "ransac_line.h"
#include <string.h>
#include <stdbool.h>

float sample_buffer_leds[BUFFER_SIZE_LEDS] = {0.0f};
int sample_buffer_positions[BUFFER_SIZE_POSITIONS] = {0};
int sample_amount = 0;

float scalars[TX_POSITIONS_COUNT] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

static inline int div_round_nearest(int a, int b)
{
    return (a + (b >> 1)) / b;
}

static void update_scalars()
{
    sample_amount = 0; // Reset to zero

    float samples[MAX_SAMPLES] = {0};
    float reference_samples[MAX_SAMPLES] = {0};

    for (int i = 0; i < TX_POSITIONS_COUNT; i++)
    {
        int sample_count = 0; // Valid sample count for this LED
        for (int j = 0; j < MAX_SAMPLES; j++)
        {
            // Get the reference RSS sample for the current LED position
            float ref = get_augmented_data_for_led(
                sample_buffer_positions[j*2],   // Use the first sample's x position
                sample_buffer_positions[j*2+1], // Use the first sample's y position
                i                               // LED index
            );
            float sample = sample_buffer_leds[j * TX_POSITIONS_COUNT + i];

            if (ref < 0.0f)
            {
                continue;
            }

            samples[sample_count] = sample;
            reference_samples[sample_count] = ref;
            sample_count++;
        }

        // Fit the samples to the reference samples using RANSAC
        float update = fit(
            samples, reference_samples, sample_count, 0.1f, 25, 42);
        scalars[i] *= update; // Ensure scalars do not decrease below 1.0
    }
}

bool add_sample(float sample[TX_POSITIONS_COUNT], float x, float y)
{
    // Copy the RSS sample to the buffer
    memcpy(&sample_buffer_leds[sample_amount * TX_POSITIONS_COUNT], sample, sizeof(float) * TX_POSITIONS_COUNT);
    // Store the position in the buffer
    sample_buffer_positions[sample_amount * 2] = div_round_nearest(x, 10);
    sample_buffer_positions[sample_amount * 2 + 1] = div_round_nearest(y, 10);

    // Increment the sample count
    sample_amount++;

    // Update scalars if we have enough samples
    if (sample_amount < MAX_SAMPLES)
    {
        // Not enough samples to update scalars
        return false;
    }
    update_scalars();
    return true;
}

float *get_scalars()
{
    // Return the current scalars
    return scalars;
}