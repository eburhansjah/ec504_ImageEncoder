#include "bit_vector.h"
#include <stdlib.h>
#include <stdio.h>


void bitvector_init(BITVECTOR* bv, long long int size) {
    bv->cap = bv->cursor = 0;
    bv->bits = size;
    bv->value = (char*) calloc((size >> 3) + 1, 1);
}

void bitvector_put_bit(BITVECTOR* bv, char bit) {
    long long int boff = bv->cursor >> 3;
    int off = bv->cursor & 0x7;

    if (bv->cursor + 1 >= bv->bits) bitvector_expand_size(bv, 0);

    if (bit) {
        bv->value[boff] |= (1 << (7 - off));
    } else {
        bv->value[boff] &= ~(1 << (7 - off));
    }

    bv->cursor ++;
    if (bv->cap < bv->cursor) bv->cap = bv->cursor;
}

void bitvector_put_binstring(BITVECTOR* bv, const char* bitstring) {
    const char* cursor = bitstring;
    for (;*cursor != '\0';) {
        if (*cursor == '1' || *cursor == 1) {
            bitvector_put_bit(bv, 1);
        } else {
            bitvector_put_bit(bv, 0);
        }
        cursor ++;
    }
    if (bv->cap < bv->cursor) bv->cap = bv->cursor;
}


#include <stdio.h>
void bitvector_put_byte_off(BITVECTOR *bv, unsigned char val, char bits, char offset) {
    // printf("cap %d val %d bits %d off %d\n", bv->cap, val, bits, offset);
    long long int boff = bv->cursor >> 3;    // Byte offset in the BITVECTOR
    int bit_offset = 7 - (bv->cursor & 0x7); // Bit offset within the current byte
    val = (val >> (8 - offset - bits)) & ((1 << bits) - 1); // Extract the relevant bits from 'val'

    // Ensure there is enough space to write the bits
    if (bv->cursor + bits > bv->bits) {
        bitvector_expand_size(bv, bv->cursor + bits - bv->bits);
    }

    while (bits > 0) {
        int bits_to_write = (bit_offset + 1 < bits) ? bit_offset + 1 : bits;

        // printf("value[%d] = %d\n", boff, (val >> (bits - bits_to_write)) << (bit_offset + 1 - bits_to_write));

        // Clear and set the bits in the current byte
        bv->value[boff] &= ~(((1 << bits_to_write) - 1) << (bit_offset + 1 - bits_to_write));
        // printf("Cleared %d\n", (unsigned char)~(((1 << bits_to_write) - 1) << (bit_offset + 1 - bits_to_write)));
        bv->value[boff] |= (val >> (bits - bits_to_write)) << (bit_offset + 1 - bits_to_write);
        // printf("Set %d %d %d\n", (unsigned char)(val >> (bits - bits_to_write)) << (bit_offset + 1 - bits_to_write), (unsigned char)((bits - bits_to_write)), (unsigned char)val);

        // Adjust counters and variables for the next iteration
        bits -= bits_to_write;

        // printf("bits write acc %d %d\n", bv->cursor, bits_to_write);
        bv->cursor += bits_to_write;
        if (bits > 0) {
            boff++;              // Move to the next byte
            bit_offset = 7;      // Reset bit offset to the MSB of the new byte
        }
    }

    // Update the capacity if the cursor has moved beyond it
    if (bv->cap < bv->cursor) {
        bv->cap = bv->cursor;
    }
}



void bitvector_put_byte(BITVECTOR* bv, char val, char bits) {
    bitvector_put_byte_off(bv, val, bits, 0);
}

void bitvector_put_byte_ent(BITVECTOR* bv, char val) {
    bitvector_put_byte_off(bv, val, 8, 0);
}

long long int bitvector_pos(BITVECTOR* bv, long long int off) {
    bv->cursor += off;
    if (bv->cap < bv->cursor) bv->cap = bv->cursor;
    return bv->cursor;
}

// concat `src` after the cursor of `dest`, with content as bits from [0, src.cap] in src
void bitvector_concat(BITVECTOR* dest, BITVECTOR* src) {
    int i = 0;

    //bitvector_print(dest);

    if (dest->bits - dest->cursor < src->cap) {
        bitvector_expand_size(dest, src->bits);
    }

    if (dest->cursor % 8 == 0) {
        // enable shortcut mode if the dest vector is padded to 8, this copy should be much faster
        memcpy(dest->value + (dest->cursor / 8), src->value, (src->cap / 8) + (src->cap % 8 ? 1 : 0));
        dest->cursor += src->cap;
        if (dest->cap < dest->cursor) dest->cap = dest->cursor;
        return;
    }

    for (;i < src->cap >> 3; i ++) {
        bitvector_put_byte_ent(dest, src->value[i]);
    }
    if (src->cap & 0x7)
        bitvector_put_byte(dest, src->value[i], src->cap & 0x7);
}

int bitvector_toarray(BITVECTOR* bv, char* output) {
    int total_bytes = bv->cap >> 3;
    int bnum = 0;
    memcpy(output, bv->value, total_bytes);
    if (bv->cap & 0x7) {
        bnum = bv->cap & 0x7;
        output[total_bytes] = bv->value[total_bytes];
        output[total_bytes++] &= ~((1 << bnum) - 1);
    }
    return total_bytes;
}

void bitvector_expand_size(BITVECTOR* bv, long long int speculative) {
    int * new_pointer;
    if (speculative != 0) {
        if (new_pointer = realloc(bv->value, (bv->bits >> 3) + (speculative) + 1)) {
            bv->value = new_pointer;
        }
        else {
            printf("REALLOC FAILED");
        }
        bv->bits = bv->bits + speculative << 3;
    } 
    else {
        // Double the size if no speculative guess
        if (new_pointer = realloc(bv->value, (bv->bits >> 2) + 1)) {
            bv->value = new_pointer;
            bv->bits = bv->bits << 1;
        }
        else {
            printf("REALLOC FAILED");
        }
    }
}


BITVECTOR* bitvector_clone(BITVECTOR* bv) {
    BITVECTOR* nvec = (BITVECTOR *) malloc(sizeof(BITVECTOR));
    bitvector_init(nvec, bv->bits);
    nvec->cap = nvec->cursor = bv->cap; // Only the cursor is not cloned
    memcpy(nvec->value, bv->value, (bv->bits / 8) + (bv->bits % 8 ? 1 : 0));
    return nvec;
}

BITVECTOR* bitvector_new(const char* binstring, long long int size) {
    BITVECTOR* nvec = (BITVECTOR *) malloc(sizeof(BITVECTOR));
    bitvector_init(nvec, size);
    bitvector_put_binstring(nvec, binstring);
    return nvec;
}

void bitvector_print(BITVECTOR *bv) {
    for (int i = 0; i < bv->cap; i++) {
        int byte_index = i / 8;          // Find the byte index in bv->value
        int bit_offset = 7 - (i % 8);   // Calculate the bit offset (most significant bit first)
        int bit = (bv->value[byte_index] >> bit_offset) & 1;
        putchar(bit + '0');             // Convert the bit to a character ('0' or '1')
    }
    putchar('\n');
}
