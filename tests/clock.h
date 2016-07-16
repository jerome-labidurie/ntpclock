/** clock.h
 */

#include <stdio.h>
#include <stdint.h>

#ifndef __CLOCK_H__
#define __CLOCK_H__

/** Array of characters for driver registers.
 * Each driver controls 2 digits.
 * ledout0-1 drives right(numbered 0 in array) digit
 * ledout2-3 drives left(numbered 1) digit.
 * Here's pinout arrangement for each 7 seg display :
 *
 *    digit 1      digit 0
 *      _11          _4_
 *   10|   |12     3|   |5
 *     | 9 |        | 2 |
 *      ---          ---
 *    8|   |13     1|   |6
 *     |   |        |   |
 *      ---  .15     ---  .7
 *       14           0
 *
 * if a segment must be lighted, set 1, else set 0.
 * Bits are arranged :
 *    ledout0   |  ledout1
 *   3  2  1  0  7  6  5  4 for digit 0
 *
 *    ledout0   |  ledout1
 *  11 10  9  8 15 14 13 12 for digit 1
 *
 * eg for number 2 in digit 0, we must ligth segments 4, 5, 2, 1, 0
 * so it's 0b01110011
 * for number 2 in digit 1, we must ligth segments 11, 12, 9, 8, 14
 * so 0b10110101
 *
 */
#define NBCHAR 10+26
extern uint8_t cars[NBCHAR][2];
uint16_t interleave (uint8_t x, uint8_t y);

#endif
