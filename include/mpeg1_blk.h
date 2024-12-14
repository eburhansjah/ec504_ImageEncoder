#include <stdlib.h>
#include "bit_vector.h"
#ifndef MPEG1_BLK
#define MPEG1_BLK

void encode_macroblock_header_i(unsigned address, short quant_scale , BITVECTOR* output);

void encode_block_header_i(unsigned char is_luma, int coeff[128], BITVECTOR* output);

void encode_block_end(BITVECTOR* output);

void mpeg1_slice(uint8_t quant_scale, uint8_t vertical_pos /* <= 175 */, BITVECTOR* out);


#endif
