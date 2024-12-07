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

void encode_macroblock_header(unsigned address, BV* output) {
    while (address > 33) {
        // whenever larger than 33, we should have added a escape
        B(concat, output, encode_macblk_address_value(MACBLK_ESCAPE));
        address -= 33;
    }
    B(concat, output, encode_macblk_address_value(address)); // always something remain after padding

    B(print, output);

    // TODO more to add, this is just example
}


