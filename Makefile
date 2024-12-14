# Compiler and flags
CC := gcc
CFLAGS := -g -I include -fPIC -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux -I$(JAVA_HOME)/include/darwin
LDFLAGS := -lm

# Source files
SRC := source/image_processing.c source/global_variables.c source/bit_vector.c source/mpeg1_enc.c source/vlc.c source/mpeg1_blk.c
JNI_SRC := encoder_jni.c
OBJ = $(SRC:.c=.o)
JNI_OBJ = $(JNI_SRC:.c=.o)
TARGET := encoder
SHARED_LIB := libencoder.so
JNI_LIB := libencoder_jni.so

# Directories
IMAGES_DIR = ./images
BITSTREAMS_DIR = ./bitstreams

# Default target
all: $(TARGET)

# Compile the executable
$(TARGET): main.o $(OBJ)
	$(CC) -o $(TARGET) main.o $(OBJ) $(LDFLAGS)

# Compile the shared library
sharedlib: $(OBJ)
	$(CC) -shared -o $(SHARED_LIB) $(OBJ) $(LDFLAGS)

# Compile the JNI-compatible shared library
jni: $(OBJ) $(JNI_OBJ)
	$(CC) -shared -o $(JNI_LIB) $(OBJ) $(JNI_OBJ) $(LDFLAGS)

# Compile .c to .o files
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

# Clean generated files
clean:
	rm -f $(OBJ) $(JNI_OBJ) $(TARGET) $(SHARED_LIB) $(JNI_LIB)
# rm -rf $(BITSTREAMS_DIR)/*

# Custom command to create bitstreams directory if it doesn’t exist
$(BITSTREAMS_DIR):
	mkdir -p $(BITSTREAMS_DIR)

.PHONY: all clean sharedlib jni
