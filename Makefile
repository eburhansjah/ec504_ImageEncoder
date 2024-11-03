init:
	echo "Building MPEG-1 Video Encoder"

CC := gcc
CFLAGS := -I include -lm
SRC := main.c source/image_processing.c source/jpeg_handler.c
TARGET := encoder

$(TARGET): $(SRC)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: all
all:
	$(TARGET)

.PHONY: clean
clean:
	rm -f $(TARGET) 
