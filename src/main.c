#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cal.h"

#define CONVERT_USAGE "c <DECIMAL_INTEGER>\n"
#define EPHEMERIS_USAGE "e <EPHEMERIS_FILE> <OUTPUT_FILE> <START_TDB> <REWIND_YEARS> <START_YEAR> <N_YEARS>\n"
#define GENERATE_USAGE "g <PATTERN_FILE> <OUTPUT_FILE>\n"
#define LATEX_USAGE "l <PATTERN_FILE> <OUTPUT_FILE> <START_GREGORIAN>\n"

typedef enum {
  CONVERT,
  EPHEMERIS,
  GENERATE,
  LATEX
} t_mode;

void print_usage(int argc, char **argv);
void print_full_usage(int argc, char **argv);
t_mode check_args(int argc, char **argv);
t_mode check_args_mode(int argc, char **argv, t_mode mode);

int main(int argc, char **argv) {
  t_mode mode = check_args(argc, argv);
  switch (mode) {
  case CONVERT:
    run_convert(argc, argv);
    break;
  case EPHEMERIS:
    run_ephemeris(argc, argv);
    break;
  case GENERATE:
    run_generate(argc, argv);
    break;
  case LATEX:
    run_latex(argc, argv);
    break;
  }
  return 0;
}

void print_usage(int argc, char **argv) {
  if (argc) {
    printf("Usage: %s ", argv[0]);
  } else {
    printf("Usage: calgen ");
  }
}

void print_full_usage(int argc, char **argv) {
  print_usage(argc, argv);
  printf(CONVERT_USAGE);
  print_usage(argc, argv);
  printf(EPHEMERIS_USAGE);
  print_usage(argc, argv);
  printf(GENERATE_USAGE);
  print_usage(argc, argv);
  printf(LATEX_USAGE);
}

t_mode check_args(int argc, char **argv) {
  if (argc < 2) {
    print_full_usage(argc, argv);
    exit(-1);
  }
  char *m = argv[1];
  if      (strcmp(m, "c") == 0) return check_args_mode(argc, argv, CONVERT);
  else if (strcmp(m, "e") == 0) return check_args_mode(argc, argv, EPHEMERIS);
  else if (strcmp(m, "g") == 0) return check_args_mode(argc, argv, GENERATE);
  else if (strcmp(m, "l") == 0) return check_args_mode(argc, argv, LATEX);
  print_full_usage(argc, argv);
  exit(-1);
}

t_mode check_args_mode(int argc, char **argv, t_mode mode) {
  switch (mode) {
  case CONVERT:
    if (argc != 3) {
      print_usage(argc, argv);
      printf(CONVERT_USAGE);
      exit(-1);
    }
    break;
  case EPHEMERIS:
    if (argc != 8) {
      print_usage(argc, argv);
      printf(EPHEMERIS_USAGE);
      exit(-1);
    }
    break;
  case GENERATE:
    if (argc != 4) {
      print_usage(argc, argv);
      printf(GENERATE_USAGE);
      exit(-1);
    }
    break;
  case LATEX:
    if (argc != 5) {
      print_usage(argc, argv);
      printf(LATEX_USAGE);
      exit(-1);
    }
    break;
  }
  return mode;
}
