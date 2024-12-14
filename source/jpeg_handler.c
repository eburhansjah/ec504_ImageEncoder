#include <stdio.h>
#include <stdlib.h>

#include "jpeg_handler.h"
// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"

// Fn. that loads jpeg files with stb_image
Image* load_jpeg(const char *filename){
    Image *img = (Image *)malloc (sizeof(Image));
    img -> data = stbi_load(filename, &img->width, &img->height, &img->channels, 0);

    if(img->data == NULL){
        printf("Failed to load image: %s\n", filename);
        free(img);

        return NULL;
    }
    printf("Loaded JPEG: %s (%dx%d, %d channels)\n", filename, img->width, img->height, img->channels);
    
    return img;
}

// Fn. that frees allocated mem. for img
void free_image(Image *img){
    if(img){
        stbi_image_free(img->data);
        free(img);
    }
}