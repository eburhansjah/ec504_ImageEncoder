#include "encode_images.h"
#include "image_processing.h"
#include <stdio.h>
#include <stdlib.h>

int encode_images(Image **images, int img_count, const char *bitstream_file){
    FILE *file = fopen(bitstream_file, "wb");
    if (!file){
        printf("Error: Failed to open bitstream file for writing.\n");
        perror("fopen");
        return 0;
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

                for (int block = 0; block < 4; block++) {
                    int x_start_pos = x + (block % 2) * 8;
                    int y_start_pos = y + (block / 2) * 8;

                    extract_8x8_block(Y, images[i]->width, x_start_pos, y_start_pos, Y_blocks[block]);

                    fast_DCT(Y_blocks[block], Y_dct_blocks[block]);
                    // // Printing transformed DCT coeffs. 
                    // printf("Fast DCT on Y Block:\n");
                    // for (int i = 0; i < 8; i++) {
                    //     for (int j = 0; j < 8; j++) {
                    //         printf("%8.2f ", Y_dct_blocks[0][i][j]);
                    //     }
                    //     printf("\n");
                    // }
                    if (fwrite(Y_dct_blocks[block], sizeof(double), 64, file) != 64) {
                        printf("Error writing Y_dct_blocks to file.\n");
                        fclose(file);

                        return 0;
                    }
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

                if (fwrite(Cb_dct, sizeof(double), 64, file) != 64 || fwrite(Cr_dct, sizeof(double), 64, file) != 64) {
                    printf("Error writing Cb/Cr blocks to file.\n");
                    fclose(file);

                    return 0;
                }

            }
        }
    }

    fclose(file);
    return 1;
}