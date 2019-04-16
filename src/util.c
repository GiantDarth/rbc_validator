//
// Created by cp723 on 2/7/2019.
//

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void decode_ordinal(mpz_t perm, const mpz_t ordinal, int mismatches, size_t key_size) {
    mpz_t binom, curr_ordinal;
    mpz_inits(binom, curr_ordinal, NULL);

    mpz_set(curr_ordinal, ordinal);

    mpz_set_ui(perm, 0);
    for (unsigned long bit = key_size * 8 - 1; mismatches > 0; bit--)
    {
        mpz_bin_uiui(binom, bit, mismatches);
        if (mpz_cmp(curr_ordinal, binom) >= 0)
        {
            mpz_sub(curr_ordinal, curr_ordinal, binom);
            mpz_setbit(perm, bit);
            mismatches--;
        }
    }

    mpz_clears(binom, curr_ordinal, NULL);
}

void get_random_permutation(mpz_t perm, int mismatches, size_t key_size, gmp_randstate_t randstate) {
    mpz_t ordinal, binom;
    mpz_inits(ordinal, binom, NULL);

    mpz_bin_uiui(binom, key_size * 8, mismatches);

    mpz_urandomm(ordinal, randstate, binom);
    decode_ordinal(perm, ordinal, mismatches, key_size);

    mpz_clears(ordinal, binom, NULL);
}

void generate_starting_permutations(mpz_t *starting_perms, size_t starting_perms_size, int mismatches,
                                    size_t key_size) {
    // Always set the first one to the global first permutation
    gmp_assign_first_permutation(starting_perms[0], mismatches);

    mpz_t ordinal, chunk_size;
    mpz_inits(ordinal, chunk_size, NULL);

    mpz_bin_uiui(chunk_size, key_size * 8, mismatches);
    mpz_tdiv_q_ui(chunk_size, chunk_size, starting_perms_size);

    for(size_t i = 0; i < starting_perms_size; i++) {
        mpz_mul_ui(ordinal, chunk_size, i);

        decode_ordinal(starting_perms[i], ordinal, mismatches, key_size);
    }

    mpz_clears(ordinal, chunk_size, NULL);
}

void gmp_assign_first_permutation(mpz_t perm, int mismatches) {
    // Set perm to first key
    // Equivalent to: (perm << mismatches) - 1
    mpz_set_ui(perm, 1);
    mpz_mul_2exp(perm, perm, mismatches);
    mpz_sub_ui(perm, perm, 1);
}

void gmp_assign_last_permutation(mpz_t perm, int mismatches, size_t key_size) {
    // First set the value to the first permutation.
    gmp_assign_first_permutation(perm, mismatches);
    // Equivalent to: perm << ((key_size * 8) - mismatches)
    // E.g. if key_size = 32 and mismatches = 5, then there are 256-bits
    // Then we want to shift left 256 - 5 = 251 times.
    mpz_mul_2exp(perm, perm, (key_size * 8) - mismatches);
}

void uint256_assign_first_permutation(uint256_t *perm, int mismatches) {
    // Set perm to first key
    // Equivalent to: (perm << mismatches) - 1
    uint256_shift_left(perm, perm, (int)mismatches);
    uint256_add(perm, perm, &UINT256_NEG_ONE);
}

void uint256_assign_last_permutation(uint256_t *perm, int mismatches, size_t key_size) {
    // First set the value to the first permutation.
    uint256_assign_first_permutation(perm, mismatches);
    // Equivalent to: perm << ((key_size * 8) - mismatches)
    // E.g. if key_size = 32 and mismatches = 5, then there are 256-bits
    // Then we want to shift left 256 - 5 = 251 times.
    uint256_shift_left(perm, perm, (int)((key_size * 8) - mismatches));
}

void get_random_key(unsigned char *key, size_t key_size, gmp_randstate_t randstate) {
    mpz_t key_mpz;
    mpz_init(key_mpz);

    mpz_urandomb(key_mpz, randstate, key_size * 8);

    mpz_export(key, NULL, sizeof(*key), 1, 0, 0, key_mpz);

    mpz_clear(key_mpz);
}

void get_random_corrupted_key(unsigned char *corrupted_key, const unsigned char *key, int mismatches,
                              size_t key_size, gmp_randstate_t randstate) {
    mpz_t key_mpz, corrupted_key_mpz, perm;
    mpz_inits(key_mpz, corrupted_key_mpz, perm, NULL);

    get_random_permutation(perm, mismatches, key_size, randstate);

    mpz_import(key_mpz, key_size, 1, sizeof(*key), 0, 0, key);

    // Perform an XOR operation between the permutation and the key.
    // If a bit is set in permutation, then flip the bit in the key.
    // Otherwise, leave it as is.
    mpz_xor(corrupted_key_mpz, key_mpz, perm);

    mpz_export(corrupted_key, NULL, sizeof(*corrupted_key), 1, 0, 0, corrupted_key_mpz);

    mpz_clears(key_mpz, corrupted_key_mpz, perm, NULL);
}

void gmp_get_perm_pair(mpz_t starting_perm, mpz_t ending_perm, size_t pair_index, size_t pair_count,
                   int mismatches, size_t key_size) {
    mpz_t total_perms, starting_ordinal, ending_ordinal;
    mpz_inits(total_perms, starting_ordinal, ending_ordinal, NULL);

    mpz_bin_uiui(total_perms, key_size * 8, mismatches);

    if(pair_index == 0) {
        gmp_assign_first_permutation(starting_perm, mismatches);
    }
    else {
        mpz_tdiv_q_ui(starting_ordinal, total_perms, pair_count);
        mpz_mul_ui(starting_ordinal, starting_ordinal, pair_index);

        decode_ordinal(starting_perm, starting_ordinal, mismatches, key_size);
    }

    if(pair_index == pair_count - 1) {
        gmp_assign_last_permutation(ending_perm, mismatches, key_size);
    }
    else {
        mpz_tdiv_q_ui(ending_ordinal, total_perms, pair_count);
        mpz_mul_ui(ending_ordinal, ending_ordinal, pair_index + 1);

        decode_ordinal(ending_perm, ending_ordinal, mismatches, key_size);
    }

    mpz_clears(total_perms, starting_ordinal, ending_ordinal, NULL);
}

void uint256_get_perm_pair(uint256_t *starting_perm, uint256_t *ending_perm, size_t pair_index,
        size_t pair_count, int mismatches, size_t key_size) {
    mpz_t starting_perm_mpz, ending_perm_mpz;
    uint64_t *starting_perm_buffer, *ending_perm_buffer;

    mpz_inits(starting_perm_mpz, ending_perm_mpz, NULL);

    gmp_get_perm_pair(starting_perm_mpz, ending_perm_mpz, pair_index, pair_count, mismatches, key_size);

    size_t count;
    starting_perm_buffer = mpz_export(NULL, &count, -1, sizeof(*starting_perm_buffer),
            0, 0, starting_perm_mpz);
    memset(starting_perm->limbs, 0, 4 * sizeof(*starting_perm_buffer));
    memcpy(starting_perm->limbs, starting_perm_buffer, count * sizeof(*starting_perm_buffer));

    ending_perm_buffer = mpz_export(NULL, &count, -1, sizeof(*ending_perm_buffer),
            0, 0, ending_perm_mpz);
    memset(ending_perm->limbs, 0, 4 * sizeof(*ending_perm_buffer));
    memcpy(ending_perm->limbs, ending_perm_buffer, count * sizeof(*ending_perm_buffer));

    free(ending_perm_buffer);
    free(starting_perm_buffer);
    mpz_clears(starting_perm_mpz, ending_perm_mpz, NULL);
}

void fprint_hex(FILE *stream, const unsigned char *array, size_t count) {
    for(size_t i = 0; i < count; i++) {
        fprintf(stream, "%02x", array[i]);
    }
}

/// Parse an individual hexadecimal character to an integer 0 to 15.
/// \param hex_char An individual hexadecimal character.
/// \return Return 0 to 15 depending on the value of hex_char, else return -1 on an invalid character.
int parse_hex_char(char hex_char) {
    if(hex_char >= '0' && hex_char <= '9') {
        return hex_char - 48;
    }
    else if(hex_char >= 'A' && hex_char <= 'F') {
        return hex_char - 55;
    }
    else if(hex_char >= 'a' && hex_char <= 'f') {
        return hex_char - 87;
    }
    else {
        return -1;
    }
}

int parse_hex(unsigned char *array, char *hex_string) {
    size_t i, b;
    int value;

    for(i = 0, b = 0; hex_string[b] != '\0'; i++) {
        if((value = parse_hex_char(hex_string[b++])) < 0) {
            return 1;
        }

        array[i] = (unsigned char)value << 4;

        // The length of hex string was odd
        if(hex_string[b] == '\0') {
            return 2;
        }

        if((value = parse_hex_char(hex_string[b++])) < 0) {
            return 1;
        }

        array[i] |= (unsigned char)value;
    }

    return 0;
}