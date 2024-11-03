# ec504_ImageEncoder
Design an efficient image encoder that accepts any amount of JPEG images and encodes them into an MPEG-1 compliant video file.

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
├── README.md
├── bitstreams
├── include
│   ├── image_processing.h
│   └── jpeg_handler.h
├── source
│   ├── image_processing.c
│   └── jpeg_handler.c
├── main.c
└── stb_image.h
```

## Dependencies
- make
- C (version 11+)
- gcc


## How To
First, clone this repository:
```
git clone https://github.com/eburhansjah/ec504_ImageEncoder.git
```
Then, in `main.c`, update the path to your directory of images.

The Makefile is currently not functional, so the way to compile this program is:
```
gcc -o encoder main.c source/image_processing.c source/jpeg_handler.c -I include -lm
```

