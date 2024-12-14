# Overview
This video encoder was designed as part of our final project for our Advanced Data Structures and Algorithms class (EC504) at Boston University.  It accepts a folder of JPEG images and encodes them into a simplified MPEG-1 file using functions we wrote ourselves.

## Group Members
| Names              | Emails                |
| :----------------- | --------------------- |
| Ellen Burhansjah   | eburhan@bu.edu        |
| Arthur Savage      | savageaf@bu.edu       |
| Olawale Akanji     | olawalea@bu.edu       |
| William Wang       | xwill@bu.edu          |

## Details on project directory

```bash
.
├── assests
│   ├── mpeg1-compliant-ds.png
│   └── mpeg1_encoder_decoder.png
├── bitstreams
│   ├── awesome_movie.mpeg
├── encoder
├── images
│   ├── Upload your images here
├── include
│   ├── bit_vector.h
│   ├── global_variables.h
│   ├── image_processing.h
│   ├── jpeg_handler.h
│   ├── mpeg1_blk.h
│   ├── mpeg1_enc.h
│   ├── mpeg1.h
│   ├── stb_image.h
│   └── vlc.h
├── main.c
├── Makefile
├── README.md
└── source
    ├── bit_vector.c
    ├── global_variables.c
    ├── image_processing.c
    ├── jpeg_handler.c
    ├── mpeg1_blk.c
    ├── mpeg1_enc.c
    ├── vlc.c


```

## Dependencies for core encoder
The following are the dependencies with which the project was created:
- make
- C (version 11+)
- gcc
- stb_image.h (which is already included here)

## Encoder and CLI will work on:
- Any 64-bit Linux system
- ARM MacOS

This project may function on other platforms, but has only been tested on the two specified above.  We also have an [Android application](https://github.com/Phosphorus15/ec504_AndroidCV/) integrated to this project.  It is runnable, and instructions for how to get the jar file for command line are available in its repository's README.

## How To
First, clone this repository:
```
git clone https://github.com/eburhansjah/ec504_ImageEncoder.git
```
Before compiling, create two folders named `images` and `bitstreams` in the root directory of this project and fill it with the images of your choice.  These images may be of any size, although **all images in this folder must be the same width and height**.  The encoder reads from the `images` folder and outputs all bitstreams into the `bitstreams` folder.

To compile and run the program, run the following commands:
```
make all    # build all targets
./encoder   # run the executable
```
If you need a clean compilation, simply call:
```
make clean  # remove all targets
```

### To pack as shared library

To work with either the CLI or android app in our ![project](https://github.com/Phosphorus15/ec504_AndroidCV/), or use the encoder as a standalone tool, you'll want to compile this into a dynamic library, you can do this by using:

```
export $JAVA_HOME=/Your/Java/Home 
# Set Java home, example for macOS /Library/Java/JavaVirtualMachines/openjdk-11.jdk/Contents/Home
# example for Linux /usr/lib/jvm/java-17-openjdk-amd64/
# Windows is not recommended but could still build
```

and then

```
make jni
```

Which will produce a `libencoder_jni.so` file that you could use to **put into** the Android/CLI codebase to make them fully functional. Or:

```
make sharedlib
```

If you only want an ordinary shared library that can be linked and use from your own code.

## MPEG-1 encoder and decoder framework

![mpeg1-encoder-decoder-framework](https://github.com/eburhansjah/ec504_ImageEncoder/blob/main/assests/mpeg1_encoder_decoder.png)

We are implementing **compression algorithms** and play the encoded file with an mpeg-1 player

**Compression steps include:**

1. Input check to ensure all images valid files of the same type and dimensions before converting to raw matrices;

2. Colorspace transform to convert the RGB image matrices to YCbCr colorspace;

3. Subsampling then compresses image from original dimensions to 4:2:0;

4. Breaks down images into 8x8 macroblocks;

5. Discrete Cosine Transform (DCT) on the macroblocks, which we sped up using a Fast Fourier Transform (FFT);

6. Quantization of the macroblocks to reduce the highest-frequency coefficients. The base quantization matrix used is the default intra matrix for MPEG-1. It is scalable according to a quality factor;
   
7. Scans the macroblocks in a zigzag pattern to collapse them to a 1D array;

8. Variable Length Encoding (VLC) on the 1D array according to the industry standard (ISO) VLC codes and headers. This framework includes both Run Length Encoding (RLE) and Huffman Encoding.

## MPEG-1 Compliant Datastructure

![mpeg-1-compliant-ds](https://github.com/eburhansjah/ec504_ImageEncoder/blob/main/assests/mpeg1-compliant-ds.png)

According to the ISO official CD 11172-2 "Coding of Moving Pictures and Associated Audio for Digital Storage Media at up to About 1.5 Mbit/s", MPEG-1 defines a variety of compression strategies and configurations that support a wide range of decoders.  In the context of this project, we implemented a limited but functional subset of these specifications, most of which are summarized here:

- We only use I-frames in this project, which means no motion vector prediction (P- or B- frames) or D-frames.
- No audio
- Fixed quantizing matrix (as opposed to scalable)
- Fixed parameters (frame rate, etc.)

## Encoder Output
This encoder will output `awesome_movie.mpeg` into your bitstream folder, which can then be played by most MPEG-1 compliant decoders. FFmpeg and Celluloid both work for playing the video, as does [PL_MPEG](https://github.com/phoboslab/pl_mpeg/tree/master), an open-source video decoder hosted by [Phoboslab](https://phoboslab.org/).

Also in your bitstream folder, you will see some individual bitstreams that represent the encoded information of each individual image.  Feel free to check them out if you wish, although they are not currently utilized within the MPEG video creation.

## Limitations
Currently, the output of this decoder, while it is 100% MPEG-1 compliant in its file structure, it **does not currently output visually coherent video**.  Most of what you will see is flashing pink blocks.  We believe this is due to lost information in the AC coefficients of the luma blocks, but we were unable to narrow down the cause before submission deadline.  More specific information is provided in our project report.
