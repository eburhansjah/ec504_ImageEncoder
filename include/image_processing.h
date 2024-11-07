#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include "jpeg_handler.h"


int check_dimensions(Image *images[], int count);
void convert_rgb_to_ycbcr(Image *img, unsigned char **Y, unsigned char **Cb, unsigned char **Cr); // RGB to YCbCr color space
void write_to_bitstream(const char *filename, unsigned char *Y, unsigned char *Cb, unsigned char *Cr, int width, int height);
void subsampling_420(unsigned char *Cb, unsigned char *Cr, int width, int height, unsigned char **Cb_sub, unsigned char **Cr_sub);
void extract_8x8_block(unsigned char *channel, int image_width, int start_x, int start_y, unsigned char block[64]);
void DCT(const unsigned char block[64], float dct_block[64]);
void quantization(float dct_block[64], int quantized_block[64]);
void zigzag_scanning(int quantized_block[64], int zigzag_block[64]);

#endif // IMAGE_PROCESSING_H