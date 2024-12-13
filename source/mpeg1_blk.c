#include "mpeg1.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "bit_vector.h"

#define B(name, ...) bitvector_ ## name (__VA_ARGS__)
#define BV struct bitvector

BV slice_start_code = {"\x00\x00\x01\x01", 32, 32, 32};

void mpeg1_slice(uint8_t quant_scale, uint8_t vertical_pos /* <= 175 */, BV* out) {
    B(concat, out, &slice_start_code);
    // B(put_byte_ent, out, vertical_pos + 1); // vertical pos starts with 1
    B(put_byte_off, out, quant_scale & 0x1f /* only 5 bits */, 5, 3);
    B(put_bit, out, 0); // fixed
    B(print, out);
    // exit(0);
    // The following should all be macroblocks
}

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

// quant_scale set to -1 if the block do not use customize scale
void encode_macroblock_header_i(unsigned address, short quant_scale , BV* output) {
    while (address > 33) {
        // whenever larger than 33, we should have added a escape
        B(concat, output, encode_macblk_address_value(MACBLK_ESCAPE));
        address -= 33;
    }
    B(concat, output, encode_macblk_address_value(address)); // always something remain after padding

    if (0) {
        B(concat, output, bitvector_new(MACBLK_TYPE_INTRA_Q, 2));
        B(put_byte_off, output, quant_scale & 0x1f /* only 5 bits */, 5, 3);
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

#include "image_processing.h"

// qscale set to -1 if the block do not use customize scale
void encode_block_header_i(unsigned char is_luma, int coeff[128], BV* output) {

    printf("Coeff starting value %d %d %d\n", coeff[0], coeff[1], is_luma);

    bitvector_print(output);

    if (coeff[0] != 0 && coeff[1] == 0) {
        int coe = coeff[0];
        if (coe < 0) coe = -coe;
        int sz = 1, probe = 1;
        int i = 0;
        for (;i < 8; i ++) {
            if (coe & (1 << (i - 1))) probe = i; // mark the msb
        }

        sz = probe;

        encode_coeff_sz_fast(output, sz, is_luma);

        if (coeff[0] < 0) {
            coe ^= (1 << (sz - 1)); // unmark lsb value
        }

        bitvector_put_byte_off(output, coe & 0xff, sz, 8 - sz);

        VLC_encode(coeff + 2, output);

        printf("Probed size %d\n", sz);
    } else { 

    if (is_luma) {
        B(concat, output, bitvector_new("100", 3));
    } else {
        B(concat, output, bitvector_new("00", 2));
    }

    VLC_encode(coeff, output);

    }

    // B(print, output);

    // TODO more to add, this is just example

    // I blocks are simple
}

void encode_block_end(BV* output) {
    B(concat, output, bitvector_new("10", 2));
}

#undef B // release the definition to avoid surprises
