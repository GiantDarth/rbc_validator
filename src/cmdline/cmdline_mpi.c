/*
  File autogenerated by gengetopt version 2.23
  generated with the following command:
  gengetopt --output-dir=src/cmdline --default-optional --unnamed-opts --file-name cmdline_mpi --no-handle-error

  The developers of gengetopt consider the fixed text that goes in all
  gengetopt output files to be in the public domain:
  we make no copyright claims on it.
*/

/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIX_UNUSED
#define FIX_UNUSED(X) (void) (X) /* avoid warnings for unused params */
#endif

#include <getopt.h>

#include "cmdline_mpi.h"

const char *gengetopt_args_info_purpose = "\nGiven an HOST_SEED and either:\n1) an AES256 CLIENT_CIPHER and plaintext UUID;\n2) a ChaCha20 CLIENT_CIPHER, plaintext UUID, and IV;\n3) an ECC Secp256r1 CLIENT_PUB_KEY;\nwhere CLIENT_* is from an unreliable source. Progressively corrupt the chosen\ncryptographic function by a certain number of bits until a matching client seed\nis found. The matching HOST_* will be sent to stdout, depending on the\ncryptographic function.\n\nThis implementation uses MPI.";

const char *gengetopt_args_info_usage = "Usage: rbc_validator_mpi [OPTIONS...] --mode=none HOST_SEED\n  or : rbc_validator_mpi [OPTIONS...] --mode=[aes,chacha20] HOST_SEED\nCLIENT_CIPHER UUID [IV]\n  or : rbc_validator_mpi [OPTIONS...] --mode=ecc HOST_SEED CLIENT_PUB_KEY\n  or : rbc_validator_mpi [OPTIONS...] --mode=* -r/--random\n-m/--mismatches=value\n  or : rbc_validator_mpi [OPTIONS...] --mode=* -b/--benchmark\n-m/--mismatches=value\nTry `rbc_validator_mpi --help' for more information.";

const char *gengetopt_args_info_versiontext = "Christopher Robert Philabaum <cp723@nau.edu>";

const char *gengetopt_args_info_description = "If the client seed is found then the program will have an exit code 0. If not\nfound, e.g. when providing --mismatches and especially --exact, then the\nprogram will have an exit code 1. For any general error, such as parsing,\nout-of-memory, etc., the program will have an exit code 2.\n\nThe original HOST_SEED, passed in as hexadecimal, is corrupted by a certain\nnumber of bits and used to generate the cryptographic output. HOST_SEED is\nalways 32 bytes, which corresponds to 64 hexadecimal characters.";

const char *gengetopt_args_info_help[] = {
  "  -h, --help              Print help and exit",
  "  -V, --version           Print version and exit",
  "      --usage             Give a short usage message",
  "      --mode=ENUM         (REQUIRED) The cryptographic function to iterate\n                            against. If `none', then only perform\n                            seed iteration.  (possible values=\"none\",\n                            \"aes\", \"chacha20\", \"ecc\")",
  "  -m, --mismatches=value  The largest # of bits of corruption to test against,\n                            inclusively. Defaults to -1. If negative, then the\n                            size of key in bits will be the limit. If in random\n                            or benchmark mode, then this will also be used to\n                            corrupt the random key by the same # of bits; for\n                            this reason, it must be set and non-negative when\n                            in random or benchmark mode. Cannot be larger than\n                            what --subkey-size is set to.  (default=`-1')",
  "  -s, --subkey=value      How many of the first bits to corrupt and iterate\n                            over. Must be between 1 and 256. Defaults to 256.\n                            (default=`256')",
  "\n Mode: Random",
  "  -r, --random            Instead of using arguments, randomly generate\n                            HOST_SEED and CLIENT_*. This must be accompanied by\n                            --mismatches, since it is used to corrupt the\n                            random key by the same # of bits. --random and\n                            --benchmark cannot be used together.  (default=off)",
  "\n Mode: Benchmark",
  "  -b, --benchmark         Instead of using arguments, strategically generate\n                            HOST_SEED and CLIENT_*. Specifically, generates a\n                            client seed that's always 50% of the way through a\n                            rank's workload, but randomly chooses the thread.\n                            --random and --benchmark cannot be used together.\n                            (default=off)",
  "  -a, --all               Don't cut out early when key is found.  (default=off)",
  "  -c, --count             Count the number of keys tested and show it as\n                            verbose output.  (default=off)",
  "  -f, --fixed             Only test the given mismatch, instead of progressing\n                            from 0 to --mismatches. This is only valid when\n                            --mismatches is set and non-negative.\n                            (default=off)",
  "  -v, --verbose           Produces verbose output and time taken to stderr.\n                            (default=off)",
    0
};

typedef enum {ARG_NO
  , ARG_FLAG
  , ARG_INT
  , ARG_ENUM
} cmdline_parser_arg_type;

static
void clear_given (struct gengetopt_args_info *args_info);
static
void clear_args (struct gengetopt_args_info *args_info);

static int
cmdline_parser_internal (int argc, char **argv, struct gengetopt_args_info *args_info,
                        struct cmdline_parser_params *params, const char *additional_error);


const char *cmdline_parser_mode_values[] = {"none", "aes", "chacha20", "ecc", 0}; /*< Possible values for mode. */

static char *
gengetopt_strdup (const char *s);

static
void clear_given (struct gengetopt_args_info *args_info)
{
  args_info->help_given = 0 ;
  args_info->version_given = 0 ;
  args_info->usage_given = 0 ;
  args_info->mode_given = 0 ;
  args_info->mismatches_given = 0 ;
  args_info->subkey_given = 0 ;
  args_info->random_given = 0 ;
  args_info->benchmark_given = 0 ;
  args_info->all_given = 0 ;
  args_info->count_given = 0 ;
  args_info->fixed_given = 0 ;
  args_info->verbose_given = 0 ;
  args_info->Benchmark_mode_counter = 0 ;
  args_info->Random_mode_counter = 0 ;
}

static
void clear_args (struct gengetopt_args_info *args_info)
{
  FIX_UNUSED (args_info);
  args_info->mode_arg = mode__NULL;
  args_info->mode_orig = NULL;
  args_info->mismatches_arg = -1;
  args_info->mismatches_orig = NULL;
  args_info->subkey_arg = 256;
  args_info->subkey_orig = NULL;
  args_info->random_flag = 0;
  args_info->benchmark_flag = 0;
  args_info->all_flag = 0;
  args_info->count_flag = 0;
  args_info->fixed_flag = 0;
  args_info->verbose_flag = 0;
  
}

static
void init_args_info(struct gengetopt_args_info *args_info)
{


  args_info->help_help = gengetopt_args_info_help[0] ;
  args_info->version_help = gengetopt_args_info_help[1] ;
  args_info->usage_help = gengetopt_args_info_help[2] ;
  args_info->mode_help = gengetopt_args_info_help[3] ;
  args_info->mismatches_help = gengetopt_args_info_help[4] ;
  args_info->subkey_help = gengetopt_args_info_help[5] ;
  args_info->random_help = gengetopt_args_info_help[7] ;
  args_info->benchmark_help = gengetopt_args_info_help[9] ;
  args_info->all_help = gengetopt_args_info_help[10] ;
  args_info->count_help = gengetopt_args_info_help[11] ;
  args_info->fixed_help = gengetopt_args_info_help[12] ;
  args_info->verbose_help = gengetopt_args_info_help[13] ;
  
}

void
cmdline_parser_print_version (void)
{
  printf ("%s %s\n",
     (strlen(CMDLINE_PARSER_PACKAGE_NAME) ? CMDLINE_PARSER_PACKAGE_NAME : CMDLINE_PARSER_PACKAGE),
     CMDLINE_PARSER_VERSION);

  if (strlen(gengetopt_args_info_versiontext) > 0)
    printf("\n%s\n", gengetopt_args_info_versiontext);
}

static void print_help_common(void)
{
	size_t len_purpose = strlen(gengetopt_args_info_purpose);
	size_t len_usage = strlen(gengetopt_args_info_usage);

	if (len_usage > 0) {
		printf("%s\n", gengetopt_args_info_usage);
	}
	if (len_purpose > 0) {
		printf("%s\n", gengetopt_args_info_purpose);
	}

	if (len_usage || len_purpose) {
		printf("\n");
	}

	if (strlen(gengetopt_args_info_description) > 0) {
		printf("%s\n\n", gengetopt_args_info_description);
	}
}

void
cmdline_parser_print_help (void)
{
  int i = 0;
  print_help_common();
  while (gengetopt_args_info_help[i])
    printf("%s\n", gengetopt_args_info_help[i++]);
}

void
cmdline_parser_init (struct gengetopt_args_info *args_info)
{
  clear_given (args_info);
  clear_args (args_info);
  init_args_info (args_info);

  args_info->inputs = 0;
  args_info->inputs_num = 0;
}

void
cmdline_parser_params_init(struct cmdline_parser_params *params)
{
  if (params)
    { 
      params->override = 0;
      params->initialize = 1;
      params->check_required = 1;
      params->check_ambiguity = 0;
      params->print_errors = 1;
    }
}

struct cmdline_parser_params *
cmdline_parser_params_create(void)
{
  struct cmdline_parser_params *params = 
    (struct cmdline_parser_params *)malloc(sizeof(struct cmdline_parser_params));
  cmdline_parser_params_init(params);  
  return params;
}

static void
free_string_field (char **s)
{
  if (*s)
    {
      free (*s);
      *s = 0;
    }
}


static void
cmdline_parser_release (struct gengetopt_args_info *args_info)
{
  unsigned int i;
  free_string_field (&(args_info->mode_orig));
  free_string_field (&(args_info->mismatches_orig));
  free_string_field (&(args_info->subkey_orig));
  
  
  for (i = 0; i < args_info->inputs_num; ++i)
    free (args_info->inputs [i]);

  if (args_info->inputs_num)
    free (args_info->inputs);

  clear_given (args_info);
}

/**
 * @param val the value to check
 * @param values the possible values
 * @return the index of the matched value:
 * -1 if no value matched,
 * -2 if more than one value has matched
 */
static int
check_possible_values(const char *val, const char *values[])
{
  int i, found, last;
  size_t len;

  if (!val)   /* otherwise strlen() crashes below */
    return -1; /* -1 means no argument for the option */

  found = last = 0;

  for (i = 0, len = strlen(val); values[i]; ++i)
    {
      if (strncmp(val, values[i], len) == 0)
        {
          ++found;
          last = i;
          if (strlen(values[i]) == len)
            return i; /* exact macth no need to check more */
        }
    }

  if (found == 1) /* one match: OK */
    return last;

  return (found ? -2 : -1); /* return many values or none matched */
}


static void
write_into_file(FILE *outfile, const char *opt, const char *arg, const char *values[])
{
  int found = -1;
  if (arg) {
    if (values) {
      found = check_possible_values(arg, values);      
    }
    if (found >= 0)
      fprintf(outfile, "%s=\"%s\" # %s\n", opt, arg, values[found]);
    else
      fprintf(outfile, "%s=\"%s\"\n", opt, arg);
  } else {
    fprintf(outfile, "%s\n", opt);
  }
}


int
cmdline_parser_dump(FILE *outfile, struct gengetopt_args_info *args_info)
{
  int i = 0;

  if (!outfile)
    {
      fprintf (stderr, "%s: cannot dump options to stream\n", CMDLINE_PARSER_PACKAGE);
      return EXIT_FAILURE;
    }

  if (args_info->help_given)
    write_into_file(outfile, "help", 0, 0 );
  if (args_info->version_given)
    write_into_file(outfile, "version", 0, 0 );
  if (args_info->usage_given)
    write_into_file(outfile, "usage", 0, 0 );
  if (args_info->mode_given)
    write_into_file(outfile, "mode", args_info->mode_orig, cmdline_parser_mode_values);
  if (args_info->mismatches_given)
    write_into_file(outfile, "mismatches", args_info->mismatches_orig, 0);
  if (args_info->subkey_given)
    write_into_file(outfile, "subkey", args_info->subkey_orig, 0);
  if (args_info->random_given)
    write_into_file(outfile, "random", 0, 0 );
  if (args_info->benchmark_given)
    write_into_file(outfile, "benchmark", 0, 0 );
  if (args_info->all_given)
    write_into_file(outfile, "all", 0, 0 );
  if (args_info->count_given)
    write_into_file(outfile, "count", 0, 0 );
  if (args_info->fixed_given)
    write_into_file(outfile, "fixed", 0, 0 );
  if (args_info->verbose_given)
    write_into_file(outfile, "verbose", 0, 0 );
  

  i = EXIT_SUCCESS;
  return i;
}

int
cmdline_parser_file_save(const char *filename, struct gengetopt_args_info *args_info)
{
  FILE *outfile;
  int i = 0;

  outfile = fopen(filename, "w");

  if (!outfile)
    {
      fprintf (stderr, "%s: cannot open file for writing: %s\n", CMDLINE_PARSER_PACKAGE, filename);
      return EXIT_FAILURE;
    }

  i = cmdline_parser_dump(outfile, args_info);
  fclose (outfile);

  return i;
}

void
cmdline_parser_free (struct gengetopt_args_info *args_info)
{
  cmdline_parser_release (args_info);
}

/** @brief replacement of strdup, which is not standard */
char *
gengetopt_strdup (const char *s)
{
  char *result = 0;
  if (!s)
    return result;

  result = (char*)malloc(strlen(s) + 1);
  if (result == (char*)0)
    return (char*)0;
  strcpy(result, s);
  return result;
}

int
cmdline_parser (int argc, char **argv, struct gengetopt_args_info *args_info)
{
  return cmdline_parser2 (argc, argv, args_info, 0, 1, 1);
}

int
cmdline_parser_ext (int argc, char **argv, struct gengetopt_args_info *args_info,
                   struct cmdline_parser_params *params)
{
  int result;
  result = cmdline_parser_internal (argc, argv, args_info, params, 0);

  return result;
}

int
cmdline_parser2 (int argc, char **argv, struct gengetopt_args_info *args_info, int override, int initialize, int check_required)
{
  int result;
  struct cmdline_parser_params params;
  
  params.override = override;
  params.initialize = initialize;
  params.check_required = check_required;
  params.check_ambiguity = 0;
  params.print_errors = 1;

  result = cmdline_parser_internal (argc, argv, args_info, &params, 0);

  return result;
}

int
cmdline_parser_required (struct gengetopt_args_info *args_info, const char *prog_name)
{
  FIX_UNUSED (args_info);
  FIX_UNUSED (prog_name);
  return EXIT_SUCCESS;
}


static char *package_name = 0;

/**
 * @brief updates an option
 * @param field the generic pointer to the field to update
 * @param orig_field the pointer to the orig field
 * @param field_given the pointer to the number of occurrence of this option
 * @param prev_given the pointer to the number of occurrence already seen
 * @param value the argument for this option (if null no arg was specified)
 * @param possible_values the possible values for this option (if specified)
 * @param default_value the default value (in case the option only accepts fixed values)
 * @param arg_type the type of this option
 * @param check_ambiguity @see cmdline_parser_params.check_ambiguity
 * @param override @see cmdline_parser_params.override
 * @param no_free whether to free a possible previous value
 * @param multiple_option whether this is a multiple option
 * @param long_opt the corresponding long option
 * @param short_opt the corresponding short option (or '-' if none)
 * @param additional_error possible further error specification
 */
static
int update_arg(void *field, char **orig_field,
               unsigned int *field_given, unsigned int *prev_given, 
               char *value, const char *possible_values[],
               const char *default_value,
               cmdline_parser_arg_type arg_type,
               int check_ambiguity, int override,
               int no_free, int multiple_option,
               const char *long_opt, char short_opt,
               const char *additional_error)
{
  char *stop_char = 0;
  const char *val = value;
  int found;
  FIX_UNUSED (field);

  stop_char = 0;
  found = 0;

  if (!multiple_option && prev_given && (*prev_given || (check_ambiguity && *field_given)))
    {
      if (short_opt != '-')
        fprintf (stderr, "%s: `--%s' (`-%c') option given more than once%s\n", 
               package_name, long_opt, short_opt,
               (additional_error ? additional_error : ""));
      else
        fprintf (stderr, "%s: `--%s' option given more than once%s\n", 
               package_name, long_opt,
               (additional_error ? additional_error : ""));
      return 1; /* failure */
    }

  if (possible_values && (found = check_possible_values((value ? value : default_value), possible_values)) < 0)
    {
      if (short_opt != '-')
        fprintf (stderr, "%s: %s argument, \"%s\", for option `--%s' (`-%c')%s\n", 
          package_name, (found == -2) ? "ambiguous" : "invalid", value, long_opt, short_opt,
          (additional_error ? additional_error : ""));
      else
        fprintf (stderr, "%s: %s argument, \"%s\", for option `--%s'%s\n", 
          package_name, (found == -2) ? "ambiguous" : "invalid", value, long_opt,
          (additional_error ? additional_error : ""));
      return 1; /* failure */
    }
    
  if (field_given && *field_given && ! override)
    return 0;
  if (prev_given)
    (*prev_given)++;
  if (field_given)
    (*field_given)++;
  if (possible_values)
    val = possible_values[found];

  switch(arg_type) {
  case ARG_FLAG:
    *((int *)field) = !*((int *)field);
    break;
  case ARG_INT:
    if (val) *((int *)field) = strtol (val, &stop_char, 0);
    break;
  case ARG_ENUM:
    if (val) *((int *)field) = found;
    break;
  default:
    break;
  };

  /* check numeric conversion */
  switch(arg_type) {
  case ARG_INT:
    if (val && !(stop_char && *stop_char == '\0')) {
      fprintf(stderr, "%s: invalid numeric value: %s\n", package_name, val);
      return 1; /* failure */
    }
    break;
  default:
    ;
  };

  /* store the original value */
  switch(arg_type) {
  case ARG_NO:
  case ARG_FLAG:
    break;
  default:
    if (value && orig_field) {
      if (no_free) {
        *orig_field = value;
      } else {
        if (*orig_field)
          free (*orig_field); /* free previous string */
        *orig_field = gengetopt_strdup (value);
      }
    }
  };

  return 0; /* OK */
}


static int check_modes(
  int given1[], const char *options1[],
                       int given2[], const char *options2[])
{
  int i = 0, j = 0, errors = 0;
  
  while (given1[i] >= 0) {
    if (given1[i]) {
      while (given2[j] >= 0) {
        if (given2[j]) {
          ++errors;
          fprintf(stderr, "%s: option %s conflicts with option %s\n",
                  package_name, options1[i], options2[j]);
        }
        ++j;
      }
    }
    ++i;
  }
  
  return errors;
}

int
cmdline_parser_internal (
  int argc, char **argv, struct gengetopt_args_info *args_info,
                        struct cmdline_parser_params *params, const char *additional_error)
{
  int c;	/* Character of the parsed option.  */

  int error_occurred = 0;
  struct gengetopt_args_info local_args_info;
  
  int override;
  int initialize;
  int check_required;
  int check_ambiguity;
  
  package_name = argv[0];
  
  /* TODO: Why is this here? It is not used anywhere. */
  override = params->override;
  FIX_UNUSED(override);

  initialize = params->initialize;
  check_required = params->check_required;

  /* TODO: Why is this here? It is not used anywhere. */
  check_ambiguity = params->check_ambiguity;
  FIX_UNUSED(check_ambiguity);

  if (initialize)
    cmdline_parser_init (args_info);

  cmdline_parser_init (&local_args_info);

  optarg = 0;
  optind = 0;
  opterr = params->print_errors;
  optopt = '?';

  while (1)
    {
      int option_index = 0;

      static struct option long_options[] = {
        { "help",	0, NULL, 'h' },
        { "version",	0, NULL, 'V' },
        { "usage",	0, NULL, 0 },
        { "mode",	1, NULL, 0 },
        { "mismatches",	1, NULL, 'm' },
        { "subkey",	1, NULL, 's' },
        { "random",	0, NULL, 'r' },
        { "benchmark",	0, NULL, 'b' },
        { "all",	0, NULL, 'a' },
        { "count",	0, NULL, 'c' },
        { "fixed",	0, NULL, 'f' },
        { "verbose",	0, NULL, 'v' },
        { 0,  0, 0, 0 }
      };

      c = getopt_long (argc, argv, "hVm:s:rbacfv", long_options, &option_index);

      if (c == -1) break;	/* Exit from `while (1)' loop.  */

      switch (c)
        {
        case 'h':	/* Print help and exit.  */
          cmdline_parser_print_help ();
          cmdline_parser_free (&local_args_info);
          exit (EXIT_SUCCESS);

        case 'V':	/* Print version and exit.  */
          cmdline_parser_print_version ();
          cmdline_parser_free (&local_args_info);
          exit (EXIT_SUCCESS);

        case 'm':	/* The largest # of bits of corruption to test against, inclusively. Defaults to -1. If negative, then the size of key in bits will be the limit. If in random or benchmark mode, then this will also be used to corrupt the random key by the same # of bits; for this reason, it must be set and non-negative when in random or benchmark mode. Cannot be larger than what --subkey-size is set to..  */
        
        
          if (update_arg( (void *)&(args_info->mismatches_arg), 
               &(args_info->mismatches_orig), &(args_info->mismatches_given),
              &(local_args_info.mismatches_given), optarg, 0, "-1", ARG_INT,
              check_ambiguity, override, 0, 0,
              "mismatches", 'm',
              additional_error))
            goto failure;
        
          break;
        case 's':	/* How many of the first bits to corrupt and iterate over. Must be between 1 and 256. Defaults to 256..  */
        
        
          if (update_arg( (void *)&(args_info->subkey_arg), 
               &(args_info->subkey_orig), &(args_info->subkey_given),
              &(local_args_info.subkey_given), optarg, 0, "256", ARG_INT,
              check_ambiguity, override, 0, 0,
              "subkey", 's',
              additional_error))
            goto failure;
        
          break;
        case 'r':	/* Instead of using arguments, randomly generate HOST_SEED and CLIENT_*. This must be accompanied by --mismatches, since it is used to corrupt the random key by the same # of bits. --random and --benchmark cannot be used together..  */
          args_info->Random_mode_counter += 1;
        
        
          if (update_arg((void *)&(args_info->random_flag), 0, &(args_info->random_given),
              &(local_args_info.random_given), optarg, 0, 0, ARG_FLAG,
              check_ambiguity, override, 1, 0, "random", 'r',
              additional_error))
            goto failure;
        
          break;
        case 'b':	/* Instead of using arguments, strategically generate HOST_SEED and CLIENT_*. Specifically, generates a client seed that's always 50% of the way through a rank's workload, but randomly chooses the thread. --random and --benchmark cannot be used together..  */
          args_info->Benchmark_mode_counter += 1;
        
        
          if (update_arg((void *)&(args_info->benchmark_flag), 0, &(args_info->benchmark_given),
              &(local_args_info.benchmark_given), optarg, 0, 0, ARG_FLAG,
              check_ambiguity, override, 1, 0, "benchmark", 'b',
              additional_error))
            goto failure;
        
          break;
        case 'a':	/* Don't cut out early when key is found..  */
        
        
          if (update_arg((void *)&(args_info->all_flag), 0, &(args_info->all_given),
              &(local_args_info.all_given), optarg, 0, 0, ARG_FLAG,
              check_ambiguity, override, 1, 0, "all", 'a',
              additional_error))
            goto failure;
        
          break;
        case 'c':	/* Count the number of keys tested and show it as verbose output..  */
        
        
          if (update_arg((void *)&(args_info->count_flag), 0, &(args_info->count_given),
              &(local_args_info.count_given), optarg, 0, 0, ARG_FLAG,
              check_ambiguity, override, 1, 0, "count", 'c',
              additional_error))
            goto failure;
        
          break;
        case 'f':	/* Only test the given mismatch, instead of progressing from 0 to --mismatches. This is only valid when --mismatches is set and non-negative..  */
        
        
          if (update_arg((void *)&(args_info->fixed_flag), 0, &(args_info->fixed_given),
              &(local_args_info.fixed_given), optarg, 0, 0, ARG_FLAG,
              check_ambiguity, override, 1, 0, "fixed", 'f',
              additional_error))
            goto failure;
        
          break;
        case 'v':	/* Produces verbose output and time taken to stderr..  */
        
        
          if (update_arg((void *)&(args_info->verbose_flag), 0, &(args_info->verbose_given),
              &(local_args_info.verbose_given), optarg, 0, 0, ARG_FLAG,
              check_ambiguity, override, 1, 0, "verbose", 'v',
              additional_error))
            goto failure;
        
          break;

        case 0:	/* Long option with no short option */
          /* Give a short usage message.  */
          if (strcmp (long_options[option_index].name, "usage") == 0)
          {
          
          
            if (update_arg( 0 , 
                 0 , &(args_info->usage_given),
                &(local_args_info.usage_given), optarg, 0, 0, ARG_NO,
                check_ambiguity, override, 0, 0,
                "usage", '-',
                additional_error))
              goto failure;
          
          }
          /* (REQUIRED) The cryptographic function to iterate against. If `none', then only perform
          seed iteration..  */
          else if (strcmp (long_options[option_index].name, "mode") == 0)
          {
          
          
            if (update_arg( (void *)&(args_info->mode_arg), 
                 &(args_info->mode_orig), &(args_info->mode_given),
                &(local_args_info.mode_given), optarg, cmdline_parser_mode_values, 0, ARG_ENUM,
                check_ambiguity, override, 0, 0,
                "mode", '-',
                additional_error))
              goto failure;
          
          }
          
          break;
        case '?':	/* Invalid option.  */
          /* `getopt_long' already printed an error message.  */
          goto failure;

        default:	/* bug: option not considered.  */
          fprintf (stderr, "%s: option unknown: %c%s\n", CMDLINE_PARSER_PACKAGE, c, (additional_error ? additional_error : ""));
          abort ();
        } /* switch */
    } /* while */



  if (args_info->Benchmark_mode_counter && args_info->Random_mode_counter) {
    int Benchmark_given[] = {args_info->benchmark_given,  -1};
    const char *Benchmark_desc[] = {"--benchmark",  0};
    int Random_given[] = {args_info->random_given,  -1};
    const char *Random_desc[] = {"--random",  0};
    error_occurred += check_modes(Benchmark_given, Benchmark_desc, Random_given, Random_desc);
  }
  
	FIX_UNUSED(check_required);

  cmdline_parser_release (&local_args_info);

  if ( error_occurred )
    return (EXIT_FAILURE);

  if (optind < argc)
    {
      int i = 0 ;
      int found_prog_name = 0;
      /* whether program name, i.e., argv[0], is in the remaining args
         (this may happen with some implementations of getopt,
          but surely not with the one included by gengetopt) */

      i = optind;
      while (i < argc)
        if (argv[i++] == argv[0]) {
          found_prog_name = 1;
          break;
        }
      i = 0;

      args_info->inputs_num = argc - optind - found_prog_name;
      args_info->inputs =
        (char **)(malloc ((args_info->inputs_num)*sizeof(char *))) ;
      while (optind < argc)
        if (argv[optind++] != argv[0])
          args_info->inputs[ i++ ] = gengetopt_strdup (argv[optind-1]) ;
    }

  return 0;

failure:
  
  cmdline_parser_release (&local_args_info);
  return (EXIT_FAILURE);
}
/* vim: set ft=c noet ts=8 sts=8 sw=8 tw=80 nojs spell : */