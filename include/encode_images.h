#ifndef ENCODE_IMAGES_H
#define ENCODE_IMAGES_H

#include "jpeg_handler.h"

int encode_images(Image **images, int img_count, const char *bitstream_file);

#endif // ENCODE_IMAGES_H