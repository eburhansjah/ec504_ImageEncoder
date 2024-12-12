#include "mpeg1.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "bit_vector.h"

#define B(name, ...) bitvector_ ## name (__VA_ARGS__)
#define BV struct bitvector

// constant for macroblock
#define MACBLK_STUFFING 34
#define MACBLK_ESCAPE 35

#define MACBLK_TYPE_INTRA_P "1"
#define MACBLK_TYPE_INTRA_Q "01"

#define MACBLK_TYPE_PRED_MC "1"
#define MACBLK_TYPE_PRED_C "01"
#define MACBLK_TYPE_PRED_M "001"
#define MACBLK_TYPE_PRED_D "00011"
#define MACBLK_TYPE_PRED_MCQ "00010"
#define MACBLK_TYPE_PRED_CQ "00001"
#define MACBLK_TYPE_PRED_Q "000001"

// qscale set to -1 if the block do not use customize scale
void encode_macroblock_header_i(unsigned address, short qscale , BV* output) {
    while (address > 33) {
        // whenever larger than 33, we should have added a escape
        B(concat, output, encode_macblk_address_value(MACBLK_ESCAPE));
        address -= 33;
    }
    B(concat, output, encode_macblk_address_value(address)); // always something remain after padding

    if (qscale > 0) {
        B(concat, output, bitvector_new(MACBLK_TYPE_INTRA_Q, 2));
        B(put_byte_off, output, qscale & 0x1f /* only 5 bits */, 6, 3);
    } else {
        B(concat, output, bitvector_new(MACBLK_TYPE_INTRA_P, 2));
    }

    // B(print, output);

    // TODO more to add, this is just example

    // I blocks are simple
}

void encode_macroblock_end(BV* output) {
    B(put_bit, output, 1);
}


// qscale set to -1 if the block do not use customize scale
void encode_block_header_i(uint8_t is_luma, BV* output) {

    if (is_luma) {
        B(concat, output, bitvector_new("100", 3));
    } else {
        B(concat, output, bitvector_new("00", 2));
    }

    // B(print, output);

    // TODO more to add, this is just example

    // I blocks are simple
}

void encode_block_end(BV* output) {
    B(concat, output, bitvector_new("10", 2));
}

#undef B // release the definition to avoid surprises
