#include "lambertian.h" // Assuming this header defines LAMBERTIAN_EXPONENT
#include <math.h>

// Transmitter positions in centimeters, now as a 1D array
// Each set of {x, y, z} coordinates occupies NUM_DIMENSIONS consecutive elements.
const float TX_POSITIONS[] = {
    23.0f, 17.0f, 176.0f, 73.0f, 17.0f, 176.0f, 124.0f, 17.0f, 176.0f, 174.0f, 17.0f, 176.0f, 224.0f, 17.0f, 176.0f, 274.0f, 17.0f, 176.0f,
    23.0f, 67.0f, 176.0f, 72.0f, 72.0f, 176.0f, 123.0f, 67.0f, 176.0f, 173.0f, 67.0f, 176.0f, 222.0f, 72.0f, 176.0f, 272.0f, 67.0f, 176.0f,
    23.0f, 117.0f, 176.0f, 73.0f, 117.0f, 176.0f, 124.0f, 117.0f, 176.0f, 174.0f, 117.0f, 176.0f, 224.0f, 117.0f, 176.0f, 273.0f, 117.0f, 176.0f,
    23.0f, 167.0f, 176.0f, 73.0f, 167.0f, 176.0f, 124.0f, 167.0f, 176.0f, 174.0f, 167.0f, 176.0f, 224.0f, 167.0f, 176.0f, 273.0f, 167.0f, 176.0f,
    23.0f, 217.0f, 176.0f, 72.0f, 222.0f, 176.0f, 123.0f, 217.0f, 176.0f, 172.0f, 217.0f, 176.0f, 222.0f, 222.0f, 176.0f, 271.0f, 217.0f, 176.0f,
    21.0f, 267.0f, 176.0f, 71.0f, 267.0f, 176.0f, 124.0f, 267.0f, 176.0f, 173.0f, 267.0f, 176.0f, 224.0f, 267.0f, 176.0f, 273.0f, 267.0f, 176.0f};

inline float reconstruct_rss_lambertian_float(
    float rss_ref,
    float x1, float y1,
    float x2, float y2,
    int l_index)
{
    // Calculate the starting index for the current LED's coordinates
    // Each LED has NUM_DIMENSIONS coordinates (x, y, z)
    int base_index = l_index * NUM_DIMENSIONS;

    float xl = TX_POSITIONS[base_index];
    float yl = TX_POSITIONS[base_index + 1];
    float zl = TX_POSITIONS[base_index + 2];

    float dx1 = x1 - xl;
    float dy1 = y1 - yl;
    float dx2 = x2 - xl;
    float dy2 = y2 - yl;

    float d1 = dx1 * dx1 + dy1 * dy1 + zl * zl;
    float d2 = dx2 * dx2 + dy2 * dy2 + zl * zl;

    return rss_ref * powf(d2 / d1, LAMBERTIAN_EXPONENT);
}