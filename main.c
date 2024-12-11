#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "image_processing.h"
#include "bit_vector.h"
#include "mpeg1.h"
#include "bit_vector.h"

int main() {
    struct vlc_macroblock v;               // initialize empty macroblock
    char out[24];                          // initialize empty buffer
    mpeg1_file_header(2202035, out);       // { multiplex_rate, buffer }
    //display_u8arr(out, 12);              // { buffer, sizeof(buffer) }
    mpeg1_sys_header(2202035, 0xe6, out);  // applies sys_header (no audio, 1 video stream, fixed ending, bound scale to 1024 bytes)
    //display_u8arr(out, 15);
    BITVECTOR* b = bitvector_new("", 8);   // initialize empty bitvector for final bitstream
    
    encode_macroblock_header(33, b);       // puts header onto bitstream
    bitvector_concat(b, encode_macblk_address_value(10));  // function to get the encoded bits for a given value
    bitvector_concat(b, encode_macblk_address_value(33));
    //bitvector_print(b); // no segfault

    printf("ready p\n");
    encode_blk_coeff(3, -127, 0); // { run, level, first }
    //bitvector_print();  
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

    //Loading images from directory
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
        free(images);
        return -1;
    }

    for (int i = 0; i < img_count; i++) {
        // Converting images from RGB to YCbCr and saving bitstreams
        unsigned char *Y, *Cb, *Cr;
        convert_rgb_to_ycbcr(images[i], &Y, &Cb, &Cr);

        // 4:2:0 subsampling on Cb and Cr channels
        unsigned char *Cb_sub, *Cr_sub;
        subsampling_420(Cb, Cr, images[i]->width, images[i]->height, &Cb_sub, &Cr_sub);

        // Extracting macroblocks and applying DCT over them
        // (4 for Y of 8x8 blocks, one Cb of 8x8 block, and one Cr of 8x8 block)
        // ref: https://stackoverflow.com/questions/8310749/discrete-cosine-transform-dct-implementation-c
        // Y
        for (int y = 0; y < images[i]->height; y += 16) {
            for (int x = 0; x < images[i]->width; x += 16) {
                // Dividing Y into 4 8x8 blocks
                unsigned char Y_blocks[4][8][8]; // Array that stores 4 8x8 blocks
                double Y_dct_blocks[4][8][8];
                int Y_quantized[4][8][8];
                int Y_zigzag[4][64];
                int Y_equalized[4][64];

                for (int block = 0; block < 4; block++) {
                    int x_start_pos = x + (block % 2) * 8;
                    int y_start_pos = y + (block / 2) * 8;

                    extract_8x8_block(Y, images[i]->width, x_start_pos, y_start_pos, Y_blocks[block]);

                    fast_DCT(Y_blocks[block], Y_dct_blocks[block]);
                    
                    /* // These coefficients are in the 0-400 range before quantization
                    // Printing transformed DCT coeffs. 
                    printf("Fast DCT on Y Block:\n");
                    for (int i = 0; i < 8; i++) {
                        for (int j = 0; j < 8; j++) {
                            printf("%8.2f ", Y_dct_blocks[0][i][j]);
                        }
                        printf("\n");
                    }
                    */
                    
                    // changes coefficients to range +-30
                    quantization(Y_dct_blocks[block], Y_quantized[block]);
                    // // Printing quantization result on Y block
                    /*
                    printf("Quantized coeffs. on Y Block:\n");
                    for (int i = 0; i < 8; i++) {
                        for (int j = 0; j < 8; j++) {
                            int block_num = 0; // only prints first of 4 blocks, change int if needed
                            printf("%4d ", Y_quantized[block_num][i][j]);
                        }
                        printf("\n");
                    }
                    */

                    zigzag_scanning(Y_quantized[block], Y_zigzag[block]);  // puts blocks in zigzag scan order
                    equalize_coefficients(Y_zigzag[block], Y_equalized[block]); // removes zeros by +-1
                    
                    /*
                    // Printing zigzag scanning result on Y block
                    printf("Equalized zigzag-scanned coeffs. on Y block:\n");
                    for (int i = 0; i < 64; i++) {
                        printf("%4d ", Y_equalized[0][i]);
                    }
                    printf("\n");
                    */
                    
                    // Creates 1D array of RLE-encoded coefficients of "tuples" (coeff, run_length) : [coeff1, RL1, coeff2, RL2, etc.]
                    int Y_encoded_array[128];
                    int *Y_RLE = run_length_encode(Y_equalized[block], Y_encoded_array);
                    
                    
                    // VLC encoding
                    int level_index = 0; 
                    int run_index = 1;   
                    int run, level, first;
                    BITVECTOR* temp_coeff_bv = bitvector_new("", 16);       // will hold individual encoded coefficients
                    BITVECTOR* temp_dest_bv = bitvector_new("", 8);      // vector that builds through concatenation
                    for (int RLE_index = 0; RLE_index < 64; RLE_index++) {  // iterate through entire 1x64 zigzag-scanned array
                        // determines whether the coefficient is the first one in the zigzag block, important for encoding.
                        if (RLE_index == 0) {
                            first = 1;
                        }
                        else {
                            first = 0;
                        }
                        //
                        level = Y_encoded_array[level_index];
                        run = Y_encoded_array[run_index];
                        
                        if (run == 0 || level == 0) { // accounts for zeros in RLE array, they have no relevant value
                            break;
                        }
                        
                        temp_coeff_bv = encode_blk_coeff(run, level, first); // encodes each coefficient into its individual BV
                        //printf("%d %d    ", RLE_index, temp_dest_bv->bits);
                        bitvector_concat(temp_dest_bv, temp_coeff_bv); 
                        run_index += 2;
                        level_index += 2;
                    }
                    
                    bitvector_concat(b, temp_dest_bv); // concatenate to BITSTREAM
                    //bitvector_print(temp_dest_bv);
                    //bitvector_print(b);
                    
                }

                // Diving Cb and Cr each into 1 8x8 block
                unsigned char Cb_block[8][8], Cr_block[8][8];
                double Cb_dct[8][8], Cr_dct[8][8];

                extract_8x8_block(Cb, images[i]->width / 2, x / 2, y / 2, Cb_block);
                extract_8x8_block(Cr, images[i]->width / 2, x / 2, y / 2, Cr_block);
            
                fast_DCT(Cb_block, Cb_dct);
                fast_DCT(Cr_block, Cr_dct);

                // printf("Fast DCT on Cb Block:\n");
                // for (int i = 0; i < 8; i++) {
                //     for (int j = 0; j < 8; j++) {
                //         printf("%8.2f ", Cb_dct[i][j]);
                //     }
                //     printf("\n");
                // }

                int Cb_quantized[8][8];
                int Cr_quantized[8][8];
                quantization(Cb_dct, Cb_quantized);
                quantization(Cr_dct, Cr_quantized);

                // printf("Quantized coeffs. on Cb Block:\n");
                // for (int i = 0; i < 8; i++) {
                //     for (int j = 0; j < 8; j++) {
                //         printf("%4d ", Cb_quantized[i][j]);
                //     }
                //     printf("\n");
                // }

                int Cb_zigzag[64];
                int Cr_zigzag[64];
                zigzag_scanning(Cb_quantized, Cb_zigzag);
                zigzag_scanning(Cr_quantized, Cr_zigzag);
                
                /*
                printf("Zigzag Scanned Coeffs. on Cb Block:\n");
                for (int i = 0; i < 64; i++) {
                    printf("%4d ", Cb_zigzag[i]);
                }
                */
                // printf("\n");

                int Cb_encoded_array[128], Cr_encoded_array[128];
                int *Cb_RLE = run_length_encode(Cb_zigzag, Cb_encoded_array);
                int *Cr_RLE = run_length_encode(Cr_zigzag, Cr_encoded_array);
            }
        }
        

        // Creating unique bitstream file names
        char bitstream_filename[256];
        snprintf(bitstream_filename, sizeof(bitstream_filename), "%s/image_%d.bit", bitstream_folder, i + 1);

        // Each bitstream for each file (e.g. image_1.bit) will contain: width, height, Y, Cb, Cr data by the pixel
        write_to_bitstream(bitstream_filename, Y, Cb, Cr, images[i]->width, images[i]->height);
        
        // Free components after processing
        free(Y);
        free(Cb);
        free(Cr);
        free(Cb_sub);
        free(Cr_sub);
    }

    // Free images
    for (int i = 0; i < img_count; i++) {
        stbi_image_free(images[i]->data);
        free(images[i]);
    }

    printf("Image processing finished.\n");

    return 0;
}
