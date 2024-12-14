
#include <stdlib.h>
#ifndef GUARD_BITVECTOR
#define GUARD_BITVECTOR 1
#define BITVECTOR struct bitvector

#include <stdio.h>

BITVECTOR {
    char* value;
    long long int bits;
    long long int cursor;
    long long int cap;
};

void bitvector_init(BITVECTOR* bv, long long int size);

BITVECTOR* bitvector_new(const char* binstring, long long int size);

void bitvector_put_bit(BITVECTOR* bv, char bit);

void bitvector_put_binstring(BITVECTOR* bv, const char* bitstring);

void bitvector_put_byte_off(BITVECTOR* bv, unsigned char val, char bits, char offset);

void bitvector_put_byte(BITVECTOR* bv, char val, char bits);

void bitvector_put_byte_ent(BITVECTOR* bv, char val);

long long int bitvector_pos(BITVECTOR* bv, long long int off);

void bitvector_concat(BITVECTOR* dest, BITVECTOR* src);

int bitvector_toarray(BITVECTOR* bv, char* output);

BITVECTOR* bitvector_clone(BITVECTOR* bv);

void bitvector_print(BITVECTOR* bv);

int bitvector_fwrite(BITVECTOR* bv, FILE* file);

void bitvector_expand_size(BITVECTOR* bv, long long int speculative);

#endif