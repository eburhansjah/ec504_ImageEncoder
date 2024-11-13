#include "mpeg1.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
// http://andrewduncan.net/mpeg/mpeg-1.html
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

// A: 1 F: 4 YBY : 3
void mpeg1_gop(uint16_t width, uint16_t height, uint8_t aspect_ratio, uint8_t frame_rate, uint8_t yby_size, uint8_t* out) {
    *(out++) = 0x00;
    *(out++) = 0x00;
    *(out++) = 0x01;
    *(out++) = 0xb8; // sequence header
    *(out++) = (width & 0xff0) >> 4;
    *(out++) = (width & 0xf) << 4 | (height & 0xf00) >> 8;
    *(out++) = (height & 0x0ff);
    *(out++) = (aspect_ratio & 0xf) << 4 | frame_rate & 0xf;
    *(out++) = 0xFF;
    *(out++) = 0xFF; // flexible bitrate
    *(out++) = 0xE0 ;//| (yby_size & 0x);
    *(out++) = (yby_size & 0x1f) << 3;
}

void display_u8arr(uint8_t* buf, int32_t size) {
    for(int i = 0; i < size; i ++)
        printf("0x%02x ", buf[i]);
    printf("\n");
}
