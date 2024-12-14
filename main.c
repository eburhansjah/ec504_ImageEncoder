#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>


#define STB_IMAGE_IMPLEMENTATION
#include "encoder.h"

// To display a buffer:
// display_u8arr(out, 12);              // { buffer, sizeof(buffer) }

int main() { 
    return mpeg_encode_procedure("images/", "bitstreams", "bitstreams/awesome_video.mpeg", 12);
}
