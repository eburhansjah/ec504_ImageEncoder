#ifndef JPEG_HANDLER_H
#define JPEG_HANDLER_H

// #include "stb_image.h"

typedef struct{
    int width;
    int height;
    int channels;
    unsigned char *data;
} Image;

Image* read_jpeg(const char *filename);
int check_dimensions(Image *images[], int count);
void free_image(Image *img);

#endif