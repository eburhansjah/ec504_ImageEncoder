#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "image_processing.h"
#include "bit_vector.h"
#include "mpeg1.h"
#include "bit_vector.h"

// To display a buffer:
// display_u8arr(out, 12);              // { buffer, sizeof(buffer) }

int main() { 
    /* 
    CHANGING HEADER PARAMETERS
    - Here be dragons
    - Everything with a ?? I am insure how it works and/or what the value should be
    
    Headers occur in order:
    1-3. First three headers (file_header, sys_header, packet_header) initialized at beginning of program
    4. Sequence_header initialized after we know image size
    5. GOP headers every $frame_skip
    6. Picture header for every picture
    7. Slice header for every slice of that picture
    8. Macroblock header for every macroblock
    - Then, repeat 5-8 as needed until data is parsed
    */
    
    // GOP HEADER
    const int frame_skip = 1;  // how often to put a GOP frame header.  1 is before every picture, 2 is every other picture, etc.
    uint8_t aspect_ratio = 1;  // A: 1 F: 4 YBY : 3 (per William's notes)
    uint8_t frame_rate = 4;
    uint8_t yby_size = 3;
    uint8_t drop_frame = 0; // set to 1 only if picture_rate is 29.97 Hz.  What is picture_rate?  Who knows!
    uint8_t hour = 0, minute = 0, second = 0; // time values increment throughout video
    uint8_t num_pic = 0;    // [0-59] set by IEC standards for "time and control codes for video tape recorders", which costs money so we don't have it
    uint8_t closed = 1;     // keep set to 1 (means there are no motion vectors)
    uint8_t broken = 0;     // keep to 0 during encoding.  Only used for editing, does not apply for us.
    
    // PACKET HEADER
    int pts_optional = 0;                   // appears to be unused within packet_header function
    
    // SLICE HEADER
    uint8_t quant_scale = 1; // [1 to 31] scales reconstruction of DCT coeffs at slice/macroblock layer.  Zero forbidden.
    uint8_t vertical_pos = 0;  // Changes based on the height within the slice, starts at zero and maxes at 175 - DO NOT CHANGE VALUE HERE
    
    // PICTURE LAYER
    uint8_t temporal_ref = 0; // incremented by 1 mod 1024 for each input picture.  Reset to zero with each GOP.
    uint8_t picture_type = 1; // 001 for I-frame (and we only have I-frames)
    uint8_t* bidir_vector;    // ??
    
    // PICTURE LAYER 2.0 - calculating VBV delay, look at Page 37 of manual, this makes no sense
    uint8_t clock_speed = 1000000000; // set at 1 GHz
    uint8_t bitrate = 1; // ??
    uint8_t vbv_occupancy = 0; // measured before removing picture layer but after removing GOP layer (?????)
    uint8_t vbv_delay = clock_speed * vbv_occupancy / bitrate;    // prevents over/underflow of decoder buffer.
    
    // default locations for files
    const char *images_folder = "./images";        // load images from here
    const char *bitstream_folder = "./bitstreams"; // save bitstreams here
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Begin program!
    
    // opening bitstream movie file for writing
    FILE *fp;
    fp = fopen("./bitstreams/awesome_movie.mpeg", "wb");
    size_t filesize; // gets larger as program runs
    if (fp == NULL) {
        perror("Error opening mpeg file");
        return 1;
    }
    
    struct vlc_macroblock v;                // initialize empty macroblock
    char file_header[15], sys_header[12], packet_header[7], out[50];                           // initialize empty buffers
    
    // initializing headers 1 to 3                       num_bytes  info
    mpeg1_file_header(2202035, file_header);          // [15]       { multiplex_rate, buffer }
    filesize = fwrite(file_header, sizeof(file_header), sizeof(file_header)/sizeof(char), fp);
    mpeg1_sys_header(2202035, 0xe6, sys_header);      // [12]       { no audio, 1 video stream, fixed ending, bound scale to 1024 bytes}
    filesize = fwrite(sys_header, sizeof(sys_header), sizeof(sys_header)/sizeof(char), fp);
    mpeg1_packet_header(pts_optional, packet_header); // [7]        { useless_variable, buffer }
    filesize = fwrite(packet_header, sizeof(packet_header), sizeof(packet_header)/sizeof(char), fp);
    // sequence_header and onward are initialized further down after image info is processed
    
    
    // Willam's test code here, not implemented currently but do not delete (this is for macroblock/block level)
    /*
    //encode_macroblock_header(33, b);        // FUNCTION UNFINISHED
    bitvector_concat(b, encode_macblk_address_value(10));  // function to get the encoded bits for a given value
    bitvector_concat(b, encode_macblk_address_value(33));
    printf("ready p\n");
    bitvector_print(b);
    encode_blk_coeff(3, -127, 0); // { run, level, first }
    */

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
    
    // for sequence header
    uint8_t width = images[0]->width;
    uint8_t height = images[0]->height;
    const int num_slices = ceil(width / 8); // MAKE SURE IS NOT GREATER THAN 175
    
    // initialize sequence header
    char* sequence_header[12];
    mpeg1_sequence_header(width, height, aspect_ratio, frame_rate, yby_size, sequence_header); // *out may need to be uint8_t instead of char
    filesize = fwrite(sequence_header, sizeof(sequence_header), sizeof(sequence_header)/sizeof(char), fp);
    
    

    for (int i = 0; i < img_count; i++) {
     
        // puts GOP header every frame_skip number of images
        if ( i % frame_skip == 0) {
            char* gop_header[8];
            mpeg1_gop(drop_frame, hour, minute, second, num_pic, closed, broken, gop_header); // initialized first here, then every frame_skip number of images
            filesize = fwrite(gop_header, sizeof(gop_header), sizeof(gop_header)/sizeof(char), fp);
        }
        
        // Converting images from RGB to YCbCr and saving bitstreams
        unsigned char *Y, *Cb, *Cr;
        convert_rgb_to_ycbcr(images[i], &Y, &Cb, &Cr);

        // 4:2:0 subsampling on Cb and Cr channels
        unsigned char *Cb_sub, *Cr_sub;
        subsampling_420(Cb, Cr, images[i]->width, images[i]->height, &Cb_sub, &Cr_sub);
        
        // reset vertical position within slice for every image
        vertical_pos = 0; // necessary for slice headers

        int quality_factor = 12; // factor to scale Q MATIX for quantization
        char picture_header[9];
        mpeg1_picture_header(temporal_ref, picture_type, vbv_delay, bidir_vector, out);
        
        // Extracting macroblocks and applying DCT over them
        // (4 for Y of 8x8 blocks, one Cb of 8x8 block, and one Cr of 8x8 block)
        // ref: https://stackoverflow.com/questions/8310749/discrete-cosine-transform-dct-implementation-c
        // Y
        for (int y = 0; y < images[i]->height; y += 16) { // SLICE LEVEL
        
            //char slice_header[10];
            BITVECTOR* slice_header = bitvector_new("", 8);
            mpeg1_slice(quant_scale, vertical_pos++, slice_header); // adds slice header, updates vertical position

            for (int x = 0; x < images[i]->width; x += 16) { // MACROBLOCK LEVEL
                unsigned char Y_blocks[4][8][8]; // Array that stores 4 8x8 blocks
                double Y_dct_blocks[4][8][8];
                int Y_quantized[4][8][8];
                int Y_zigzag[4][64];
                int Y_equalized[4][64];

                for (int block = 0; block < 4; block++) { // BLOCK LEVEL
                
                    // macroblock and block header stuff
                    // add here
                    
                    // position within slice
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
                    quantization(Y_dct_blocks[block], Y_quantized[block], quality_factor);
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
                    
                    // encodes Y_array in VLC code and then converts it to a char array
                    BITVECTOR* temp_bv = bitvector_new("", 8);
                    VLC_encode(Y_encoded_array, temp_bv);                             // VLC encode
                    char Y_encoded_char_array[temp_bv->cap >> 3];
                    int Y_length = bitvector_toarray(temp_bv, Y_encoded_char_array);  // to char
                    
                    char* output = concat_char(slice_header, Y_encoded_char_array);
                    
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
                quantization(Cb_dct, Cb_quantized, quality_factor);
                quantization(Cr_dct, Cr_quantized, quality_factor);

                /*
                printf("Quantized coeffs. on Cb Block:\n");
                for (int i = 0; i < 8; i++) {
                    for (int j = 0; j < 8; j++) {
                        printf("%4d ", Cb_quantized[i][j]);
                    }
                    printf("\n");
                }
                */

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
    
    fclose(fp);
    printf("Image processing finished.\n");

    return 0;
}
