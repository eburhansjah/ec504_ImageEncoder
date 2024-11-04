#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include "jpeg_handler.h"


int check_dimensions(Image *images[], int count);
void convert_rgb_to_ycbcr(Image *img, unsigned char **Y, unsigned char **Cb, unsigned char **Cr); // RGB to YCbCr color space
void write_to_bitstream(const char *filename, unsigned char *Y, unsigned char *Cb, unsigned char *Cr, int width, int height);
void subsampling_420(unsigned char *Cb, unsigned char *Cr, int width, int height, unsigned char **Cb_sub, unsigned char **Cr_sub);

#endif // IMAGE_PROCESSING_H