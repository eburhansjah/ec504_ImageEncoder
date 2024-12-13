#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <math.h>

#define PI 3.14159265358979323846

#include "image_processing.h"
#include "global_variables.h"
#include "bit_vector.h"
#include "mpeg1.h"

// The following is the default intra Q MATRIX for MPEG-1
// note: small quantization coeffs. retain more info from image
//       large quantization coeffs. retain less info from image
//       values are restricted to be ints 1 <= q[m,n] <= 255
const int Q_MATRIX[8][8] = {
    {8, 16, 19, 22, 26, 27, 29, 34},
    {16, 16, 22, 24, 27, 29, 34, 37},
    {19, 22, 26, 27, 29, 34, 34, 38},
    {22, 22, 26, 27, 29, 34, 37, 40},
    {22, 26, 27, 29, 32, 35, 40, 48},
    {26, 27, 29, 32, 35, 40, 48, 58},
    {26, 27, 29, 34, 38, 46, 56, 69},
    {27, 29, 35, 38, 46, 56, 69, 83}
};

const int ZIGZAG_ORDER[8][8] = {
     { 0,  1,  5,  6, 14, 15, 27, 28 },
     { 2,  4,  7, 13, 16, 26, 29, 42 },
     { 3,  8, 12, 17, 25, 30, 41, 43 },
     { 9, 11, 18, 24, 31, 40, 44, 53 },
     { 10, 19, 23, 32, 39, 45, 52, 54 },
     { 20, 22, 33, 38, 46, 51, 55, 60 },
     { 21, 34, 37, 47, 50, 56, 59, 61 },
     { 35, 36, 48, 49, 57, 58, 62, 63 }
};

// Constants for fast_DCT and fast_IDCT (implementation with Fast Fourier Transform)
static const int c1 = 1004; // cos(pi/16) << 10
static const int s1 = 200;  // sin(pi/16)
static const int c3 = 851;  // cos(3pi/16) << 10
static const int s3 = 569;  // sin(3pi/16)
static const int r2c6 = 554; // sqrt(2)*cos(6pi/16) << 10
static const int r2s6 = 1337; // sqrt(2)*sin(6pi/16) << 10
static const int r2 = 181;    // sqrt(2) << 7

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
        //      https://en.wikipedia.org/wiki/Y′UV
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
void extract_8x8_block(unsigned char *channel, int image_width, int start_x, int start_y, unsigned char block[8][8]) {
    // for (int row = 0; row < 8; row++) {
    //     int source_idx = (start_y + row) * image_width + start_x;

    //     // Copying 8 pixels per row; memcpy copies a block of mem. from one location to another 
    //     memcpy(&block[row * 8], &channel[source_idx], 8);
    // }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            block[i][j] = channel[(start_y + i) * image_width + (start_x + j)];
        }
    }
}

// 2 dimensional discrete cosine transform (2D DCT)
// Fn. that transforms images into the frequency domain (important features are extracted in small number of DCT coeffs.)
// DCT is applied to every block serially
// ref: https://towardsdatascience.com/image-compression-dct-method-f2bb79419587
//      https://stackoverflow.com/questions/8310749/discrete-cosine-transform-dct-implementation-c
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
                    sum += pixel * 
                            cos((2 * x + 1) * u * PI / (2.0 * N)) * 
                            cos((2 * y + 1) * v * PI / (2.0 * N));
                }
            }
            dct_block[v * N + u] = c_u * c_v * sum;
        }
    }

}

// Fn. that does 2D DCT with Fast Fourier Transform
// significantly reduces run time to n log n from n^4
// ref: https://unix4lyfe.org/dct/
//      It is based on the AAN algorithm:
//          Y. Arai, T. Agui, and M. Nakajima, “A fast DCT-SQ scheme for
//      images,” Transactions of the IEICE, vol. 71, no. 11, pp. 1095–
//      1097, 1988.
void fast_DCT(const unsigned char block[8][8], double dct_block[8][8]){
    const int N = 8;
    int rows[8][8];
    int x0, x1, x2, x3, x4, x5, x6, x7, x8;

    // Transforming rows
    for(int i = 0; i < N; i++){
        x0 = block[i][0];
        x1 = block[i][1];
        x2 = block[i][2];
        x3 = block[i][3];
        x4 = block[i][4];
        x5 = block[i][5];
        x6 = block[i][6];
        x7 = block[i][7];

        // Stage 1
        x8 = x7 + x0;
        x0 -= x7;
        x7 = x1 + x6;
        x1 -= x6;
        x6 = x2 + x5;
        x2 -= x5;
        x5 = x3 + x4;
        x3 -= x4;

        // Stage 2
        x4 = x8 + x5;
        x8 -= x5;
        x5 = x7 + x6;
        x7 -= x6;
        x6 = c1 * (x1 + x2);
        x2 = (-s1 - c1) * x2 + x6;
        x1 = (s1 - c1) * x1 + x6;
        x6 = c3 * (x0 + x3);
        x3 = (-s3 - c3) * x3 + x6;
        x0 = (s3 - c3) * x0 + x6;

        // Stage 3
        x6 = x4 + x5;
        x4 -= x5;
        x5 = r2c6 * (x7 + x8);
        x7 = (-r2s6 - r2c6) * x7 + x5;
        x8 = (r2s6 - r2c6) * x8 + x5;
        x5 = x0 + x2;
        x0 -= x2;
        x2 = x3 + x1;
        x3 -= x1;

        // Stage 4 and output
        rows[i][0] = x6;
        rows[i][4] = x4;
        rows[i][2] = x8 >> 10;
        rows[i][6] = x7 >> 10;
        rows[i][7] = (x2 - x5) >> 10;
        rows[i][1] = (x2 + x5) >> 10;
        rows[i][3] = (x3 * r2) >> 17;
        rows[i][5] = (x0 * r2) >> 17;
    }

    // Transforming columns
    for (int i = 0; i < 8; i++) {
        x0 = rows[0][i];
        x1 = rows[1][i];
        x2 = rows[2][i];
        x3 = rows[3][i];
        x4 = rows[4][i];
        x5 = rows[5][i];
        x6 = rows[6][i];
        x7 = rows[7][i];

        // Stage 1
        x8 = x7 + x0;
        x0 -= x7;
        x7 = x1 + x6;
        x1 -= x6;
        x6 = x2 + x5;
        x2 -= x5;
        x5 = x3 + x4;
        x3 -= x4;

        // Stage 2
        x4 = x8 + x5;
        x8 -= x5;
        x5 = x7 + x6;
        x7 -= x6;
        x6 = c1 * (x1 + x2);
        x2 = (-s1 - c1) * x2 + x6;
        x1 = (s1 - c1) * x1 + x6;
        x6 = c3 * (x0 + x3);
        x3 = (-s3 - c3) * x3 + x6;
        x0 = (s3 - c3) * x0 + x6;

        // Stage 3
        x6 = x4 + x5;
        x4 -= x5;
        x5 = r2c6 * (x7 + x8);
        x7 = (-r2s6 - r2c6) * x7 + x5;
        x8 = (r2s6 - r2c6) * x8 + x5;
        x5 = x0 + x2;
        x0 -= x2;
        x2 = x3 + x1;
        x3 -= x1;

        // Stage 4 and output
        dct_block[0][i] = (double)((x6 + 16) >> 3);
        dct_block[4][i] = (double)((x4 + 16) >> 3);
        dct_block[2][i] = (double)((x8 + 16384) >> 13);
        dct_block[6][i] = (double)((x7 + 16384) >> 13);
        dct_block[7][i] = (double)((x2 - x5 + 16384) >> 13);
        dct_block[1][i] = (double)((x2 + x5 + 16384) >> 13);
        dct_block[3][i] = (double)(((x3 >> 8) * r2 + 8192) >> 12);
        dct_block[5][i] = (double)(((x0 >> 8) * r2 + 8192) >> 12);
    }

}

// Fn. that scales Q_MATRIX according to quality factor [1, 100]
// quality factor = 1 indicates higest compression ratio, gives worst quality
// quality factor = 50 no change to base Q MATRIX
// quality factor = 100 indicates lowest compression ratio, gives the best quality
// ref: https://stackoverflow.com/questions/29215879/how-can-i-generalize-the-quantization-matrix-in-jpeg-compression
void scale_quantization_matrix(int scaled_q_matrix[8][8], int quality_factor){
    const int N = 8;

    // Ensuring that quality factor is [1, 100]
    if (quality_factor < 1){
        quality_factor = 1;
    }
    if (quality_factor > 100){
        quality_factor = 100;
    }

    float scaling_factor;
    if(quality_factor < 50){
        scaling_factor = 5000.0 / quality_factor;
    } else {
        scaling_factor = 200.0 - 2 * quality_factor;
    }

    for (int i=0; i<N; i++){
        for(int j=0; j<N; j++){
            scaled_q_matrix[i][j] = (int)round(Q_MATRIX[i][j] * scaling_factor / 100.0);

            // Ensure that there are no 0 entries to prevent division by zero during the 
            // quantization process
            if(scaled_q_matrix[i][j] < 1){
                scaled_q_matrix[i][j] = 1;
            }
        }
    }    
}

// Fn. that reduces DCT coeffs. by dividing them with the 8x8 quantization matrix
// goal would be to reduce high freq coeffs. more than low freq coeffs. according to quality factor
// This is the only time we introduce errors in the encoder decoder system
// ref: https://asecuritysite.com/comms/dct
void quantization(double dct_block[8][8], int quantized_block[8][8], int quality_factor){
    const int N = 8;
    int scaled_q_matrix[8][8];

    scale_quantization_matrix(scaled_q_matrix, quality_factor);

    /*
    printf("Scaled Q MATRIX: ");
    for(int i=0; i<N; i++){
        for(int j=0; j<N; j++){
            printf("%4d ", scaled_q_matrix[i][j]);
        }
        printf("\n");
    }
    */

    for (int i=0; i < N; i++){
        for (int j = 0; j < N; j++){
            quantized_block[i][j] = (int)(round(dct_block[i][j]) / scaled_q_matrix[i][j]);
        }
    }
}

// Fn. that orders quantized values into 1D array for RLE
void zigzag_scanning(int quantized_block[8][8], int zigzag_array[64]){
    const int N = 8;

    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            zigzag_array[ZIGZAG_ORDER[i][j]] = quantized_block[i][j];
        }
    }
}

// Removes zeros from quantized values of 1D zigzag array, enables successful VLC coding
// Adds 1 if coefficient is greater than zero, subtracts 1 if coefficient is negative
void equalize_coefficients(int zigzag_array[64], int equalized_array[64]) {
    const int N = 64;
    int coeff;
    for (int i = 0; i < N; i++){
        coeff = zigzag_array[i];
        // if (coeff >= 0) {
        //     coeff++;
        // }
        // else {
        //     coeff--;
        // }
        equalized_array[i] = coeff;
    }
}

void VLC_encode(int RLE_array[128], BITVECTOR* temp_dest_bv) {

    // VLC encoding
    int level_index = 0; 
    int run_index = 1;   
    int run, level, first;
    int trun = 0;
    BITVECTOR* temp_var;
    BITVECTOR* temp_coeff_bv = bitvector_new("", 16);       // will hold individual encoded coefficients
    for (int RLE_index = 0; RLE_index < 64; RLE_index++) {  // iterate through entire 1x64 zigzag-scanned array
        // determines whether the coefficient is the first one in the zigzag block, important for encoding.
        if (RLE_index == 0) {
            first = 0;
        }
        else {
            first = 0;
        }
        level = RLE_array[level_index];
        run = RLE_array[run_index];
        trun += run;
                        
        if (run == 0 || level == 0) { // accounts for zeros in RLE array, they have no relevant value
            break;
        }
                        
        temp_var = encode_blk_coeff(run, level, first); // encodes each coefficient into its individual BV
        // printf("Encoded pair %d %d\n", run, level);
        bitvector_concat(temp_dest_bv, temp_var); 
        run_index += 2;
        level_index += 2;
    }
    printf("Total run value: %d\n", trun);
}

// Fn. for dequantization (decoding)
// Quantized block are multipliedd by the Q matrix to get DCT values back before running Inverse DCT
// ref: https://ayushijani.github.io/Projectcontent.html
void dequantization(int quantized_block[8][8], double dct_block[8][8]){
    const int N = 8;

    for (int i=0; i < N; i++){
        for (int j = 0; j < N; j++){
            dct_block[i][j] = quantized_block[i][j] * Q_MATRIX[i][j];
        }
    }
}

// 2 dimensional discrete inverse cosine transform (2D IDCT)
// Fn. that converts freq. domain coeffs. back into spatial domain
// ref: https://www.imageeprocessing.com/2013/03/2-d-inverse-discrete-cosine-transform.html
//      https://unix4lyfe.org/dct/
void IDCT(const float dct_block[64], unsigned char block[64]){
    const int N = 8; // JPEG algorithm standard
    float c_u, c_v; // Normalization factors for frequency pair(u,v)
    float sum;

    // Computing x, y entries of IDCT
    for (int x = 0; x < N; x++) {
        for (int y = 0; y < N; y++) {
            sum = 0.0; // Storing weighted sum of cosines

            for (int u = 0; u < N; u++) {
                for (int v = 0; v < N; v++) {
                    // Computing normalization factors
                    c_u = (u == 0) ? sqrt(1.0 / N) : sqrt(2.0 / N);
                    c_v = (v == 0) ? sqrt(1.0 / N) : sqrt(2.0 / N);
                    
                    sum += c_u * c_v * dct_block[v * N + u] *
                           cos((2 * x + 1) * u * PI / (2.0 * N)) *
                           cos((2 * y + 1) * v * PI / (2.0 * N));
                }
            }

            // Limiting output to fit in the 8-bit unsigned char range of [0, 255]
            int pixel_value = (int)round(sum);
            block[y * N + x] = (unsigned char)(pixel_value < 0 ? 0 : (pixel_value > 255 ? 255 : pixel_value));
        }
    }

}

// Fn. that does 2D Inverse DCT with Fast Fourier Transform
// significantly reduces run time to n log n from n^4
// Process is similar to fast_DCT but in reverse, converting freq. domain (dct_block)
// back into spatial-domain (pixel values)
// ref: https://unix4lyfe.org/dct/
//      https://kibichomurage.medium.com/aan-dct-forward-and-inverse-7380e2fcbc32
//      It is based on the AAN algorithm:
//          Y. Arai, T. Agui, and M. Nakajima, “A fast DCT-SQ scheme for
//      images,” Transactions of the IEICE, vol. 71, no. 11, pp. 1095–
//      1097, 1988.
void fast_IDCT(const double dct_block[8][8], unsigned char block[8][8]){
    const int N = 8;
    int cols[8][8];
    int x0, x1, x2, x3, x4, x5, x6, x7, x8;

    // Transforming columns
    for (int i = 0; i < N; i++){
        x0 = dct_block[0][i];
        x1 = dct_block[1][i];
        x2 = dct_block[2][i];
        x3 = dct_block[3][i];
        x4 = dct_block[4][i];
        x5 = dct_block[5][i];
        x6 = dct_block[6][i];
        x7 = dct_block[7][i];

        // Stage 1
        x8 = x7 + x0;
        x0 -= x7;
        x7 = x1 + x6;
        x1 -= x6;
        x6 = x2 + x5;
        x2 -= x5;
        x5 = x3 + x4;
        x3 -= x4;

        // Stage 2
        x4 = x8 + x5;
        x8 -= x5;
        x5 = x7 + x6;
        x7 -= x6;
        x6 = c1 * (x1 + x2);
        x2 = (-s1 - c1) * x2 + x6;
        x1 = (s1 - c1) * x1 + x6;
        x6 = c3 * (x0 + x3);
        x3 = (-s3 - c3) * x3 + x6;
        x0 = (s3 - c3) * x0 + x6;

        // Stage 3
        x6 = x4 + x5;
        x4 -= x5;
        x5 = r2c6 * (x7 + x8);
        x7 = (-r2s6 - r2c6) * x7 + x5;
        x8 = (r2s6 - r2c6) * x8 + x5;
        x5 = x0 + x2;
        x0 -= x2;
        x2 = x3 + x1;
        x3 -= x1;

        // Stage 4 and output
        cols[0][i] = x6;
        cols[4][i] = x4;
        cols[2][i] = x8 >> 10;
        cols[6][i] = x7 >> 10;
        cols[7][i] = (x2 - x5) >> 10;
        cols[1][i] = (x2 + x5) >> 10;
        cols[3][i] = (x3 * r2) >> 17;
        cols[5][i] = (x0 * r2) >> 17;
    }

    // Transforming rows
    for (int i = 0; i < N; i++) {
        x0 = cols[i][0];
        x1 = cols[i][1];
        x2 = cols[i][2];
        x3 = cols[i][3];
        x4 = cols[i][4];
        x5 = cols[i][5];
        x6 = cols[i][6];
        x7 = cols[i][7];

        // Repeat the stages in reverse
        x8 = x7 + x0;
        x0 -= x7;
        x7 = x1 + x6;
        x1 -= x6;
        x6 = x2 + x5;
        x2 -= x5;
        x5 = x3 + x4;
        x3 -= x4;

        x4 = x8 + x5;
        x8 -= x5;
        x5 = x7 + x6;
        x7 -= x6;
        x6 = c1 * (x1 + x2);
        x2 = (-s1 - c1) * x2 + x6;
        x1 = (s1 - c1) * x1 + x6;
        x6 = c3 * (x0 + x3);
        x3 = (-s3 - c3) * x3 + x6;
        x0 = (s3 - c3) * x0 + x6;

        x6 = x4 + x5;
        x4 -= x5;
        x5 = r2c6 * (x7 + x8);
        x7 = (-r2s6 - r2c6) * x7 + x5;
        x8 = (r2s6 - r2c6) * x8 + x5;
        x5 = x0 + x2;
        x0 -= x2;
        x2 = x3 + x1;
        x3 -= x1;

        block[i][0] = (unsigned char)(x6 < 0 ? 0 : (x6 > 255 ? 255 : x6));
        block[i][4] = (unsigned char)(x4 < 0 ? 0 : (x4 > 255 ? 255 : x4));
        block[i][2] = (unsigned char)(x8 >> 10);
        block[i][6] = (unsigned char)(x7 >> 10);
        block[i][7] = (unsigned char)((x2 - x5) >> 10);
        block[i][1] = (unsigned char)((x2 + x5) >> 10);
        block[i][3] = (unsigned char)((x3 * r2) >> 17);
        block[i][5] = (unsigned char)((x0 * r2) >> 17);
    }
}

// Fn. that does the reversing of subsampling_420()
// Goal is to upsample each Cb and Cr value back to 2x2 block
void upsampling(unsigned char *Cb_sub, unsigned char *Cr_sub, int width, int height, unsigned char **Cb, unsigned char **Cr){
    int sub_width = width / 2;
    int sub_height = height / 2;

    *Cb = (unsigned char *)malloc(width * height);
    *Cr = (unsigned char *)malloc(width * height);

    // We are iterating over each subsampled pixel and expanding it to a 2x2 block
    for (int y = 0; y < sub_height; y ++) {
        for (int x = 0; x < sub_width; x++) {
            int idx_sub = y * sub_width + x;

            // Instead of replacing 2x2 block for both Cb and Cr with their average values,
            // We are filling in the 2x2 block, copying subsampled values
            // into their respective locations to achieve a full resolution
            int idx_up_1 = (2 * y) * width + (2 * x);
            int idx_up_2 = (2 * y) * width + (2 * x + 1);
            int idx_up_3 = (2 * y + 1) * width + (2 * x);
            int idx_up_4 = (2 * y + 1) * width + (2 * x + 1);

            (*Cb)[idx_up_1] = Cb_sub[idx_sub];
            (*Cb)[idx_up_2] = Cb_sub[idx_sub];
            (*Cb)[idx_up_3] = Cb_sub[idx_sub];
            (*Cb)[idx_up_4] = Cb_sub[idx_sub];

            (*Cr)[idx_up_1] = Cr_sub[idx_sub];
            (*Cr)[idx_up_2] = Cr_sub[idx_sub];
            (*Cr)[idx_up_3] = Cr_sub[idx_sub];
            (*Cr)[idx_up_4] = Cr_sub[idx_sub];
        }
    }
}

// Fn. that reverses extract_8x8_block function
void insert_8x8_block(unsigned char *channel, int image_width, int start_x, int start_y, unsigned char block[8][8]){
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            channel[(start_y + i) * image_width + (start_x + j)] = block[i][j];
        }
    }
}

// Fn. that converts YCbCr to RGB color space back
void convert_ycbcr_to_rgb(unsigned char *Y, unsigned char *Cb, unsigned char *Cr, Image *img){
    if(img->channels < 3){
        printf("Error: Image does not have correct color channels for YCbCr to RGB conversion.\n");
        return;
    }

    int width = img->width;
    int height = img->height;
    int total_pixels = width * height;

    // Allocating memory for RGB image
    img->data = (unsigned char *)malloc(total_pixels * img->channels);
    if (img->data == NULL) {
        printf("Error: Failed to allocate memory for RGB image.\n");
        
        return;
    }

    // Converting YCbCr to RGB
    for (int i = 0; i < total_pixels; i++) {
        // Current Y, Cb, Cr values
        unsigned char y = img->data[i];
        unsigned char cb = img->data[i];
        unsigned char cr = img->data[i];

        // Based on the ITU-R BT.601 (Rec. 601) standard:
        // ref: https://en.wikipedia.org/wiki/Y′UV
        // ref: https://stackoverflow.com/questions/58676546/i-m-confused-with-how-to-convert-rgb-to-ycrcb
        int r = (int)(y + 1.402 * (cr - 128));
        int g = (int)(y - 0.344136 * (cb - 128) - 0.714136 * (cr - 128));
        int b = (int)(y + 1.772 * (cb - 128));

        // Limit values to be in the range of [0, 255] to fit into 8-bit unsigned char
        r = (r < 0) ? 0 : (r > 255) ? 255 : r;
        g = (g < 0) ? 0 : (g > 255) ? 255 : g;
        b = (b < 0) ? 0 : (b > 255) ? 255 : b;

        int index = i * img->channels;
        img->data[index] = (unsigned char)r;
        img->data[index + 1] = (unsigned char)g;
        img->data[index + 2] = (unsigned char)b;
    }
    printf("Image converted from RGB to YCbCr.\n");
}

void print_array(int arr[], int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}


int* run_length_encode(int zigzag_block[64], int encoded_array[128]) { 
    
    int encoded_int = 0;
    int index = 0;
    int count = 0;
    
    encoded_int = zigzag_block[0];
    
    for (int i = 0; i < 64; i++) {
        
        //printf("encoded int: %d \n", zigzag_block[i]);
        //encoded_array[index++] = zigzag_block[i];
        
        if (/*zigzag_block[i] != zigzag_block[i+1] && zigzag_block[i-1] != zigzag_block[i+1]*/ zigzag_block[i] != 0) {
            encoded_array[index++] = zigzag_block[i];
            encoded_array[index++] = count;
            count = 0;
        }
        else {
            count ++;
            // encoded_array[index++] = zigzag_block[i];
            // while (zigzag_block[i] == zigzag_block[i+1] && i + 1 < 64) {
            //     count++;
            //     i++;
            //     if (count == 33) {  // accounts for maximum run of 33 in VLC coding
            //         break;
            //     }
            // }
            // encoded_array[index++] = count;
            // count = 1;
        }
        
    }

    
    
    index = index;
    // int *return_array = (int*)malloc(index * sizeof(int));
    // if (!return_array){
    //     return NULL;
    // }
    // for (int i = 0; i < index; i++) {
    //     return_array[i] = encoded_array[i];
    // }
    // encoded_array[index] = 127; encoded_array[index ++] = 0;
    encoded_array[index] = encoded_array[index ++] = 0;
    // print_array(return_array, index);
    return encoded_array;
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

    // Allocating a contiguous buffer for Y, Cb, Cr components and perform single fwrite call
    size_t total_size = (width * height * 3);
    unsigned char *buffer = (unsigned char*) malloc(total_size);

    if (!buffer){
        printf("Error: Failed to allocate memory for a buffer.\n");
        fclose(file);
        return;
    }

    // Filling in buffer after mem. allocation
    // Visualization of buffer:
    // | Y_data (width * height bytes) | Cb_data (width * height bytes) | Cr_data (width * height bytes) |
    memcpy(buffer, Y, width * height);
    memcpy(buffer + width * height, Cb, width * height);
    memcpy(buffer + (2 * width * height), Cr, width * height);

    fwrite(buffer, 1, total_size, file);

    free(buffer);
    fclose(file);
    printf("Bitstream written for %s.\n", filename);
}
