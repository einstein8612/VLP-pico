#include "data.h"

#include "downsampled_data.h"
#include "lambertian.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    const float *data;
    int x_offset;
    int y_offset;
} QuadrantInfo;

static inline QuadrantInfo get_quadrant_info(int x, int y)
{
    QuadrantInfo qinfo = {.data = (void *)0, .x_offset = 0, .y_offset = 0};

    if (x >= cross_x_end && y >= cross_y_end) // Q1
    {
        qinfo.data = downsampled_data_q1;
        qinfo.x_offset = cross_x_end;
        qinfo.y_offset = cross_y_end;
    }
    else if (x <= cross_x_start && y >= cross_y_end) // Q2
    {
        qinfo.data = downsampled_data_q2;
        qinfo.y_offset = cross_y_end;
    }
    else if (x <= cross_x_start && y <= cross_y_start) // Q3
    {
        qinfo.data = downsampled_data_q3;
    }
    else if (x >= cross_x_end && y <= cross_y_start) // Q4
    {
        qinfo.data = downsampled_data_q4;
        qinfo.x_offset = cross_x_end;
    }

    return qinfo;
}

static inline int div_round_nearest(int a, int b)
{
    return (a + (b >> 1)) / b;
}

static inline int get_flattened_index(int w, int h)
{
    return h * (downsampled_data_q_width * downsampled_data_q_no_led) + w * downsampled_data_q_no_led;
}

static inline bool is_out_of_bounds(int x, int y)
{
    // Check if the coordinates are inside the cross points
    if (x >= cross_x_start && x < cross_x_end)
    {
        return true;
    }

    // Check if the coordinates are inside the cross points
    if (y >= cross_y_start && y < cross_y_end)
    {
        return true;
    }

    return false;
}

static inline int clamp_index(int value, int max)
{
    if (value < 0)
        return 0;
    if (value >= max)
        return max - 1;
    return value;
}

const float *get_nearest_data_all_leds(int x, int y, int *nearest_x, int *nearest_y)
{
    if (is_out_of_bounds(x, y))
        return (void *)0; // Out of bounds

    QuadrantInfo qinfo = get_quadrant_info(x, y);
    if (!qinfo.data)
        return (void *)0;

    // Adjust to quadrant space
    int local_x = x - qinfo.x_offset;
    int local_y = y - qinfo.y_offset;

    *nearest_x = div_round_nearest(local_x, downsampled_data_factor);
    *nearest_y = div_round_nearest(local_y, downsampled_data_factor);

    *nearest_x = clamp_index(*nearest_x, downsampled_data_q_width);
    *nearest_y = clamp_index(*nearest_y, downsampled_data_q_height);

    int flattened_index = get_flattened_index(*nearest_x, *nearest_y);

    // Convert nearest_x/y back to global coordinates
    *nearest_x = *nearest_x * downsampled_data_factor + qinfo.x_offset;
    *nearest_y = *nearest_y * downsampled_data_factor + qinfo.y_offset;

    return &qinfo.data[flattened_index];
}

int get_augmented_data(int x, int y, float *out_data)
{
    int nearest_x, nearest_y;
    const float *data = get_nearest_data_all_leds(x, y, &nearest_x, &nearest_y);
    if (!data)
    {
        return -1;
    }

    for (int i = 0; i < TX_POSITIONS_COUNT; i++)
    {
        out_data[i] = reconstruct_rss_lambertian_float(
            data[i],
            x, y,
            nearest_x, nearest_y,
            i);
    }

    return 0;
}

float get_augmented_data_for_led(int x, int y, int l_index)
{
    int nearest_x, nearest_y;
    const float *data = get_nearest_data_all_leds(x, y, &nearest_x, &nearest_y);
    if (!data)
    {
        return -1;
    }

    return reconstruct_rss_lambertian_float(
        data[l_index],
        x, y,
        nearest_x, nearest_y,
        l_index);
}
