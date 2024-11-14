#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include "jpeg_handler.h"


int check_dimensions(Image *images[], int count);
void convert_rgb_to_ycbcr(Image *img, unsigned char **Y, unsigned char **Cb, unsigned char **Cr); // RGB to YCbCr color space
void write_to_bitstream(const char *filename, unsigned char *Y, unsigned char *Cb, unsigned char *Cr, int width, int height);
void subsampling_420(unsigned char *Cb, unsigned char *Cr, int width, int height, unsigned char **Cb_sub, unsigned char **Cr_sub);
void extract_8x8_block(unsigned char *channel, int image_width, int start_x, int start_y, unsigned char block[8][8]);

void DCT(const unsigned char block[64], float dct_block[64]);
void fast_DCT(const unsigned char block[8][8], double dct_block[8][8]);
void quantization(double dct_block[8][8], int quantized_block[8][8]);
void zigzag_scanning(int quantized_block[8][8], int zigzag_array[64]);
int* run_length_encode(int array[64], int encode_array[128]);
void dequantization(int quantized_block[8][8], double dct_block[8][8]);
void IDCT(const float dct_block[64], unsigned char block[64]);
void fast_IDCT(const double dct_block[8][8], unsigned char block[8][8]);

void upsampling(unsigned char *Cb_sub, unsigned char *Cr_sub, int width, int height, unsigned char **Cb, unsigned char **Cr);
void insert_8x8_block(unsigned char *channel, int image_width, int start_x, int start_y, unsigned char block[8][8]);
void convert_ycbcr_to_rgb(unsigned char *Y, unsigned char *Cb, unsigned char *Cr, Image *img);

void print_array(int array[], int size);

#endif // IMAGE_PROCESSING_H