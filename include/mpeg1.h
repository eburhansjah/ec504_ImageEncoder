#include <stdint.h>

void mpeg1_file_header(uint32_t multiplex_rate, uint8_t out[12]);

void display_u8arr(uint8_t* buf, int32_t size);