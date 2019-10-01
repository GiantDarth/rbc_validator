#pragma clang diagnostic push
#pragma ide diagnostic ignored "openmp-use-default-none"
//
// Created by cp723 on 2/7/2019.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <omp.h>
#include <argp.h>

#include "iter/uint256_key_iter.h"
#include "util.h"
#include "../../micro-ecc/uECC.h"

#define ERROR_CODE_FOUND 0
#define ERROR_CODE_NOT_FOUND 1
#define ERROR_CODE_FAILURE 2

#define PRIV_KEY_SIZE 32
#define PUB_KEY_SIZE 64

const char *argp_program_version = "ecc_validator OpenMP 0.1.0";
const char *argp_program_bug_address = "<cp723@nau.edu, Chris.Coffey@nau.edu>";
error_t argp_err_exit_status = ERROR_CODE_FAILURE;

static char args_doc[] = "HOST_PRIV_KEY CLIENT_PUB_KEY\n-r/--random -m/--mismatches=value";
static char prog_desc[] = "Given an ECC secp256r1 and a HOST_PRIV_KEY (host private key)"
                          " and a CLIENT_PUB_KEY (client public key) from an unreliable"
                          " source, progressively corrupt the host private key by a"
                          " certain number of bits until a matching corrupted key is"
                          " found. The matching key will be sent to stdout."

                          "\n\nThis implementation uses OpenMP."

                          "\vIf the key is found then the program will have an exit code"
                          " 0. If not found, e.g. when providing --mismatches and"
                          " especially --exact, then the program will have an exit code"
                          " 1. For any general error, such as parsing, out-of-memory,"
                          " etc., the program will have an exit code 2."

                          "\n\nThe original HOST_PRIV_KEY (host private key), passed in as"
                          " hexadecimal, is corrupted by a certain number of bits. The"
                          " resulting new private key's public key is compared against"
                          " the CLIENT_PUB_KEY (client public key) which is also passed"
                          " in hexadecimal in uncompressed form."

                          "\n\nOnly ECC secp256r1 keys are currently supported.";

struct arguments {
    int verbose, benchmark, random, fixed, count, all;
    char *host_priv_key_hex, *client_pub_key_hex;
    int mismatches, subkey_length, threads;
};

static struct argp_option options[] = {
    {"all", 'a', 0, 0, "Don't cut out early when key is found."},
    {"mismatches", 'm', "value", 0, "The largest # of bits of corruption to test against,"
                                    " inclusively. Defaults to -1. If negative, then the"
                                    " size of key in bits will be the limit. If in random"
                                    " or benchmark mode, then this will also be used to"
                                    " corrupt the random key by the same # of bits; for"
                                    " this reason, it must be set and non-negative when"
                                    " in random or benchmark mode. Cannot be larger than"
                                    " what --subkey-size is set to."},
    {"subkey", 's', "value", 0, "How many of the first bits to corrupt and iterate over."
                                " Must be between 1 and 256 bits. Defaults to 256."},
    {"count", 'c', 0, 0, "Count the number of keys tested and show it as verbose output."},
    {"fixed", 'f', 0, 0, "Only test the given mismatch, instead of progressing from 0 to"
                         " --mismatches. This is only valid when --mismatches is set and"
                         " non-negative."},
    {"random", 'r', 0, 0, "Instead of using arguments, randomly generate CIPHER, KEY, and"
                          " UUID. This must be accompanied by --mismatches, since it is used to"
                          " corrupt the random key by the same # of bits. --random and"
                          " --benchmark cannot be used together."},
    {"benchmark", 'b', 0, 0, "Instead of using arguments, strategically generate CIPHER, KEY, and"
                             " UUID. Specifically, generates a corrupted key that's always 50% of"
                             " way through a rank's workload, but randomly chooses the thread."
                             " --random and --benchmark cannot be used together."},
    {"verbose", 'v', 0, 0, "Produces verbose output and time taken to stderr."},
    {"threads", 't', "count", 0, "How many worker threads to use. Defaults to 0. If set to 0,"
                                 " then the number of threads used will be detected by the"
                                 " system." },
    { 0 }
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;

    // Used for strtol
    char *endptr;
    long value;

    switch(key) {
        case 'v':
            arguments->verbose = 1;
            break;
        case 'c':
            arguments->count = 1;
            break;
        case 'r':
            arguments->random = 1;
            break;
        case 'b':
            arguments->benchmark = 1;
            break;
        case 'f':
            arguments->fixed = 1;
            break;
        case 'a':
            arguments->all = 1;
            break;
        case 'm':
            errno = 0;
            value = strtol(arg, &endptr, 10);

            if((errno == ERANGE && (value == LONG_MAX || value == LONG_MIN))
                    || (errno && value == 0)) {
                argp_failure(state, ERROR_CODE_FAILURE, errno, "--mismatches");
            }

            if(*endptr != '\0') {
                argp_error(state, "--mismatches contains invalid characters.\n");
            }

            if (value > PRIV_KEY_SIZE * 8) {
                fprintf(stderr, "--mismatches cannot exceed the private key size for secp256r1"
                                " in bits.\n");
            }

            arguments->mismatches = (int)value;

            break;
        case 's':
            errno = 0;
            value = strtol(arg, &endptr, 10);

            if((errno == ERANGE && (value == LONG_MAX || value == LONG_MIN))
               || (errno && value == 0)) {
                argp_failure(state, ERROR_CODE_FAILURE, errno, "--subkey");
            }

            if(*endptr != '\0') {
                argp_error(state, "--subkey contains invalid characters.\n");
            }

            if (value > PRIV_KEY_SIZE * 8) {
                argp_error(state, "--subkey cannot exceed the private key size for secp256r1"
                                  "in bits.\n");
            }
            else if (value < 1) {
                argp_error(state, "--subkey must be at least 1.\n");
            }

            arguments->subkey_length = (int)value;

            break;
        case 't':
            errno = 0;
            value = strtol(arg, &endptr, 10);

            if((errno == ERANGE && (value == LONG_MAX || value == LONG_MIN))
                    || (errno && value == 0)) {
                argp_failure(state, ERROR_CODE_FAILURE, errno, "--threads");
            }

            if(*endptr != '\0') {
                argp_error(state, "--threads contains invalid characters.\n");
            }

            if(value > omp_get_thread_limit()) {
                argp_error(state, "--threads exceeds program thread limit.\n");
            }

            arguments->threads = (int)value;

            break;
        case ARGP_KEY_ARG:
            switch(state->arg_num) {
                case 0:
                    if(strlen(arg) != PRIV_KEY_SIZE * 2) {
                        argp_error(state, "The HOST_PRIV_KEY (host private key) must be 32 bytes"
                                          " long for secp256r1.\n");
                    }
                    arguments->host_priv_key_hex = arg;
                    break;
                case 1:
                    if(strlen(arg) != PUB_KEY_SIZE * 2) {
                        argp_error(state, "The CLIENT_PUB_KEY (client public key) must be 64 bytes"
                                          " long for secp256r1.\n");
                    }
                    arguments->client_pub_key_hex = arg;
                    break;
                default:
                    argp_usage(state);
            }
            break;
        case ARGP_KEY_NO_ARGS:
            if(!arguments->random && !arguments->benchmark) {
                argp_usage(state);
            }
            break;
        case ARGP_KEY_END:
            if(arguments->mismatches < 0) {
                if(arguments->random) {
                    argp_error(state, "--mismatches must be set and non-negative when using"
                                      " --random.\n");
                }
                if(arguments->benchmark) {
                    argp_error(state, "--mismatches must be set and non-negative when using"
                                      " --benchmark.\n");
                }
                if(arguments->fixed) {
                    argp_error(state, "--mismatches must be set and non-negative when using"
                                      " --fixed.\n");
                }
            }

            if(arguments->random && arguments->benchmark) {
                argp_error(state, "--random and --benchmark cannot be both set simultaneously.\n");
            }

            if(arguments->mismatches > arguments->subkey_length) {
                argp_error(state, "--mismatches cannot be set larger than --subkey.\n");
            }

            break;
        case ARGP_KEY_INIT:
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

/// Given a starting permutation, iterate forward through every possible permutation until one that's
/// matching last_perm is found, or until a matching cipher is found.
/// \param corrupted_priv_key An allocated corrupted key to fill if the corrupted key was found. Must be at
/// least key_size bytes big.
/// \param starting_perm The permutation to start iterating from.
/// \param last_perm The final permutation to stop iterating at, inclusively.
/// \param host_priv_key The original host private key (32 bytes).
/// \param client_pub_key The client public key (64 bytes).
/// \param userId A uuid_t that's used to as the message to encrypt.
/// \param auth_cipher The authentication cipher to test against
/// \param signal A pointer to a shared value. Used to signal the function to prematurely leave.
/// \param all If benchmark mode is set to a non-zero value, then continue even if found.
/// \param validated_keys A counter to keep track of how many keys were traversed. If NULL, then this is
/// skipped.
/// \return Returns a 1 if found or a 0 if not. Returns a -1 if an error has occurred.
int gmp_validator(unsigned char *corrupted_priv_key, const uint256_t *starting_perm,
        const uint256_t *last_perm, const unsigned char *host_priv_key,
        const unsigned char *client_pub_key,const int* signal, int all, mpz_t *validated_keys) {
    // Declarations
    int found = 0;
    const struct uECC_Curve_t * curve = uECC_secp256r1();
    unsigned char current_priv_key[PRIV_KEY_SIZE]; // this one changes, until found
    unsigned char current_pub_key[PUB_KEY_SIZE];   // this is generated from current_priv_key
    uint256_key_iter *iter;

    if((iter = uint256_key_iter_create(corrupted_priv_key, starting_perm, last_perm)) == NULL) {
        perror("ERROR");
        return -1;
    }

    if(all) { // all == don't cut out early if end is found
        // While we haven't reached the end of iteration
        while(!uint256_key_iter_end(iter)) {
            if(validated_keys != NULL) {
                mpz_add_ui(*validated_keys, *validated_keys, 1);
            }
            // get next current_priv_key
            uint256_key_iter_get(iter, current_priv_key);

            // If fails for some reason, break prematurely.
            if (! uECC_compute_public_key(current_priv_key, current_pub_key, curve)) {
                printf("ERROR uECC_compute_public_key - abort run");
                found = -1;
                break;
            }

            // If the new cipher is the same as the passed in auth_cipher, set found to true and break
            if(memcmp(current_pub_key, client_pub_key, PUB_KEY_SIZE) == 0) {
                found = 1;
                // Only have one thread copy the key at a time
                // This might happen more than once if the # of threads exceeds the number of possible
                // keys
#pragma omp critical
                memcpy(corrupted_priv_key, current_priv_key, PRIV_KEY_SIZE);
            }

            uint256_key_iter_next(iter);
        } // while(!uint256_key_iter_end(iter))
    }
    else { // all == false : cut out early once found or iter end
        // While we haven't reached the end of iteration
        while(!uint256_key_iter_end(iter) && !(*signal)) {
            if(validated_keys != NULL) {
                mpz_add_ui(*validated_keys, *validated_keys, 1);
            }
            // get next current_priv_key
            uint256_key_iter_get(iter, current_priv_key);

            // If fails for some reason, break prematurely.
            if (! uECC_compute_public_key(current_priv_key, current_pub_key, curve)) {
                printf("ERROR uECC_compute_public_key - abort run");
                found = -1;
                break;
            }

            // If the new cipher is the same as the passed in auth_cipher, set found to true and break
            if(memcmp(current_pub_key, client_pub_key, PUB_KEY_SIZE) == 0) {
                found = 1;
                // Only have one thread copy the key at a time
                // This might happen more than once if the # of threads exceeds the number of possible
                // keys
#pragma omp critical
                memcpy(corrupted_priv_key, current_priv_key, PRIV_KEY_SIZE);
                break;
            }

            uint256_key_iter_next(iter);
        } // while(!uint256_key_iter_end(iter) && !(*signal))
    } // else all

    // Cleanup
    uint256_key_iter_destroy(iter);

    return found;
}

/// OpenMP implementation
/// \return Returns a 0 on successfully finding a match, a 1 when unable to find a match,
/// and a 2 when a general error has occurred.
int main(int argc, char *argv[]) {
    int numcores = 0;
    struct arguments arguments;
    static struct argp argp = {options, parse_opt, args_doc, prog_desc};

    gmp_randstate_t randseed;

    const struct uECC_Curve_t * curve = uECC_secp256r1();
    unsigned char *host_priv_key;
    //unsigned char pub_prefix;
    unsigned char *client_pub_key;
    unsigned char *corrupted_priv_key; // the key is corrupted, unless it is found

    int mismatch, ending_mismatch;

    double start_time, duration;
    mpz_t validated_keys;
    int found, subfound, signal, error;

    // Memory allocation
    if ((host_priv_key = malloc(sizeof(*host_priv_key) * PRIV_KEY_SIZE)) == NULL) {
        perror("ERROR");
        return ERROR_CODE_FAILURE;
    }

    if ((client_pub_key = malloc(sizeof(*client_pub_key) * PUB_KEY_SIZE)) == NULL) {
        perror("ERROR");
        free(host_priv_key);
        return ERROR_CODE_FAILURE;
    }

    if ((corrupted_priv_key = malloc(sizeof(*corrupted_priv_key) * PRIV_KEY_SIZE)) == NULL) {
        perror("ERROR");
        free(client_pub_key);
        free(host_priv_key);
        return ERROR_CODE_FAILURE;
    }

    mpz_init(validated_keys);

    memset(&arguments, 0, sizeof(arguments));
    arguments.host_priv_key_hex = NULL;
    arguments.client_pub_key_hex = NULL;
    // Default to -1 for no mismatches provided, aka. go through all mismatches.
    arguments.mismatches = -1;
    arguments.subkey_length = PRIV_KEY_SIZE * 8;

    // Parse arguments
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    // Initialize values
    // Set the gmp prng algorithm and set a seed based on the current time
    gmp_randinit_default(randseed);
    gmp_randseed_ui(randseed, (unsigned long)time(NULL));

    mismatch = 0;
    ending_mismatch = arguments.subkey_length;

    // If --fixed option was set, set the validation range to only use the --mismatches value.
    if (arguments.fixed) {
        mismatch = arguments.mismatches;
        ending_mismatch = arguments.mismatches;
    }
    // If --mismatches is set and non-negative, set the ending_mismatch to its value.
    else if(arguments.mismatches >= 0) {
        ending_mismatch = arguments.mismatches;
    }

    if (arguments.threads > 0) {
        omp_set_num_threads(arguments.threads);
    }

    // omp_get_num_threads() must be called in a parallel region, but
    // ensure that only one thread calls it
#pragma omp parallel default(none) shared(numcores)
#pragma omp single
    numcores = omp_get_num_threads();

    if (arguments.random || arguments.benchmark) {
        if(arguments.random) {
            fprintf(stderr, "WARNING: Random mode set. All three arguments will be ignored and randomly"
                            " generated ones will be used in their place.\n");
        }
        else if(arguments.benchmark) {
            fprintf(stderr, "WARNING: Benchmark mode set. All three arguments will be ignored and"
                            " randomly generated ones will be used in their place.\n");
        }

        // good key - random
        get_random_key(host_priv_key, PRIV_KEY_SIZE, randseed);
        // get a corrupt key from a good key
        get_random_corrupted_key(corrupted_priv_key, host_priv_key, arguments.mismatches, PRIV_KEY_SIZE,
                arguments.subkey_length, randseed, arguments.benchmark, numcores);

        if (! uECC_compute_public_key(host_priv_key, client_pub_key, curve)) {
            printf("ERROR host uECC_compute_public_key - abort run");
            //found = -1;
            return ERROR_CODE_FAILURE;
        }
    }
    else { // not arguments.random || arguments.benchmark
        switch(parse_hex(host_priv_key, arguments.host_priv_key_hex)) {
            case 1:
                fprintf(stderr, "ERROR: HOST_PRIV_KEY had non-hexadecimal characters.\n");
                return ERROR_CODE_FAILURE;
            case 2:
                fprintf(stderr, "ERROR: HOST_PRIV_KEY did not have even length.\n");
                return ERROR_CODE_FAILURE;
            default:
                break;
        }

        switch(parse_hex(client_pub_key, arguments.client_pub_key_hex)) {
            case 1:
                fprintf(stderr, "ERROR: CLIENT_PUB_KEY had non-hexadecimal characters.\n");
                return ERROR_CODE_FAILURE;
            case 2:
                fprintf(stderr, "ERROR: CLIENT_PUB_KEY did not have even length.\n");
                return ERROR_CODE_FAILURE;
            default:
                break;
        }

        unsigned char *temp_host_pub_key;
        if ((temp_host_pub_key = malloc(sizeof(*temp_host_pub_key) * PUB_KEY_SIZE)) == NULL) {
            perror("ERROR");
            // Cleanup
            mpz_clear(validated_keys);
            free(corrupted_priv_key);
            free(client_pub_key);
            free(host_priv_key);
            return ERROR_CODE_FAILURE;
        }
        if (! uECC_compute_public_key(host_priv_key, temp_host_pub_key, curve)) {
            printf("ERROR host uECC_compute_public_key - abort run");
            // Cleanup
            mpz_clear(validated_keys);
            free(corrupted_priv_key);
            free(client_pub_key);
            free(host_priv_key);
            free(temp_host_pub_key);
            return ERROR_CODE_FAILURE;
        }

        // is the initial client_pub_key free of errors?
        if (memcmp(temp_host_pub_key, client_pub_key, PUB_KEY_SIZE) == 0) {
            if(arguments.verbose) fprintf(stdout, "found with no errors:\n");
            fprint_hex(stdout, host_priv_key, PRIV_KEY_SIZE);
            printf("\n");
            mpz_clear(validated_keys);
            free(corrupted_priv_key);
            free(client_pub_key);
            free(host_priv_key);
            free(temp_host_pub_key);
            return ERROR_CODE_FOUND;
        }

        // errors, so let try to find a valid corrupted_priv_key / client_pub_key pair
        memcpy(corrupted_priv_key, host_priv_key, PRIV_KEY_SIZE);
    }

    if (arguments.verbose) {
        fprintf(stderr, "INFO: Using secp256r1 Host Private Key            : ");
        fprint_hex(stderr, host_priv_key, PRIV_KEY_SIZE);
        fprintf(stderr, "\n");

        if(arguments.random) {
            fprintf(stderr, "INFO: Using secp256r1 Corrupted Key (%d mismatches): ", arguments.mismatches);
            fprint_hex(stderr, corrupted_priv_key, PRIV_KEY_SIZE);
            fprintf(stderr, "\n");
        }

        fprintf(stderr, "INFO: Using secp256r1 Client Public Key:\n ");
        fprint_hex(stderr, client_pub_key, PUB_KEY_SIZE);
        fprintf(stderr, "\n");
    }

    start_time = omp_get_wtime();
    found = 0;
    signal = 0;
    error = 0;

    for (; mismatch <= ending_mismatch && !found; mismatch++) {
        if(arguments.verbose) {
            fprintf(stderr, "INFO: Checking a hamming distance of %d...\n", mismatch);
        }

#pragma omp parallel default(shared)
        {
            uint256_t starting_perm, ending_perm;
            mpz_t sub_validated_keys;

            mpz_init(sub_validated_keys);

            uint256_get_perm_pair(&starting_perm, &ending_perm, (size_t) omp_get_thread_num(),
                                  (size_t) omp_get_num_threads(), mismatch, PRIV_KEY_SIZE,
                                  arguments.subkey_length);

            int subfound = gmp_validator(corrupted_priv_key, &starting_perm, &ending_perm, host_priv_key,
                    client_pub_key, &signal, arguments.all, arguments.count ? &sub_validated_keys : NULL);
            // If the result is positive, set the "global" found to 1. Will cause the other threads to
            // prematurely stop.
            if (subfound > 0) {
#pragma omp critical
                {
                    found = 1;
                    signal = 1;
                }
            }
            // If the result is negative, set a flag that an error has occurred, and stop the other threads.
            // Will cause the other threads to prematurely stop.
            else if (subfound < 0) {
                // Set the error flag, then set the signal to stop the other threads
#pragma omp critical
                {
                    error = 1;
                    signal = 1;
                }
            }
#pragma omp critical
            {
                if(arguments.count) {
                    mpz_add(validated_keys, validated_keys, sub_validated_keys);
                }
            }

            mpz_clear(sub_validated_keys);
        } // omp parallel
    } // for mismatch

    // Check if an error occurred in one of the threads.
    if(error) {
        // Cleanup
        mpz_clear(validated_keys);
        free(corrupted_priv_key);
        free(client_pub_key);
        free(host_priv_key);
        return EXIT_FAILURE;
    }

    duration = omp_get_wtime() - start_time;

    if(arguments.verbose) {
        fprintf(stderr, "INFO: Clock time: %f s\n", duration);
        fprintf(stderr, "INFO: Found: %d\n", found);
    }

    if(arguments.count) {
        mpf_t duration_mpf, key_rate;
        mpf_inits(duration_mpf, key_rate, NULL);

        mpf_set_d(duration_mpf, duration);
        mpf_set_z(key_rate, validated_keys);

        // Divide validated_keys by duration
        mpf_div(key_rate, key_rate, duration_mpf);

        gmp_fprintf(stderr, "INFO: Keys searched: %Zd\n", validated_keys);
        gmp_fprintf(stderr, "INFO: Keys per second: %.9Fg\n", key_rate);

        mpf_clears(duration_mpf, key_rate, NULL);
    }

    if(found) {
        if(arguments.verbose) fprintf(stdout, "found:\n");
        fprint_hex(stdout, corrupted_priv_key, PRIV_KEY_SIZE);
        printf("\n");
    } else {
        if(arguments.verbose) fprintf(stdout, "not-found <============\n");
    }

    // Cleanup
    mpz_clear(validated_keys);
    free(corrupted_priv_key);
    free(client_pub_key);
    free(host_priv_key);

    return found ? ERROR_CODE_FOUND : ERROR_CODE_NOT_FOUND;
}

#pragma clang diagnostic pop