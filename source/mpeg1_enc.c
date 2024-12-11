#include "mpeg1.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
// http://andrewduncan.net/mpeg/mpeg-1.html
// One of these
void mpeg1_file_header(uint32_t multiplex_rate, uint8_t out[12]) {
    memcpy(out, "\x00\x00\x01\xba", 4);
    out[4] = 0x21; // clock reference not set
    out[5] = 0x00; 
    out[6] = 0x01; // intended time not set
    out[7] = 0x00;
    out[8] = 0x01; // not set
    multiplex_rate = (multiplex_rate & 0x3fffff) | 0x400000; // 22 bits used
    multiplex_rate <<= 1;
    multiplex_rate |= 1;
    uint8_t *p = &(multiplex_rate);
    out[9] = p[2];
    out[10] = p[1];
    out[11] = p[0];
}

// 1
void mpeg1_sys_header(uint32_t multiplex_rate, uint8_t packet_num, uint8_t out[15]) {
    memcpy(out, "\x00\x00\x01\xbb", 4);
    out[4] = 0x00;
    out[5] = 0x09; // 9 bytes fixed for 1 video stream
    multiplex_rate = (multiplex_rate & 0x3fffff) | 0x400000; // 22 bits used
    multiplex_rate <<= 1;
    multiplex_rate |= 1;
    uint8_t *p = &(multiplex_rate);
    out[6] = p[2];
    out[7] = p[1];
    out[8] = p[0];
    out[9] = 0x00; // no audio and variable rate
    out[10] = 0x21; // no audio and only 1 video stream
    out[11] = 0xff; // fixed ending
    /*
     Emit the first stream ID
    */
    out[12] = 0xE0; // 0b11100000 - video stream 0
    out[13] = 0xE0; // 0b11100000 - bound scale to 1024 bytes
    out[14] = packet_num;
}

// 1
void mpeg1_packet_header(uint32_t pts_optinal ,uint8_t *out) {
    *(out++) = 0x00;
    *(out++) = 0x00;
    *(out++) = 0x01;
    *(out++) = 0xe0; // stream id 0

    uint8_t* length_writer = out;
    out += 2;

    *(out++) = 0x0f; // when pts is not applicable
    length_writer[1] = 0x00;
    length_writer[1] = 0x01;

}

// A: 1 F: 4 YBY : 3
// Only one video sequence header, only need once - this is where video begins
void mpeg1_sequence_header(uint16_t width, uint16_t height, uint8_t aspect_ratio, uint8_t frame_rate, uint8_t yby_size, uint8_t* out) {
    *(out++) = 0x00;
    *(out++) = 0x00;
    *(out++) = 0x01;
    *(out++) = 0xb3; // sequence header
    *(out++) = (width & 0xff0) >> 4;
    *(out++) = (width & 0xf) << 4 | (height & 0xf00) >> 8;
    *(out++) = (height & 0x0ff);
    *(out++) = (aspect_ratio & 0xf) << 4 | frame_rate & 0xf;
    *(out++) = 0xFF;
    *(out++) = 0xFF; // flexible bitrate
    *(out++) = 0xE0 ;//| (yby_size & 0x);
    *(out++) = (yby_size & 0x1f) << 3;
}

void mpeg1_sequence_end(uint8_t out[4]) {
    memcpy(out, "\x00\x00\x01\xb7", 4);
}

// A: 1 F: 4 YBY : 3
// each video sequence is many GOP
// may need more than one of these headers - NEED TO TEST THIS, FIGURE OUT HOW MANY GOP TO USE
void mpeg1_gop(uint8_t drop_frame, uint8_t hour, uint8_t minute, 
    uint8_t second, uint8_t num_pic, uint8_t closed, uint8_t broken, uint8_t* out) {
    *(out++) = 0x00;
    *(out++) = 0x00;
    *(out++) = 0x01;
    *(out++) = 0xb8; // sequence header
    *(out++) = (drop_frame << 7) | (hour & 0x1f) << 2 | (minute & 0x30) >> 4;
    *(out++) = (minute & 0xf) << 4 | 0x8 | (second & 0x38) >> 3;
    *(out++) = (second & 0x7) << 5 | (num_pic & 0xfc) >> 1;
    *(out++) = (num_pic & 1) << 7 | (closed & 1) << 6 | (broken & 1) << 5;
}

/*
I - 001
P - 010
B - 011
*/
void mpeg1_picture_header(uint16_t temporal_ref, uint8_t picture_type,
    uint16_t vbv_delay, uint8_t* bidir_vector, uint8_t* out) {
    *(out++) = 0x00;
    *(out++) = 0x00;
    *(out++) = 0x01;
    *(out++) = 0x00; // sequence header
    *(out++) = (temporal_ref & 0x3fc) >> 2;
    *(out++) = (temporal_ref & 0x3) << 6 | (picture_type & 0x7) << 3 | (vbv_delay & 0xe000) >> 13;
    *(out++) = (vbv_delay & 0x1fe0) >> 5;
    *(out++) = (vbv_delay & 0x1f) << 3;
    if (picture_type == 2 || picture_type == 3) {
        *(out - 1) |= (bidir_vector[0] & 1) << 2 | (bidir_vector[1] & 6) >> 1;
        *(out ++) = (bidir_vector[1] & 1) << 7;
    }
    if (picture_type == 3) {
        *(out - 1) |= (bidir_vector[2] & 1) << 6 | (bidir_vector[3] & 7) << 3;
    }
}

void mpeg1_slice(uint8_t quant_scale, uint8_t vertical_pos /* <= 175 */, uint8_t* out) {
    *(out++) = 0x00;
    *(out++) = 0x00;
    *(out++) = 0x01;
    *(out++) = vertical_pos; // slice header, cannot be greater than (8*175)
    *(out) = (quant_scale & 0x1f) << 3; 
    // special process, slice contain packed macroblocks
}

void display_u8arr(uint8_t* buf, int32_t size) {
    for(int i = 0; i < size; i ++)
        printf("0x%02x ", buf[i]);
    printf("\n");
}
