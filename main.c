#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "image_processing.h"


int main() {
    const char *folder = "PATH_TO_IMAGES_FOLDER";
    const char *bitstream_folder = "bitstreams";

    // Generate folder to store bitstreams if it does not exist
    struct stat st = {0};
    if (stat(bitstream_folder, &st) == -1) {
        mkdir(bitstream_folder, 0700);
        printf("Created directory: %s\n", bitstream_folder);
    }

    struct dirent *entry;
    DIR *dir = opendir(folder);

    if (!dir) {
        printf("Error: Could not open images directory.\n");
        return -1;
    }

    Image *images[100]; // assume we have 100 images
    int img_count = 0;
    char filepath[256];

    // Loading images from directory
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".jpg") != NULL || strstr(entry->d_name, ".jpeg") != NULL) {
            snprintf(filepath, sizeof(filepath), "%s/%s", folder, entry->d_name);
            Image *img = (Image *)malloc(sizeof(Image));

            if(!img){
                printf("Error: could not allocate memory.\n");
                closedir(dir);
                return -1;
            }
            
            img->data = stbi_load(filepath, &img->width, &img->height, &img->channels, 0);
            if (!img->data) {
                printf("Error loading image %s\n", filepath);
                free(img);
                continue;
            }
            images[img_count++] = img;
            printf("Loaded image: %s (Width: %d, Height: %d)\n", entry->d_name, img->width, img->height);
        }
    }
    closedir(dir);

    // Checking if file has images and that their dimensions match
    if (!check_dimensions(images, img_count)) {
        printf("Image dimensions do not match.\n");
        for (int i = 0; i < img_count; i++) {
            stbi_image_free(images[i]->data);
            free(images[i]);
        }
        return -1;
    }

    // Converting images from RGB to YCbCr and saving bitstreams
    for (int i = 0; i < img_count; i++) {
        unsigned char *Y, *Cb, *Cr;
        convert_rgb_to_ycbcr(images[i], &Y, &Cb, &Cr);
        
        // Creating unique bitstream file names
        char bitstream_filename[256];
        snprintf(bitstream_filename, sizeof(bitstream_filename), "%s/image_%d.bit", bitstream_folder, i + 1);

        // Each bitstream for each file (e.g. image_1.bit) will contain: width, height, Y, Cb, Cr data by the pixel
        write_to_bitstream(bitstream_filename, Y, Cb, Cr, images[i]->width, images[i]->height);
        
        // Free YCbCr components after processing
        free(Y);
        free(Cb);
        free(Cr);
    }

    // Free images
    for (int i = 0; i < img_count; i++) {
        stbi_image_free(images[i]->data);
        free(images[i]);
    }

    print("Image processing finished.\n");

    return 0;
}
