#include <stdint.h>
#include "bit_vector.h"

void mpeg1_file_header(uint32_t multiplex_rate, uint8_t out[12]);

void display_u8arr(uint8_t* buf, int32_t size);

BITVECTOR *encode_macblk_address_value(int value);