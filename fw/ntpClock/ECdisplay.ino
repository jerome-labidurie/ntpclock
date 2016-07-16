#include "ECdisplay.h"

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
 *   3  2  1  0   7  6  5  4 for digit 0
 *
 *    ledout0   |  ledout1
 *  11 10  9  8  15 14 13 12 for digit 1
 *
 * eg for number 2 in digit 0, we must ligth segments 4, 5, 2, 1, 0
 * so it's 0b01110011
 * for number 2 in digit 1, we must ligth segments 11, 12, 9, 8, 14
 * so 0b10110101
 *
 */
uint8_t cars[NBCHAR][2] = {
    // digit 0 ,   digit 1
    {0b10110111, 0b11010111}, // 0
    {0b00000110, 0b00000011}, // 1
    {0b01110011, 0b10110101}, // 2
    {0b01010111, 0b10100111}, // 3
    {0b11000110, 0b01100011}, // 4
    {0b11010101, 0b11100110}, // 5
    {0b11110101, 0b11110110}, // 6
    {0b00000111, 0b10000011}, // 7
    {0b11110111, 0b11110111}, // 8
    {0b11010111, 0b11100111}, // 9
    {0b11100111, 0b11110011}, // A
    {0b11110100, 0b01110110}, // b
    {0b10110001, 0b11010100}, // C
    {0b01110110, 0b00110111}, // d
    {0b11110001, 0b11110100}, // E
    {0b11100001, 0b11110000}, // F
    {0b10110101, 0b11010110}, // G
    {0b11100110, 0b01110011}, // H
    {0b00000110, 0b00000011}, // I
    {0b00110110, 0b00010111}, // J
    {0b11100110, 0b01110011}, // K
    {0b10110000, 0b01010100}, // L
    {0b00100101, 0b10010010}, // M
    {0b01100100, 0b00110010}, // n
    {0b01110100, 0b00110110}, // o
    {0b11100011, 0b11110001}, // P
    {0b11000111, 0b11100011}, // q
    {0b01100000, 0b00110000}, // r
    {0b11010101, 0b11100110}, // S
    {0b11110000, 0b01110100}, // t
    {0b10110110, 0b01010111}, // U
    {0b00110100, 0b00010110}, // v
    {0b10010010, 0b01000101}, // W
    {0b11100110, 0b01110011}, // X
    {0b11010110, 0b01100111}, // Y
    {0b01110011, 0b10110101}, // Z
    {0b00000000, 0b00000000}, // space
    {0b00010000, 0b00000100}, // _
    {0b01000000, 0b00100000}  // -
};



/** interleave bits from 2 numbers
 *
 * Resulting number will have alterning bits from x and y
 * 1st number will have the MSB
 * interleave(0x55, 0xAA) -> 0x6666
 * interleave(0b01101001, 0b00001010) -> 0b0010100011000110
 *
 * @See https://graphics.stanford.edu/~seander/bithacks.html#InterleaveTableObvious for algorithm
 *
 * Using this function, we will be able to creates bytes for the 2 registers controling one digit.
 * Following TLC59116 datasheet page 17, ledout registers have 2 bits to control output.
 * 00 : off
 * 01 ! fully on                    interleave (0, cars[<character>][<digit>])
 * 10 : individual brithness        interleave (cars[<character>][<digit>], 0)
 * 11 : indiv & global brigthness   interleave (cars[<character>][<digit>], cars[<character>][<digit>])
 *
 * @param[in] x 1st number
 * @param[in] y 2nd number
 * @return interleaved number
 */
uint16_t interleave (uint8_t x, uint8_t y) {
    uint16_t r = 0;

    for (int i = 0; i < 16; i++) {
        r |= (y & 1U << i) << i | ((x & 1U << i) << (i+1));
    }
    return r;
} // interleave

