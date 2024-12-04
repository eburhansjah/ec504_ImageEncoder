#include <stdio.h>
#include "bit_vector.h"
#include "mpeg1.h"
#define MAX_VALUE 33

// Special constants for non-numeric values
#define MACROBLOCK_STUFFING 34
#define MACROBLOCK_ESCAPE 35

struct vlc_macroblock_len
{
    const char* binstring;
    unsigned bit_len;
};

#define VLC_MACBLK_LENS(str) {str, sizeof(str)}


// Define the lookup table directly, using an array of strings
struct vlc_macroblock_len encoding_table[MAX_VALUE + 3] = {
    {NULL, 0},                   // Placeholder for 0, since our values start at 1
    VLC_MACBLK_LENS("1"),                    // Value 1
    VLC_MACBLK_LENS("011"),                  // Value 2
    VLC_MACBLK_LENS("010"),                  // Value 3
    VLC_MACBLK_LENS("0011"),                 // Value 4
    VLC_MACBLK_LENS("0010"),                 // Value 5
    VLC_MACBLK_LENS("00011"),               // Value 6
    VLC_MACBLK_LENS("00010"),               // Value 7
    VLC_MACBLK_LENS("0000111"),             // Value 8
    VLC_MACBLK_LENS("0000110"),             // Value 9
    VLC_MACBLK_LENS("00001011"),            // Value 10
    VLC_MACBLK_LENS("00001010"),            // Value 11
    VLC_MACBLK_LENS("00001001"),            // Value 12
    VLC_MACBLK_LENS("00001000"),            // Value 13
    VLC_MACBLK_LENS("00000111"),            // Value 14
    VLC_MACBLK_LENS("00000110"),            // Value 15
    VLC_MACBLK_LENS("0000010111"),         // Value 16
    VLC_MACBLK_LENS("0000010110"),         // Value 17
    VLC_MACBLK_LENS("0000010101"),         // Value 18
    VLC_MACBLK_LENS("0000010100"),         // Value 19
    VLC_MACBLK_LENS("0000010011"),         // Value 20
    VLC_MACBLK_LENS("0000010010"),         // Value 21
    VLC_MACBLK_LENS("00000100011"),        // Value 22
    VLC_MACBLK_LENS("00000100010"),        // Value 23
    VLC_MACBLK_LENS("00000100001"),        // Value 24
    VLC_MACBLK_LENS("00000100000"),        // Value 25
    VLC_MACBLK_LENS("00000011111"),        // Value 26
    VLC_MACBLK_LENS("00000011110"),        // Value 27
    VLC_MACBLK_LENS("00000011101"),        // Value 28
    VLC_MACBLK_LENS("00000011100"),        // Value 29
    VLC_MACBLK_LENS("00000011011"),        // Value 30
    VLC_MACBLK_LENS("00000011010"),        // Value 31
    VLC_MACBLK_LENS("00000011001"),        // Value 32
    VLC_MACBLK_LENS("00000011000"),         // Value 33
    VLC_MACBLK_LENS("00000001111"), /* Note! redudant here! this is for stuffing and escape */
    VLC_MACBLK_LENS("00000001000") 
};

// Define additional special cases
// const char *macroblock_stuffing = "00000001111";
// const char *macroblock_escape = "00000001000";

// Function to get the encoded bits for a given value
BITVECTOR *encode_macblk_address_value(int value) {
    if (value >= 1 && value <= MAX_VALUE + 2) {
        return bitvector_new(encoding_table[value].binstring, encoding_table[value].bit_len);
    }
    return NULL;
}