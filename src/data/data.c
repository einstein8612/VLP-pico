#include "data.h"
#include "downsampled_data.h"

#include <stdint.h>

typedef struct {
    const float* data;
    int x_offset;
    int y_offset;
} QuadrantInfo;

static inline QuadrantInfo get_quadrant_info(int x, int y)
{
    QuadrantInfo qinfo = { .data = (void*)0, .x_offset = 0, .y_offset = 0 };

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

static inline int div_round_nearest(int a, int b) {
    return (a + (b >> 1)) / b;
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

static inline int clamp_index(int value, int max)
{
    if (value < 0)
        return 0;
    if (value >= max)
        return max - 1;
    return value;
}

const float* get_nearest_data_all_leds(int x, int y, int *nearest_x, int *nearest_y)
{
    // Ensure led_index is within bounds
    round_to_cross_points(&x, &y);

    QuadrantInfo qinfo = get_quadrant_info(x, y);
    if (!qinfo.data)
        return (void*) 0;

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
