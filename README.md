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
├── bitstreams
│   └── images.bit
├── images
│   └── images.jpg
├── include
│   ├── global_variables.h
│   ├── image_processing.h
│   ├── jpeg_handler.h
│   └── stb_image.h
├── main.c
├── Makefile
├── README.md
└── source
    ├── global_variables.c
    ├── image_processing.c
    ├── jpeg_handler.c

```

## Dependencies
The following are the dependencies with which the project was created:
- make
- C (version 11+)
- gcc

This program was built and tested on the following operating systems:
- Linux Mint 21.3 (Virginia)
- MacOS

We will also be integrating this system into an Android application as part of our project.  This project may work on other platforms or with other package versions, but the above configurations are what we developed and tested upon. 

The runnable Android application and instructions on how to get the jar file for command line  is available in this repository:
https://github.com/Phosphorus15/ec504_AndroidCV/


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
