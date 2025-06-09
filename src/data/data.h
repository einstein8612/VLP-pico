#ifndef DATA_H
#define DATA_H

/*! \brief Get pointer to nearest downsampled data point for given coordinates
 *  
 * Returns a pointer to the start of the 36 LED values corresponding to the nearest 
 * downsampled data point based on input coordinates.
 *
 * \note The returned pointer gives direct access to 36 consecutive LED values 
 *       (starting at LED index 0). The pointer is NULL if the coordinates fall 
 *       outside the valid region.
 *
 * \param x The x-coordinate in the original (non-downsampled) space
 * \param y The y-coordinate in the original (non-downsampled) space
 * \param nearest_x Pointer to store the adjusted x-coordinate in the downsampled space
 * \param nearest_y Pointer to store the adjusted y-coordinate in the downsampled space
 * \return A const pointer to the first of 36 LED values, or NULL if out of bounds
 */
float* get_nearest_data_all_leds(int x, int y, int* nearest_x, int* nearest_y);

#endif // DATA_H