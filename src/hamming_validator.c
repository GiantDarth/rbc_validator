//
// Created by cp723 on 2/7/2019.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <uuid/uuid.h>
#include <omp.h>

#include "uint256_key_iter.h"
#include "aes256-ni.h"
#include "util.h"

#define ERROR_CODE_FOUND 0
#define ERROR_CODE_NOT_FOUND 1
#define ERROR_CODE_FAILURE 2

/// Given a starting permutation, iterate forward through every possible permutation until one that's matching
/// last_perm is found, or until a matching cipher is found.
/// \param starting_perm The permutation to start iterating from.
/// \param last_perm The final permutation to stop iterating at, inclusively.
/// \param key The original AES key.
/// \param key_size The key size in # of bytes, typically 32.
/// \param userId A uuid_t that's used to as the message to encrypt.
/// \param auth_cipher The authentication cipher to test against
/// \param signal A pointer to a shared value. Used to signal the function to prematurely leave.
/// \return Returns a 1 if found or a 0 if not. Returns a -1 if an error has occurred.
int gmp_validator(const uint256_t *starting_perm, const uint256_t *last_perm, const unsigned char *key,
        size_t key_size, uuid_t userId, const unsigned char *auth_cipher, const int* signal) {
    // Declaration
    unsigned char *corrupted_key;
    unsigned char cipher[sizeof(uuid_t)];
    int found = 0;

    uint256_key_iter *iter;
    aes256_enc_key_scheduler *key_scheduler;

    // Memory allocation
    if((corrupted_key = malloc(sizeof(*corrupted_key) * key_size)) == NULL) {
        perror("Error");
        return -1;
    }

    if((key_scheduler = aes256_enc_key_scheduler_create()) == NULL) {
        perror("Error");
        free(corrupted_key);

        return -1;
    }

    // Allocation and initialization
    if((iter = uint256_key_iter_create(key, starting_perm, last_perm)) == NULL) {
        perror("Error");
        aes256_enc_key_scheduler_destroy(key_scheduler);
        free(corrupted_key);

        return -1;
    }

    // While we haven't reached the end of iteration
    while(!uint256_key_iter_end(iter) && !(*signal)) {
        uint256_key_iter_get(iter, corrupted_key);
        aes256_enc_key_scheduler_update(key_scheduler, corrupted_key);

        // If encryption fails for some reason, break prematurely.
        if(aes256_ecb_encrypt(cipher, key_scheduler, userId, sizeof(uuid_t))) {
            found = -1;
            break;
        }

        // If the new cipher is the same as the passed in auth_cipher, set found to true and break
        if(memcmp(cipher, auth_cipher, sizeof(uuid_t)) == 0) {
            found = 1;
            break;
        }

        uint256_key_iter_next(iter);
    }

    // Cleanup
    aes256_enc_key_scheduler_destroy(key_scheduler);
    uint256_key_iter_destroy(iter);
    free(corrupted_key);

    return found;
}

/// OpenMP implementation
/// \return Returns a 0 on successfully finding a match, a 1 when unable to find a match,
/// and a 2 when a general error has occurred.
int main() {
    const size_t KEY_SIZE = 32;
    const size_t MISMATCHES = 3;
    // Use this line to manually set the # of threads, otherwise it detects it by your machine
//    omp_set_num_threads(1);

    gmp_randstate_t randstate;

    uuid_t userId;
    char uuid_str[37];

    unsigned char *key;
    unsigned char *corrupted_key;
    unsigned char auth_cipher[sizeof(uuid_t)];

    aes256_enc_key_scheduler *key_scheduler;

    // Memory allocation
    if((key = malloc(sizeof(*key) * KEY_SIZE)) == NULL) {
        perror("Error");
        return ERROR_CODE_FAILURE;
    }

    if((corrupted_key = malloc(sizeof(*corrupted_key) * KEY_SIZE)) == NULL) {
        perror("Error");
        free(key);
        return ERROR_CODE_FAILURE;
    }

    if((key_scheduler = aes256_enc_key_scheduler_create()) == NULL) {
        perror("Error");
        free(corrupted_key);
        free(key);

        return ERROR_CODE_FAILURE;
    }

    // Initialize values
    uuid_generate(userId);

    // Convert the uuid to a string for printing
    uuid_unparse(userId, uuid_str);
    printf("Using UUID: %s\n", uuid_str);

    // Set the gmp prng algorithm and set a seed based on the current time
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate, (unsigned long)time(NULL));

    get_random_key(key, KEY_SIZE, randstate);
    get_random_corrupted_key(corrupted_key, key, MISMATCHES, KEY_SIZE, randstate);

    aes256_enc_key_scheduler_update(key_scheduler, corrupted_key);
    if(aes256_ecb_encrypt(auth_cipher, key_scheduler, userId, sizeof(uuid_t))) {
        // Cleanup
        aes256_enc_key_scheduler_destroy(key_scheduler);
        free(corrupted_key);
        free(key);

        return ERROR_CODE_FAILURE;
    }

    double startTime = omp_get_wtime();
    int found = 0, signal = 0, error = 0;
#pragma omp parallel
    {
        uint256_t starting_perm, ending_perm;

        uint256_get_perm_pair(&starting_perm, &ending_perm, (size_t)omp_get_thread_num(),
                (size_t)omp_get_num_threads(), MISMATCHES, KEY_SIZE);

        int subfound = gmp_validator(&starting_perm, &ending_perm, key, KEY_SIZE, userId, auth_cipher, &signal);
        // If the result is positive, set the "global" found to 1. Will cause the other threads to
        // prematurely stop.
        if(subfound > 0) {
#pragma omp critical
            {
                found = 1;
                signal = 1;
            };
        }
        // If the result is negative, set a flag that an error has occurred, and stop the other threads.
        // Will cause the other threads to prematurely stop.
        else if(subfound < 0) {
            // Set the error flag, then set the signal to stop the other threads
#pragma omp critical
            {
                error = 1;
                signal = 1;
            };
        }
    }

    // Check if an error occurred in one of the threads.
    if(error) {
        // Cleanup
        aes256_enc_key_scheduler_destroy(key_scheduler);
        free(corrupted_key);
        free(key);

        return EXIT_FAILURE;
    }

    double duration = omp_get_wtime() - startTime;

    printf("Clock time: %f s\n", duration);
    printf("Found: %d\n", found);
  
    // Cleanup
    aes256_enc_key_scheduler_destroy(key_scheduler);
    free(corrupted_key);
    free(key);

    return found ? ERROR_CODE_FOUND : ERROR_CODE_NOT_FOUND;
}