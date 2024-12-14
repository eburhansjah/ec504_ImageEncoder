#include <stdlib.h>
#include <stdint.h>
#include "bit_vector.h"
#include "mpeg1_blk.h"
#ifndef MPEG1_ENC
#define MPEG1_ENC

void mpeg1_file_header(uint32_t multiplex_rate, uint8_t out[12]);
void mpeg1_sys_header(uint32_t multiplex_rate, uint8_t packet_num, uint8_t out[15]);
void mpeg1_packet_header(uint32_t pts_optinal ,uint8_t *out);
void mpeg1_sequence_header(uint16_t width, uint16_t height, uint8_t aspect_ratio, uint8_t frame_rate, uint8_t yby_size, uint8_t* out);
void mpeg1_sequence_end(uint8_t out[4]);
void mpeg1_gop(uint8_t drop_frame, uint8_t hour, uint8_t minute, 
    uint8_t second, uint8_t num_pic, uint8_t closed, uint8_t broken, uint8_t* out);
void mpeg1_picture_header(uint16_t temporal_ref, uint8_t picture_type,
    uint16_t vbv_delay, uint8_t* bidir_vector, uint8_t* out);
void display_u8arr(uint8_t* buf, int32_t size);


#endif
