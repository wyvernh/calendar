#ifndef __CAL_H
#define __CAL_H

#include <stdbool.h>
#include <calceph.h>

// speed of light in km/s
#define C 299792.458
// number of degrees in one radian
#define DGR_RAD 57.2957795131
// required precision of longitude optimisation (days)
#define OPT_LON 0.001 // < 2 min


typedef struct {
  double f;
  bool err;
} t_double_result;

typedef struct {
  long ip;
  double fp;
  bool err;
} t_tdb;

// By convention, the first term is the negative zenth
typedef struct {
  t_tdb tdb[24];
  bool err;
} t_year_ecliptic_dates;

// Here, the first value is between the negative zenth and negative leventh terms
typedef struct {
  int s_days[24];
  bool err;
} t_year_pattern;

/* year_ecliptic_dates_t year_ecliptic_dates_from(astro_time_t start_time); */

/* year_s_day_pattern_t year_s_day_pattern_from(year_ecliptic_dates_t year); */

/* int write_s_day_pattern(FILE *f, year_s_day_pattern_t year_pattern, char spc[]); */

// Convert decimal integers to chronal
void run_convert(int argc, char **argv);
// Calculate year patterns from ephemeris
void run_ephemeris(int argc, char **argv);
// Generate calendar from year patterns
void run_generate(int argc, char **argv);
// Generate latex table of year patterns
void run_latex(int argc, char **argv);

#endif
