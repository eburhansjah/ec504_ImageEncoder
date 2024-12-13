#include <stdlib.h>
#include "bit_vector.h"
#ifndef MPEG1_BLK
#define MPEG1_BLK

void encode_macroblock_header(unsigned address, BITVECTOR* output);

void encode_block_header_i(unsigned char is_luma, int coeff[128], BITVECTOR* output);

#endif
