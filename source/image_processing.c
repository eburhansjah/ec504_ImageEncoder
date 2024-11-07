#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <math.h>

#define PI 3.14159265358979323846

#include "image_processing.h"

// Change if needed
// note: small quantization coeffs. retain more info from image
//       large quantization coeffs. retain less info from image
//       values are restricted to be ints 1 <= q[m,n] <= 255
const int Q_MATRIX[8][8] = {
    {16, 11, 10, 16, 24, 40, 51, 61},
    {12, 12, 14, 19, 26, 58, 60, 55},
    {14, 13, 16, 24, 40, 57, 69, 56},
    {14, 17, 22, 29, 51, 87, 80, 62},
    {18, 22, 37, 56, 68, 109, 103, 77},
    {24, 35, 55, 64, 81, 104, 113, 92},
    {49, 64, 78, 87, 103, 121, 120, 101},
    {72, 92, 95, 98, 112, 100, 103, 99}
};

const int ZIGZAG_ORDER[64] = {
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

int check_dimensions(Image *images[], int count) {
    if (count == 0){
        printf("No images found in directory.\n");
        return 0;
    }

    int width = images[0]->width;
    int height = images[0]->height;

    for (int i = 1; i < count; i++) {
        if (images[i]->width != width || images[i]->height != height) {
            printf("Error: Image dimensions do not match\n");
            return 0;
        }
    }

    printf("Images have matching dimensions of width = %d and height = %d\n", width, height);
    return 1;
}

void convert_rgb_to_ycbcr(Image *img, unsigned char **Y, unsigned char **Cb, unsigned char **Cr){
    if(img->channels < 3){
        printf("Error: Image does not have correct color channels for RBG to YCbCr conversion.\n");

        return;
    }

    int width = img->width;
    int height = img->height;
    int total_pixels = width * height;

    *Y = (unsigned char *)malloc(total_pixels);
    *Cb = (unsigned char *)malloc(total_pixels);
    *Cr = (unsigned char *)malloc(total_pixels);

    if (*Y == NULL || *Cb == NULL || *Cr == NULL) {
        printf("Error: memory allocation failed for YCbCr components.\n");
        free(*Y);
        free(*Cb);
        free(*Cr);

        return;
    }

    // Converting RGB to YCbCr
    for (int i = 0; i < total_pixels; i++) {
        int index = i * img->channels;
        unsigned char r = img->data[index];
        unsigned char g = img->data[index + 1];
        unsigned char b = img->data[index + 2];

        // Based on the ITU-R BT.601 (Rec. 601) standard:
        // YCbCr assumes that R, G, B are 8 bits unsigned ints in range [0, 255]
        // Range of Cb Cr has 128 added as an offset ensures that result is always positive (full range)
        // ref: https://stackoverflow.com/questions/58676546/i-m-confused-with-how-to-convert-rgb-to-ycrcb
        (*Y)[i]  = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
        (*Cb)[i] = (unsigned char)(128 - 0.168736 * r - 0.331264 * g + 0.5 * b);
        (*Cr)[i] = (unsigned char)(128 + 0.5 * r - 0.418688 * g - 0.081312 * b);
    }

    printf("Image converted from RGB to YCbCr.\n");
}

// Subsampling for Cb and Cr data
// Each 2 x 2 block of pixels in the original image will be replaced by the average value of Cb and Cr
void subsampling_420(unsigned char *Cb, unsigned char *Cr, int width, int height, unsigned char **Cb_sub, unsigned char **Cr_sub){
    int sub_width = width / 2;
    int sub_height = height / 2;

    *Cb_sub = (unsigned char *)malloc(sub_width * sub_height);
    *Cr_sub = (unsigned char *)malloc(sub_width * sub_height);

    for (int y = 0; y < height; y += 2) {
        for (int x = 0; x < width; x += 2) {
            int idx = (y / 2) * sub_width + (x / 2);

            // Replacing 2x2 block for both Cb and Cr with their average values
            (*Cb_sub)[idx] = (Cb[y * width + x] + Cb[y * width + x + 1] + 
                              Cb[(y + 1) * width + x] + Cb[(y + 1) * width + x + 1]) / 4;

            (*Cr_sub)[idx] = (Cr[y * width + x] + Cr[y * width + x + 1] + 
                              Cr[(y + 1) * width + x] + Cr[(y + 1) * width + x + 1]) / 4;
        }
    }
}

// Fn. that extracts 8x8 blocks from Y, Cb, and Cr component
// MPEG-1 operates on video in a picture resolution of 16x16, which includes subsampled blocks 
// (4 for Y of size 8x8, one Cb of 8x8, and one Cr of 8x8)
void extract_8x8_block(unsigned char *channel, int image_width, int start_x, int start_y, unsigned char block[64]) {
    for (int row = 0; row < 8; row++) {
        int source_idx = (start_y + row) * image_width + start_x;

        // Copying 8 pixels per row; memcpy copies a block of mem. from one location to another 
        memcpy(&block[row * 8], &channel[source_idx], 8);
    }
}

// 2D-dimensional discrete cosine transform (2D DCT)
// Fn. that transforms images into the frequency domain (important features are extracted in small number of DCT coeffs.)
// DCT is applied to every block serially
// ref: https://towardsdatascience.com/image-compression-dct-method-f2bb79419587
void DCT(const unsigned char block[64], float dct_block[64]){
    const int N = 8; // JPEG algorithm standard
    float c_u, c_v; // Normalization factors for frequency pair(u,v)
    float sum;

    // Computing u,vth entry of DCT of an image (freq.)
    for (int u = 0; u < N; u++) {
        for (int v = 0; v < N; v++) {
            sum = 0.0; // Storing weighted sum of cosines

            // Simplifying with the following vector representation
            c_u = (u == 0) ? sqrt(1.0 / N) : sqrt(2.0 / N);
            c_v = (v == 0) ? sqrt(1.0 / N) : sqrt(2.0 / N);

            for (int x = 0; x < N; x++) {
                for (int y = 0; y < N; y++) {
                    float pixel = (float)block[y * N + x];
                    sum += pixel * cos((2 * x + 1) * u * PI / (2.0 * N)) * cos((2 * y + 1) * v * PI / (2.0 * N));
                }
            }
            dct_block[v * N + u] = c_u * c_v * sum;
        }
    }

}

// Fn. that reduces DCT coeffs. by dividing them with the 8x8 quantization matrix
// goal would be to reduce high freq coeffs. more than low freq coeffs.
void quantization(float dct_block[64], int quantized_block[64]){
    const int N = 8;

    for (int i=0; i < N; i++){
        for (int j = 0; j < N; j++){
            int index = i * N + j;
            quantized_block[index] = (int)round(dct_block[index]) / Q_MATRIX[i][j];
        }
    }
}

// Fn. that orders quantized values into 1D array for RLE
void zigzag_scanning(int quantized_block[64], int zigzag_block[64]){
    for (int i = 0; i < 64; i++){
        zigzag_block[i] = quantized_block[ZIGZAG_ORDER[i]];
    }
}

void write_to_bitstream(const char *filename, unsigned char *Y, unsigned char *Cb, unsigned char *Cr, int width, int height){
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Could not open bitstream file.\n");
        return;
    }

    // metadata s.t. each bitstream file will have the following data:
    // width, height, Y, Cb, Cr data by the pixel
    fwrite(&width, sizeof(int), 1, file);
    fwrite(&height, sizeof(int), 1, file);

    fwrite(Y, 1, width * height, file); 
    fwrite(Cb, 1, width * height, file);
    fwrite(Cr, 1, width * height, file);

    fclose(file);
    printf("Bitstream written for %s.\n", filename);
}