#include <stdio.h>
#include <stdlib.h>

#include "image_processing.h"


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