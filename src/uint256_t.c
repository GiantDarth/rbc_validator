//
// Created by cp723 on 3/8/2019.
//

#include <stddef.h>
#include <string.h>
#include <x86intrin.h>
#include <stdio.h>
#include "uint256_t.h"

void uint256_set_ui(uint256_t *rop, unsigned long long value) {
    rop->limbs[0] = value;
    memset(&(rop->limbs[1]), 0, 3);
}

void uint256_and(uint256_t *rop, const uint256_t *op1, const uint256_t *op2) {
    for(int i = 0; i < 4; ++i) {
        rop->limbs[i] = op1->limbs[i] & op2->limbs[i];
    }
}

void uint256_ior(uint256_t *rop, const uint256_t *op1, const uint256_t *op2) {
    for(int i = 0; i < 4; ++i) {
        rop->limbs[i] = op1->limbs[i] | op2->limbs[i];
    }
}

void uint256_xor(uint256_t *rop, const uint256_t *op1, const uint256_t *op2) {
    for(int i = 0; i < 4; ++i) {
        rop->limbs[i] = op1->limbs[i] ^ op2->limbs[i];
    }
}

void uint256_com(uint256_t *rop, const uint256_t *op1) {
    for(int i = 0; i < 4; ++i) {
        rop->limbs[i] = ~(op1->limbs[i]);
    }
}

void uint256_neg(uint256_t *rop, const uint256_t *op1) {
    // First get the bitwise complement of op.
    uint256_com(rop, op1);
    uint256_add(rop, rop, &UINT256_ONE);
}

void uint256_shift_right(uint256_t *rop, const uint256_t* op1, int shift) {
    // How many limb shifts to perform
    int limb_shifts = shift / 64;
    // Copy the words by a gap of "limb_shifts" limbs
    for(int i = limb_shifts; i < 4; ++i) {
        rop->limbs[i - limb_shifts] = op1->limbs[i];
    }

    // Zero out the leading limbs
    for(int i = 4 - limb_shifts; i < 4; ++i) {
        rop->limbs[i] = 0;
    }

    // Make sure remaining shift is within range of 64-bits
    shift %= 64;
    // Start from least-significant limb to second-to-most
    for(int i = 0; i < 3; ++i) {
        // Right shift the current limb, then attach the next limb's trailing bits to this one's
        // leading bits
        rop->limbs[i] = (rop->limbs[i] >> shift) | (rop->limbs[i + 1] << (64 - shift));
    }
    // Only right shift the last limb
    rop->limbs[3] >>= shift;
}

void uint256_shift_left(uint256_t *rop, const uint256_t* op1, int shift) {
    // How many limb shifts to perform
    int limb_shifts = shift / 64;
    // Copy the words by a gap of "limb_shifts" limbs
    for(int i = 0; i < 4 - limb_shifts; ++i) {
        rop->limbs[i + limb_shifts] = op1->limbs[i];
    }

    // Zero out the trailing limbs
    for(int i = 0; i < limb_shifts; ++i) {
        rop->limbs[i] = 0;
    }

    // Make sure remaining shift is within range of 64-bits
    shift %= 64;
    // Start from most-significant limb to second-to-least
    for(int i = 3; i > 0; --i) {
        // left shift the current limb, then attach the next limb's leading bits to this one's
        // trailing bits
        rop->limbs[i] = (rop->limbs[i] << shift) | (rop->limbs[i - 1] >> (64 - shift));
    }
    // Only left shift the last limb
    rop->limbs[0] <<= shift;
}


unsigned char uint256_add(uint256_t *rop, const uint256_t *op1, const uint256_t *op2) {
    unsigned char carry = 0;
    // Chain in series each addition
    for(int i = 0; i < 4; ++i) {
        // Add with carry
        carry = _addcarry_u64(carry, op1->limbs[i], op2->limbs[i], (unsigned long long*)&(rop->limbs[i]));
    }

    return carry;
}

int uint256_ctz(const uint256_t *op1) {
    // Make sure to check if the limb is non-zero, otherwise define it to be 64.
    int count = op1->limbs[0] ? (int)__builtin_ctzll(op1->limbs[0]) : 64;
    int count_limit = 64;

    // Continue through other limbs until a limb whose value is 0 is met.
    for(int i = 1; count == count_limit && i < 4; i++) {
        // Make sure to check if the limb is non-zero, otherwise define it to be 64.
        count += op1->limbs[i] ? (int)__builtin_ctzll(op1->limbs[i]) : 64;
        count_limit += 64;
    }

    return count;
}

int uint256_cmp(const uint256_t* op1, const uint256_t* op2) {
    int result = 0;
    // Continue from the most-significant limb until a non-zero result is found
    for(int i = 3; result == 0 && i >= 0; --i) {
        // Do a comparison by subtracting the negative condition from the positive condition
        result = (op1->limbs[i] > op2->limbs[i]) - (op1->limbs[i] < op2->limbs[i]);
    }

    return result;
}

void uint256_import(uint256_t *rop, const unsigned char *buffer) {
    size_t b = 0;

    // Zero-out the destination first
    memset(rop->limbs, 0, sizeof(*(rop->limbs) * 4));

    // Start from most-significant limb
    for(int i = 0; i < 4; ++i) {
        // Start from most-significant byte
        for(int j = 0; j < 8; ++j) {
            rop->limbs[i] >>= 8;
            rop->limbs[i] |= (uint64_t)buffer[b++] << 56;
        }
    }
}

void uint256_export(unsigned char *buffer, const uint256_t *rop) {
    int b = 0;

    // Start from most-significant limb
    for(int i = 0; i < 4; ++i) {
        // Start from most-significant byte
        for(int shift = 0; shift < 64; shift += 8) {
            buffer[b++] = (unsigned char)(rop->limbs[i] >> shift);
        }
    }
}