#include <stdint.h>
#include "bit_vector.h"
#include <stdio.h>

#ifndef MPEG1_VG
#define MPEG1_VG 1
#include "mpeg1_blk.h"
#include "mpeg1_enc.h"

/*
struct vlc_block {
    BITVECTOR* content; // the encoded vlc content
};
*/

// MOVED THE CONTENTS OF THIS TO VLC.C
/*
struct vlc_macroblock
{
    unsigned x; // by width
    unsigned y; // by height
    unsigned time; // in second
    struct vlc_block blocks[6]; // the 4:2:0 blocks
};
*/

struct vlc_macroblock
{
    const char* binstring;
    unsigned bit_len;
    //unsigned x; // by width
    //unsigned y; // by height
    //unsigned time; // in second
    //struct vlc_block blocks[6]; // the 4:2:0 blocks
};

struct vlc_block
{
    const char* binstring;
    unsigned bit_len;
};

void mpeg1_file_header(uint32_t multiplex_rate, uint8_t out[12]);

void display_u8arr(uint8_t* buf, int32_t size);

BITVECTOR *encode_macblk_address_value(int value);

BITVECTOR *encode_blk_coeff(int run, int level, int first);

void encode_coeff_sz_fast(BITVECTOR* output, char value, char is_luma);

#endif