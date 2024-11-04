#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "image_processing.h"


int main() {
    const char *images_folder = "./images";
    const char *bitstream_folder = "./bitstreams";

    // Generate folder to store bitstreams if it does not exist
    struct stat st = {0};
    if (stat(bitstream_folder, &st) == -1) {
        mkdir(bitstream_folder, 0700);
        printf("Created directory for bitstreams: %s\n", bitstream_folder);
    }

    // Generate images folder if it does not exist
    if (stat(images_folder, &st) == -1) {
        mkdir(images_folder, 0700);
        printf("Created directory for images: %s\n", images_folder);
        printf("Please add your .jpg images in the '%s' folder and rerun the program.\n", images_folder);
        return 0;
    }

    struct dirent *entry;
    DIR *dir = opendir(images_folder);

    if (!dir) {
        printf("Error: Could not open images directory.\n");
        return -1;
    }

    Image **images = NULL; // Dynamic array
    int img_count = 0;
    int img_capacity = 100; // Initial capacity of images
    images = malloc(img_capacity * sizeof(Image *));

    if (!images) {
        printf("Error: Memory allocation failed for images array.\n");
        closedir(dir);
        return -1;
    }

    char filepath[256];

    // Loading images from directory
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".jpg") != NULL || strstr(entry->d_name, ".jpeg") != NULL) {
            if (img_count >= img_capacity) {
                img_capacity *= 2; // Increase capacity x2
                images = realloc(images, img_capacity * sizeof(Image *));
                if (!images) {
                    printf("Error: Memory reallocation failed for images array.\n");
                    closedir(dir);
                    return -1;
                }
            }

            snprintf(filepath, sizeof(filepath), "%s/%s", images_folder, entry->d_name);
            Image *img = (Image *)malloc(sizeof(Image));

            if(!img){
                printf("Error: could not allocate memory.\n");
                closedir(dir);
                free(images);
                return -1;
            }
            
            img->data = stbi_load(filepath, &img->width, &img->height, &img->channels, 0);
            if (!img->data) {
                printf("Error loading image %s: %s\n", filepath, stbi_failure_reason());
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

    printf("Image processing finished.\n");

    return 0;
}
