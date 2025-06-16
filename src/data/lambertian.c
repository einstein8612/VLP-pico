#include "lambertian.h" // Assuming this header defines LAMBERTIAN_EXPONENT
#include <math.h>

// Transmitter positions in centimeters, now as a 1D array
// Each set of {x, y, z} coordinates occupies NUM_DIMENSIONS consecutive elements.
const float TX_POSITIONS[] = {
    22.605263f, 17.605263f, 176.0f, 67.500000f, 19.000000f, 176.0f, 118.865028f, 16.098160f, 176.0f, 168.006027f, 18.993977f, 176.0f, 216.493500f, 17.983767f, 176.0f, 266.506836f, 16.561644f, 176.0f,
    21.426979f, 71.896721f, 176.0f, 67.368919f, 63.809563f, 176.0f, 116.055557f, 71.000000f, 176.0f, 166.500000f, 67.500000f, 176.0f, 211.047363f, 58.218948f, 176.0f, 274.500000f, 64.500000f, 176.0f,
    18.038462f, 122.038460f, 176.0f, 68.235008f, 121.258247f, 176.0f, 119.464584f, 121.483471f, 176.0f, 169.911758f, 122.000000f, 176.0f, 218.500000f, 123.500000f, 176.0f, 267.530243f, 118.281517f, 176.0f,
    17.875000f, 175.175003f, 176.0f, 69.693596f, 172.700211f, 176.0f, 117.507927f, 169.041992f, 176.0f, 171.500000f, 170.500000f, 176.0f, 216.479691f, 171.038071f, 176.0f, 271.150391f, 170.267807f, 176.0f,
    25.554163f, 221.024033f, 176.0f, 56.968086f, 227.989365f, 176.0f, 117.716270f, 224.837616f, 176.0f, 169.467438f, 223.916199f, 176.0f, 212.755722f, 224.659302f, 176.0f, 268.287903f, 223.020935f, 176.0f,
    24.210526f, 270.736847f, 176.0f, 67.481133f, 268.518860f, 176.0f, 124.021873f, 269.356567f, 176.0f, 173.342590f, 269.314819f, 176.0f, 224.484451f, 269.738739f, 176.0f, 268.758057f, 271.953766f, 176.0f
};

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