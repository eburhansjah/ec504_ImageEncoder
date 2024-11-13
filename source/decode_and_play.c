#include "decode_and_play.h"
#include "image_processing.h"
#include <stdio.h>
#include <stdlib.h>
// #include <opencv2/opencv.hpp>
// #include <SDL2/SDL.h>

void decode_and_play(const char *bitstream_file, int width, int height, int img_count){
    // // Creating a window before playing images
    // cv::namedWindow("Video Playback", cv::WINDOW_AUTOSIZE);

    // if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    //     printf("Error initializing SDL2: %s\n", SDL_GetError());
    //     return;
    // }

    // SDL_Window *window = SDL_CreateWindow("Video Playback", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    // if (!window) {
    //     printf("Error creating window: %s\n", SDL_GetError());
    //     SDL_Quit();
    //     return;
    // }

    // SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    // if (!renderer) {
    //     printf("Error creating renderer: %s\n", SDL_GetError());
    //     SDL_DestroyWindow(window);
    //     SDL_Quit();
    //     return;
    // }

    FILE *file = fopen(bitstream_file, "rb");
    if (!file) {
        printf("Error: Failed to open bitstream file for reading.\n");
        return;
    }

    for (int img = 0; img < img_count; img++) {
        unsigned char *Y = malloc(width * height);
        unsigned char *Cb = malloc((width / 2) * (height / 2));
        unsigned char *Cr = malloc((width / 2) * (height / 2));

        unsigned char *RGB = malloc(width * height * 3);

        if (!Y || !Cb || !Cr || !RGB) {
            printf("Error: Failed to allocate memory.\n");
            fclose(file);

            return;
        }

        for (int y = 0; y < height; y += 16) {
            for (int x = 0; x < width; x += 16) {
                double Y_dct_blocks[4][8][8];
                double Cb_dct[8][8];
                double Cr_dct[8][8];

                fread(Y_dct_blocks, sizeof(double), 128, file);
                fread(Cb_dct, sizeof(double), 64, file);
                fread(Cr_dct, sizeof(double), 64, file);

                unsigned char Y_blocks[4][8][8];
                unsigned char Cb_block[8][8];
                unsigned char Cr_block[8][8];

                for (int block = 0; block < 4; block++) {
                    fast_IDCT(Y_dct_blocks[block], Y_blocks[block]);

                    int x_start_pos = x + (block % 2) * 8;
                    int y_start_pos = y + (block / 2) * 8;
                    insert_8x8_block(Y, width, x_start_pos, y_start_pos, Y_blocks[block]);
                }

                fast_IDCT(Cb_dct, Cb_block);
                fast_IDCT(Cr_dct, Cr_block);

                insert_8x8_block(Cb, width / 2, x / 2, y / 2, Cb_block);
                insert_8x8_block(Cr, width / 2, x / 2, y / 2, Cr_block);
            }
        }

        convert_ycbcr_to_rgb(Y, Cb, Cr, RGB, width, height);

        // // Converting RGB array into OpenCV Mat
        // cv::Mat frame(height, width, CV_8UC3, rgb);

        // // Display frames in OpenCV window
        // cv::imshow("Video Playback", frame);

        // // Wait for 33ms for frame display (30 FPS)
        // if (cv::waitKey(33) >= 0) {
        //     break;  // Exit if any key is pressed
        // }

        // // Create a texture for the RGB image
        // SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, width, height);
        // if (!texture) {
        //     printf("Error creating texture: %s\n", SDL_GetError());
        //     free(Y);
        //     free(Cb);
        //     free(Cr);
        //     free(RGB);
        //     fclose(file);
        //     SDL_DestroyRenderer(renderer);
        //     SDL_DestroyWindow(window);
        //     SDL_Quit();
        //     return;
        // }

        // SDL_UpdateTexture(texture, NULL, RGB, width * 3);

        // SDL_RenderClear(renderer);
        // SDL_RenderCopy(renderer, texture, NULL, NULL);
        // SDL_RenderPresent(renderer);

        // SDL_Delay(33);

        // SDL_DestroyTexture(texture);

        // // Save each frame as an image (frame_<img>.jpg)
        // char filename[256];
        // snprintf(filename, sizeof(filename), "./decoded_frames/frame_%d.jpg", img + 1);
        // stbi_write_jpg(filename, width, height, 3, rgb, 100);
        // printf("Decoded and saved frame %d to %s\n", img + 1, filename);

        free(Y);
        free(Cb);
        free(Cr);
        free(RGB);

        printf("Decoded and displayed frame %d\n", img + 1);
    }

    fclose(file);

    // cv::destroyWindow("Video Playback");

    // SDL_DestroyRenderer(renderer);
    // SDL_DestroyWindow(window);
    // SDL_Quit();

    printf("Finished decoding and playback.\n");
}