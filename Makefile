init:
	echo "Building MPEG-1 Video Encoder"

# Compiler and flags
CC := gcc
CFLAGS := -I include
LDFLAGS := -lm # math libary should be specified only during the linking stage

# Source files and output executable
SRC := main.c source/image_processing.c source/jpeg_handler.c source/global_variables.c
OBJ = $(SRC:.c=.o)
TARGET := encoder

# Directories
IMAGES_DIR = ./images
BITSTREAMS_DIR = ./bitstreams

# Default target is to compile everything
all: $(TARGET)

# Compile the executable
$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(LDFLAGS) 

# Compile .c to .o files
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Clean generated files
clean:
	rm -f $(OBJ) $(TARGET)
# rm -rf $(BITSTREAMS_DIR)/*

# Custom command to create bitstreams directory if it doesn’t exist
$(BITSTREAMS_DIR):
	mkdir -p $(BITSTREAMS_DIR)

.PHONY: all clean run
