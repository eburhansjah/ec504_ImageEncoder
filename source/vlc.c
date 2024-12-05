#include <stdio.h>
#include "bit_vector.h"
#include "mpeg1.h"
#define MAX_VALUE 33

// Special constants for non-numeric values
#define MACROBLOCK_STUFFING 34
#define MACROBLOCK_ESCAPE 35

struct vlc_macroblock
{
    const char* binstring;
    unsigned bit_len;
};

struct vlc_block
{
    const char* binstring;
    unsigned bit_len;
};

#define VLC_MACBLK(str) {str, sizeof(str)}
#define VLC_BLK(str) {str, sizeof(str)}


// Define the lookup table directly, using an array of strings
struct vlc_macroblock encoding_table[MAX_VALUE + 3] = {
    {NULL, 0},                   // Placeholder for 0, since our values start at 1
    VLC_MACBLK("1"),                    // Value 1
    VLC_MACBLK("011"),                  // Value 2
    VLC_MACBLK("010"),                  // Value 3
    VLC_MACBLK("0011"),                 // Value 4
    VLC_MACBLK("0010"),                 // Value 5
    VLC_MACBLK("00011"),               // Value 6
    VLC_MACBLK("00010"),               // Value 7
    VLC_MACBLK("0000111"),             // Value 8
    VLC_MACBLK("0000110"),             // Value 9
    VLC_MACBLK("00001011"),            // Value 10
    VLC_MACBLK("00001010"),            // Value 11
    VLC_MACBLK("00001001"),            // Value 12
    VLC_MACBLK("00001000"),            // Value 13
    VLC_MACBLK("00000111"),            // Value 14
    VLC_MACBLK("00000110"),            // Value 15
    VLC_MACBLK("0000010111"),         // Value 16
    VLC_MACBLK("0000010110"),         // Value 17
    VLC_MACBLK("0000010101"),         // Value 18
    VLC_MACBLK("0000010100"),         // Value 19
    VLC_MACBLK("0000010011"),         // Value 20
    VLC_MACBLK("0000010010"),         // Value 21
    VLC_MACBLK("00000100011"),        // Value 22
    VLC_MACBLK("00000100010"),        // Value 23
    VLC_MACBLK("00000100001"),        // Value 24
    VLC_MACBLK("00000100000"),        // Value 25
    VLC_MACBLK("00000011111"),        // Value 26
    VLC_MACBLK("00000011110"),        // Value 27
    VLC_MACBLK("00000011101"),        // Value 28
    VLC_MACBLK("00000011100"),        // Value 29
    VLC_MACBLK("00000011011"),        // Value 30
    VLC_MACBLK("00000011010"),        // Value 31
    VLC_MACBLK("00000011001"),        // Value 32
    VLC_MACBLK("00000011000"),         // Value 33
    VLC_MACBLK("00000001111"), /* Note! redudant here! this is for stuffing and escape */
    VLC_MACBLK("00000001000") 
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

struct vlc_macroblock mv_encoding_table[17] = {
    VLC_MACBLK("1"), // 0
    VLC_MACBLK("010"),
    VLC_MACBLK("0010"),
    VLC_MACBLK("00010"),
    VLC_MACBLK("0000110"), // 4
    VLC_MACBLK("00001010"), // 5
    VLC_MACBLK("00001000"), // 6
    VLC_MACBLK("00000110"),
    VLC_MACBLK("0000010110"),
    VLC_MACBLK("0000010100"),
    VLC_MACBLK("0000010010"),
    VLC_MACBLK("00000100010"), // 11
    VLC_MACBLK("00000100000"),
    VLC_MACBLK("00000011110"), // 13
    VLC_MACBLK("00000011100"),
    VLC_MACBLK("00000011010"),
    VLC_MACBLK("00000011000"), // 16
};

// Function to get the encoded bits for a given value
BITVECTOR *encode_macblk_address_value(int value) {
    BITVECTOR* res;
    int nvalue = 0;
    if (value >= -16 && value <= 16) {
        if (value < 0) nvalue = -value;
        res = bitvector_new(mv_encoding_table[nvalue].binstring, mv_encoding_table[nvalue].bit_len);
        if (value < 0) { bitvector_pos(res, -1); bitvecotr_put_bit(res, 1); };
        return res;
    }
    return NULL;
}

struct vlc_macroblock dc_sz_luma_table[9] = {
    VLC_BLK("100"), // 0
    VLC_BLK("00"), // 1
    VLC_BLK("101"),
    VLC_BLK("110"),
    VLC_BLK("1110"),
    VLC_BLK("11110"),
    VLC_BLK("111110"),
    VLC_BLK("1111110"), // 8
};

struct vlc_macroblock dc_sz_luma_table[9] = {
    VLC_BLK("00"), // 0
    VLC_BLK("101"), // 1
    VLC_BLK("110"),
    VLC_BLK("1110"),
    VLC_BLK("11110"),
    VLC_BLK("111110"),
    VLC_BLK("1111110"),
    VLC_BLK("11111110"), // 8
};

#define VLC_BLK_E(str) {str, sizeof(str) + 1}

struct vlc_block_rle
{
    unsigned run;
    unsigned level;
    struct vlc_block code;
};

// offset into the vlc lookup table
unsigned int blk_rle_lookup[] = {
    0, 39, 57, 62, 66, 69, 72, 75, 77, 79, 81, 83, 85, 87, 89, 91, 93, 95
};

struct vlc_block_rle blk_rle_table[] = {
{0	,2	,VLC_BLK_E("0100")},
{0	,3	,VLC_BLK_E("00101")},
{0	,4	,VLC_BLK_E("0000110")},
{0	,5	,VLC_BLK_E("00100110")},
{0	,6	,VLC_BLK_E("00100001")},
{0	,7	,VLC_BLK_E("0000001010")},
{0	,8	,VLC_BLK_E("000000011101")},
{0	,9	,VLC_BLK_E("000000011000")},
{0	,10	,VLC_BLK_E("000000010011")},
{0	,11	,VLC_BLK_E("000000010000")},
{0	,12	,VLC_BLK_E("0000000011010")},
{0	,13	,VLC_BLK_E("0000000011001")},
{0	,14	,VLC_BLK_E("0000000011000")},
{0	,15	,VLC_BLK_E("0000000010111")},
{0	,16	,VLC_BLK_E("00000000011111")},
{0	,17	,VLC_BLK_E("00000000011110")},
{0	,18	,VLC_BLK_E("00000000011101")},
{0	,19	,VLC_BLK_E("00000000011100")},
{0	,20	,VLC_BLK_E("00000000011011")},
{0	,21	,VLC_BLK_E("00000000011010")},
{0	,22	,VLC_BLK_E("00000000011001")},
{0	,23	,VLC_BLK_E("00000000011000")},
{0	,24	,VLC_BLK_E("00000000010111")},
{0	,25	,VLC_BLK_E("00000000010110")},
{0	,26	,VLC_BLK_E("00000000010101")},
{0	,27	,VLC_BLK_E("00000000010100")},
{0	,28	,VLC_BLK_E("00000000010011")},
{0	,29	,VLC_BLK_E("00000000010010")},
{0	,30	,VLC_BLK_E("00000000010001")},
{0	,31	,VLC_BLK_E("00000000010000")},
{0	,32	,VLC_BLK_E("000000000011000")},
{0	,33	,VLC_BLK_E("000000000010111")},
{0	,34	,VLC_BLK_E("000000000010110")},
{0	,35	,VLC_BLK_E("000000000010101")},
{0	,36	,VLC_BLK_E("000000000010100")},
{0	,37	,VLC_BLK_E("000000000010011")},
{0	,38	,VLC_BLK_E("000000000010010")},
{0	,39	,VLC_BLK_E("000000000010001")},
{0	,40	,VLC_BLK_E("000000000010000")},
{1	,1	,VLC_BLK_E("011")},
{1	,2	,VLC_BLK_E("000110")},
{1	,3	,VLC_BLK_E("00100101")},
{1	,4	,VLC_BLK_E("0000001100")},
{1	,5	,VLC_BLK_E("000000011011")},
{1	,6	,VLC_BLK_E("0000000010110")},
{1	,7	,VLC_BLK_E("0000000010101")},
{1	,8	,VLC_BLK_E("000000000011111")},
{1	,9	,VLC_BLK_E("000000000011110")},
{1	,10	,VLC_BLK_E("000000000011101")},
{1	,11	,VLC_BLK_E("000000000011100")},
{1	,12	,VLC_BLK_E("000000000011011")},
{1	,13	,VLC_BLK_E("000000000011010")},
{1	,14	,VLC_BLK_E("000000000011001")},
{1	,15	,VLC_BLK_E("0000000000010011")},
{1	,16	,VLC_BLK_E("0000000000010010")},
{1	,17	,VLC_BLK_E("0000000000010001")},
{1	,18	,VLC_BLK_E("0000000000010000")},
{2	,1	,VLC_BLK_E("0101")},
{2	,2	,VLC_BLK_E("0000100")},
{2	,3	,VLC_BLK_E("0000001011")},
{2	,4	,VLC_BLK_E("000000010100")},
{2	,5	,VLC_BLK_E("0000000010100")},
{3	,1	,VLC_BLK_E("00111")},
{3	,2	,VLC_BLK_E("00100100")},
{3	,3	,VLC_BLK_E("000000011100")},
{3	,4	,VLC_BLK_E("0000000010011")},
{4	,1	,VLC_BLK_E("00110")},
{4	,2	,VLC_BLK_E("0000001111")},
{4	,3	,VLC_BLK_E("000000010010")},
{5	,1	,VLC_BLK_E("000111")},
{5	,2	,VLC_BLK_E("0000001001")},
{5	,3	,VLC_BLK_E("0000000010010")},
{6	,1	,VLC_BLK_E("000101")},
{6	,2	,VLC_BLK_E("000000011110")},
{6	,3	,VLC_BLK_E("0000000000010100")},
{7	,1	,VLC_BLK_E("000100")},
{7	,2	,VLC_BLK_E("000000010101")},
{8	,1	,VLC_BLK_E("0000111")},
{8	,2	,VLC_BLK_E("000000010001")},
{9	,1	,VLC_BLK_E("0000101")},
{9	,2	,VLC_BLK_E("0000000010001")},
{10	,1	,VLC_BLK_E("00100111")},
{10	,2	,VLC_BLK_E("0000000010000")},
{11	,1	,VLC_BLK_E("00100011")},
{11	,2	,VLC_BLK_E("0000000000011010")},
{12	,1	,VLC_BLK_E("00100010")},
{12	,2	,VLC_BLK_E("0000000000011001")},
{13	,1	,VLC_BLK_E("00100000")},
{13	,2	,VLC_BLK_E("0000000000011000")},
{14	,1	,VLC_BLK_E("0000001110")},
{14	,2	,VLC_BLK_E("0000000000010111")},
{15	,1	,VLC_BLK_E("0000001101")},
{15	,2	,VLC_BLK_E("0000000000010110")},
{16	,1	,VLC_BLK_E("0000001000")},
{16	,2	,VLC_BLK_E("000000000010101")},
{17	,1	,VLC_BLK_E("000000011111")},
{18	,1	,VLC_BLK_E("000000011010")},
{19	,1	,VLC_BLK_E("000000011001")},
{20	,1	,VLC_BLK_E("000000010111")},
{21	,1	,VLC_BLK_E("000000010110")},
{22	,1	,VLC_BLK_E("0000000011111")},
{23	,1	,VLC_BLK_E("0000000011110")},
{24	,1	,VLC_BLK_E("0000000011101")},
{25	,1	,VLC_BLK_E("0000000011100")},
{26	,1	,VLC_BLK_E("0000000011011")},
{27	,1	,VLC_BLK_E("0000000000011111")},
{28	,1	,VLC_BLK_E("0000000000011110")},
{29	,1	,VLC_BLK_E("0000000000011101")},
{30	,1	,VLC_BLK_E("0000000000011100")},
{31	,1	,VLC_BLK_E("0000000000011011")}
};