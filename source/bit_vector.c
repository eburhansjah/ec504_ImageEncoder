#include "bit_vector.h"
#include <stdlib.h>

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

void bitvector_put_byte_off(BITVECTOR* bv, char val, char bits, char offset) {
    long long int boff = bv->cursor >> 3;  // Byte offset in the BITVECTOR
    int off = 7 - (bv->cursor & 0x7) + 1;  // Bit offset within the byte
    val &= (1u << (8 - offset)) - 1;       // Mask to use only the specified number of bits

    if (bv->cursor + bits >= bv->bits) bitvector_expand_size(bv, 0);

    if (off >= bits) {
        // If remaining bits in the current byte are sufficient to hold 'bits' bits of 'val'
        bv->value[boff] &= ~(((1 << bits) - 1) << (off - bits));
        bv->value[boff] |= (val & ((1 << bits) - 1)) << (off - bits);
        bv->cursor += bits; // Move the cursor forward by 'bits'
    } else {
        // If bits need to be split between this byte and the next one
        bv->value[boff] &= ~((1 << off) - 1); // Clear the bits at 'off'
        bv->value[boff] |= (val >> (bits - off)) & ((1 << off) - 1);
        bv->cursor += off; // Move the cursor forward to the next byte

        // Handle the remaining bits in the next byte
        bits -= off;
        boff++;
        bv->value[boff] &= ~((1 << bits) - 1); // Clear bits in the next byte
        bv->value[boff] |= val & ((1 << bits) - 1);
        bv->cursor += bits; // Advance the cursor by remaining bits
    }
    if (bv->cap < bv->cursor) bv->cap = bv->cursor;
}


void bitvecotr_put_byte(BITVECTOR* bv, char val, char bits) {
    bitvector_put_byte_off(bv, val, bits, 0);
}

void bitvecotr_put_byte_ent(BITVECTOR* bv, char val) {
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
    if (dest->bits - dest->cursor < src->cap) {
        bitvector_expand_size(dest, src->bits);
    }
    for (;i < src->cap >> 3; i ++) {
        bitvecotr_put_byte_ent(dest, src->value[i]);
    }
    if (src->cap & 0x7)
        bitvecotr_put_byte(dest, src->value[i], src->cap & 0x7);
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
    if (speculative != 0) {
        bv->value = realloc(bv->value, (bv->bits >> 3) + (speculative) + 1);
        bv->bits = bv->bits + speculative << 3;
    } else {
        // Double the size if no speculative guess
        bv->value = realloc(bv->value, (bv->bits >> 2) + 1);
        bv->bits = bv->bits << 1;
    }
}


BITVECTOR* bitvector_clone(BITVECTOR* bv) {
    BITVECTOR* nvec = (BITVECTOR *) malloc(sizeof(BITVECTOR));
    bitvector_init(nvec, bv->bits);
    nvec->cap = nvec->cursor = bv->cap; // Only the cursor is not cloned
    memcpy(nvec->value, bv->value, (bv->bits / 8) + (bv->bits % 8 ? 1 : 0));
    return nvec;
}