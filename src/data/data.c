#include "data.h"
#include "downsampled_data.h"

#include <stdint.h>

static inline int div_round_nearest(int a, int b) {
    return (a + (b >> 1)) / b;
}


static inline int get_quadrant(int x, int y)
{
    if (x >= cross_x_end && y >= cross_y_end)
    {
        return 0; // Quadrant 1
    }
    else if (x <= cross_x_start && y >= cross_y_end)
    {
        return 1; // Quadrant 2
    }
    else if (x <= cross_x_start && y <= cross_y_start)
    {
        return 2; // Quadrant 3
    }
    else if (x >= cross_x_end && y <= cross_y_start)
    {
        return 3; // Quadrant 4
    }
    return -1; // Invalid quadrant
}

static inline int get_flattened_index(int w, int h)
{
    return h * (downsampled_data_q_width * downsampled_data_q_len) + w * downsampled_data_q_no_led;
}

static inline void round_to_cross_points(int *x, int *y)
{
    // Round w to the nearest cross point
    if (*x >= cross_x_start && *x <= cross_x_end)
    {
        int distance_to_start = *x - cross_x_start;
        int distance_to_end = cross_x_end - *x;

        *x = (distance_to_start < distance_to_end) ? cross_x_start : cross_x_end;
    }

    // Round h to the nearest cross point
    if (*y >= cross_y_start && *y <= cross_y_end)
    {
        int distance_to_start = *y - cross_y_start;
        int distance_to_end = cross_y_end - *y;

        *y = (distance_to_start < distance_to_end) ? cross_y_start : cross_y_end;
    }
}

static inline float *get_quadrant_data_and_adjust_coords(int *x, int *y, int quadrant)
{
    switch (quadrant)
    {
    case 0: // Quadrant 1
        *x -= cross_x_end;
        *y -= cross_y_end;
        return downsampled_data_q1;
    case 1: // Quadrant 2
        *y -= cross_y_end;
        return downsampled_data_q2;
    case 2: // Quadrant 3
        return downsampled_data_q3;
    case 3: // Quadrant 4
        *x -= cross_x_end;
        return downsampled_data_q4;
    default:
        return (void *)0; // Invalid quadrant
    }
}

/*!
 * Adjusts the coordinates to the global space based on the quadrant coords.
 */
static inline void get_global_coords(int *x, int *y, int quadrant)
{
    switch (quadrant)
    {
    case 0: // Quadrant 1
        *x = *x * downsampled_data_factor + cross_x_end;
        *y = *y * downsampled_data_factor + cross_y_end;
        break;
    case 1: // Quadrant 2
        *x = *x * downsampled_data_factor;
        *y = *y * downsampled_data_factor + cross_y_end;
        break;
    case 2: // Quadrant 3
        *x = *x * downsampled_data_factor;
        *y = *y * downsampled_data_factor;
        break;
    case 3: // Quadrant 4
        *x = *x * downsampled_data_factor + cross_x_end;
        *y = *y * downsampled_data_factor;
        break;
    }
}

static inline int clamp_index(int value, int max)
{
    if (value < 0)
        return 0;
    if (value >= max)
        return max - 1;
    return value;
}

float *get_nearest_data_all_leds(int x, int y, int *nearest_x, int *nearest_y)
{
    // Ensure led_index is within bounds
    round_to_cross_points(&x, &y);

    int quadrant = get_quadrant(x, y);

    // Get the quadrant based on the coordinates and adjust them to quadrant space
    float *downsampled_data_q = get_quadrant_data_and_adjust_coords(&x, &y, quadrant);
    if (!downsampled_data_q)
        return (void *)0;

    // Get downsampled data index
    *nearest_x = div_round_nearest(x, downsampled_data_factor);
    *nearest_y = div_round_nearest(y, downsampled_data_factor);

    // Ensure we don't go out of bounds
    *nearest_x = clamp_index(*nearest_x, downsampled_data_q_width);
    *nearest_y = clamp_index(*nearest_y, downsampled_data_q_height);

    // Calculate the flattened index for the downsampled data
    int flattened_index = get_flattened_index(*nearest_x, *nearest_y);

    // Adjust the x coordinate back to real space
    get_global_coords(nearest_x, nearest_y, quadrant);

    return &downsampled_data_q[flattened_index];
}
