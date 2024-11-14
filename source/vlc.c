#include <stdio.h>

#define MAX_VALUE 33

// Special constants for non-numeric values
#define MACROBLOCK_STUFFING 34
#define MACROBLOCK_ESCAPE 35

// Define the lookup table directly, using an array of strings
const char *encoding_table[MAX_VALUE + 1] = {
    NULL,                   // Placeholder for 0, since our values start at 1
    "1",                    // Value 1
    "011",                  // Value 2
    "010",                  // Value 3
    "0011",                 // Value 4
    "0010",                 // Value 5
    "00011",               // Value 6
    "00010",               // Value 7
    "0000111",             // Value 8
    "0000110",             // Value 9
    "00001011",            // Value 10
    "00001010",            // Value 11
    "00001001",            // Value 12
    "00001000",            // Value 13
    "00000111",            // Value 14
    "00000110",            // Value 15
    "0000010111",         // Value 16
    "0000010110",         // Value 17
    "0000010101",         // Value 18
    "0000010100",         // Value 19
    "0000010011",         // Value 20
    "0000010010",         // Value 21
    "00000100011",        // Value 22
    "00000100010",        // Value 23
    "00000100001",        // Value 24
    "00000100000",        // Value 25
    "00000011111",        // Value 26
    "00000011110",        // Value 27
    "00000011101",        // Value 28
    "00000011100",        // Value 29
    "00000011011",        // Value 30
    "00000011010",        // Value 31
    "00000011001",        // Value 32
    "00000011000"         // Value 33
};

// Define additional special cases
const char *macroblock_stuffing = "00000001111";
const char *macroblock_escape = "00000001000";

// Function to get the encoded bits for a given value
const char *encode_value(int value) {
    if (value >= 1 && value <= MAX_VALUE) {
        return encoding_table[value];
    } else if (value == MACROBLOCK_STUFFING) {
        return macroblock_stuffing;
    } else if (value == MACROBLOCK_ESCAPE) {
        return macroblock_escape;
    }
    return "Unknown value";
}