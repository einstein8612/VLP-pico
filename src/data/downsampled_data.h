#ifndef DOWNSAMPLED_DATA_H
#define DOWNSAMPLED_DATA_H

// Downsampled data arrays
extern float downsampled_data_q1[];
extern float downsampled_data_q2[];
extern float downsampled_data_q3[];
extern float downsampled_data_q4[];

// Metadata
extern unsigned int downsampled_data_q_height;
extern unsigned int downsampled_data_q_width;
extern unsigned int downsampled_data_q_no_led; // Number of LEDs in the downsampled data
extern unsigned int downsampled_data_q_len;
extern unsigned int downsampled_data_factor;

extern unsigned int cross_x_start;
extern unsigned int cross_x_end;
extern unsigned int cross_y_start;
extern unsigned int cross_y_end;

#endif // DOWNSAMPLED_DATA_H
