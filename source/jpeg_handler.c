#include <stdio.h>
#include <stdlib.h>

#include "jpeg_handler.h"
#include "stb_image.h"

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