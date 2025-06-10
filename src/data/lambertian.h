#ifndef LAMBERTIAN_H
#define LAMBERTIAN_H

// Precomputed Lambertian exponent: ((log(2.0) / log(cos(pi/12))) + 3) / 2
#define LAMBERTIAN_EXPONENT 11.4968636793f

#define NUM_DIMENSIONS 3      // Define the number of dimensions for each position (x, y, z)
#define TX_POSITIONS_COUNT 36 // Total number of transmitter positions

extern const float TX_POSITIONS[];

/**
 * Reconstructs the RSS at (x1, y1) given a reference RSS at (x2, y2),
 * using the Lambertian model with an LED at (xl, yl, zl).
 *
 * @param rss_ref RSS value at reference point (x2, y2)
 * @param x1, y1 Coordinates of the point to reconstruct
 * @param x2, y2 Coordinates of the reference point
 * @param l_index Index of the LED position in TX_POSITIONS
 * @return Reconstructed RSS at (x1, y1)
 */
float reconstruct_rss_lambertian_float(
    float rss_ref,
    float x1, float y1,
    float x2, float y2,
    int l_index // LED position on the plane
);

#endif // LAMBERTIAN_H
