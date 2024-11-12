# ec504_ImageEncoder
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

## How it works

Encoder runtime steps:
1) Converts RGB images to YCbCr
2) Subsamples to 4:2:1
3) Breaks down images into 8x8 macroblocks
4) Performs a Discrete Cosine Transform (DCT) on them with a Fast Fourier Transform (FFT)
5) Quantizes the macroblocks
6) Scans the macroblocks in a zigzag pattern to collapse them to a 1D array
7) Performs Run Length Encoding (RLE) to compress the sparse matrices
8) Huffman encoding and table generation **(In-Progress)**
