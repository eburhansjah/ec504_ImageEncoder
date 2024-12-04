
#include <stdlib.h>
#define BITVECTOR struct bitvector

BITVECTOR {
    char* value;
    long long int bits;
    long long int cursor;
    long long int cap;
};

void bitvector_init(BITVECTOR* bv, long long int size);

void bitvector_put_bit(BITVECTOR* bv, char bit);

void bitvector_put_binstring(BITVECTOR* bv, const char* bitstring);

void bitvecotr_put_byte_off(BITVECTOR* bv, char val, char bits, char offset);

void bitvecotr_put_byte(BITVECTOR* bv, char val, char bits);

void bitvecotr_put_byte_ent(BITVECTOR* bv, char val);

long long int bitvector_pos(BITVECTOR* bv, long long int off);

void bitvector_concat(BITVECTOR* dest, BITVECTOR* src);

int bitvector_toarray(BITVECTOR* bv, char* output);

BITVECTOR* bitvector_clone(BITVECTOR* bv);
