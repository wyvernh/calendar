#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <calceph.h>
#include "cal.h"
#include <sofa.h>

t_double_result double_error() {
  t_double_result err = { 0, true };
  return err;
}
t_double_result double_result_from(double d) {
  t_double_result res = { d, false };
  return res;
}

t_tdb tdb_error() {
  t_tdb err = { 0, 0, true };
  return err;
}
t_tdb tdb_from(long ip, double fp) {
  t_tdb res = { ip, fp, false };
  return res;
}

double tdb_diff(t_tdb tdb1, t_tdb tdb2) {
  return tdb1.ip - tdb2.ip + tdb1.fp - tdb2.fp;
}
long day_of(t_tdb date) {
  return date.ip + floor(date.fp + 0.5);
}

double sq(double f) { return f * f; }
double dot(double u[3], double v[3]) {
  return (u[0] * v[0]) + (u[1] * v[1]) + (u[2] * v[2]);
}
double norm(double v[3]) {
  return sqrt(dot(v, v));
}
double proj(double u[3], double v[3], double out[3]) {
  double scalar = dot(u, v) / dot(v, v);
  for (int i = 0; i < 3; i++) out[i] = scalar * v[i];
}
double vscale(double v[3], double s, double out[3]) {
  for (int i = 0; i < 3; i++) out[i] = v[i] * s;
}
double vsum(double u[3], double v[3], double out[3]) {
  for (int i = 0; i < 3; i++) out[i] = u[i] + v[i];
}
double vdiff(double u[3], double v[3], double out[3]) {
  for (int i = 0; i < 3; i++) out[i] = u[i] - v[i];
}

// PV is in km, and km/s
void light_time_correction(double PV[], double dist, int n) {
  double time = dist / C;
  double newpos[3];
  for (int i = 0; i < 3; i++) {
    newpos[i] = PV[i] - PV[i+3] * time;
  }
  if (n <= 0) {
    for (int i = 0; i < 3; i++) PV[i] = newpos[i];
  } else {
    light_time_correction(PV, norm(newpos), n-1);
  }
}

void aberration_correction(double sun_pos[], double earth_vel[]) {
  double photon_x[3];
  double photon_y[3];
  proj(sun_pos, earth_vel, photon_x);
  vdiff(sun_pos, photon_x, photon_y);
  double norm_photon = norm(sun_pos);
  double normed_vel = norm(earth_vel) / C;
  // special relativistic correction for earth's velocity
  double inv_gamma_sq = 1 - normed_vel * normed_vel;
  double norm_photon_x = norm(photon_x);
  double norm_photon_y = norm(photon_y);
  double photon_sin_sq = norm_photon_y * norm_photon_y / (norm_photon * norm_photon);
  double photon_cos = norm_photon_x / norm_photon;
  double photon_tan = norm_photon_y / norm_photon_x;
  double new_photon_tan_sq = photon_sin_sq * inv_gamma_sq / sq(normed_vel + photon_cos);
  double new_photon_x = norm_photon / sqrt(1 + new_photon_tan_sq);
  double new_photon_y = norm_photon * sqrt(1 - 1 / (1 + new_photon_tan_sq));
  vscale(photon_x, new_photon_x / norm_photon_x, photon_x);
  vscale(photon_y, new_photon_y / norm_photon_y, photon_y);
  vsum(photon_x, photon_y, sun_pos);
}

double *icrs_to_ecliptic(double p[], double tt1, double tt2) {
  double BP[3][3];
  double BPN[3][3];
  iauPmat06(tt1, tt2, BP);
  iauNum06a(tt1, tt2, BPN);
  iauRxr(BPN, BP, BPN);
  double obliquity = iauObl06(tt1, tt2);
  double nut_lon;
  double nut_obl;
  iauNut06a(tt1, tt2, &nut_lon, &nut_obl);
  double true_obliquity = obliquity + nut_obl;
  iauRx(true_obliquity, BPN);
  iauRxp(BPN, p, p);
  return p;
}

t_double_result solar_longitude(t_calcephbin *eph_fp, t_tdb tdb) {
  if (tdb.err) return double_error();
  double icrs_sun[6];
  double bcrs_earth[6];
  int units = CALCEPH_UNIT_KM + CALCEPH_UNIT_SEC;
  int result1 = calceph_compute_unit(eph_fp, tdb.ip, tdb.fp, 11, 3, units, icrs_sun);
  int result2 = calceph_compute_unit(eph_fp, tdb.ip, tdb.fp, 3, 12, units, bcrs_earth);
  if (result1 == 0 || result2 == 0) {
    return double_error();
  }

  // correct for planetary aberration
  light_time_correction(icrs_sun, norm(icrs_sun), 1);
  aberration_correction(icrs_sun, bcrs_earth + 3);

  // convert from icrs to ecliptic coordinates
  // tdb stays within 1.7us of tt, which is within the
  // error of the iau 2000a precession-nutation model
  double *p_ecl = icrs_to_ecliptic(icrs_sun, tdb.ip, tdb.fp);

  // calculate longitude
  double theta, pi, r;
  iauP2s(p_ecl, &theta, &pi, &r);
  return double_result_from(theta * DGR_RAD);
}

t_tdb tdb_balance(t_tdb tdb) {
  int weight = floor(tdb.fp);
  tdb.ip += weight;
  tdb.fp -= weight;
}

bool is_good_enough(double diff, t_tdb tdb) {
  if (fabs(diff) <= OPT_LON) return true;
  double tdb_fp = tdb.fp - floor(tdb.fp);
  bool direction = ((diff < 0 && tdb_fp < 0.5) || (diff > 0 && tdb_fp >= 0.5));
  return fabs(diff) <= 0.1 && direction;
}

t_tdb ecliptic_date(t_calcephbin *eph_fp, t_tdb start_tdb, double target) {
  if (start_tdb.err) return tdb_error();
  t_tdb tdb = start_tdb;
  t_tdb prev_tdb = { };
  double prev_diff = 0;
  double slope = -1;
  for (int i = 0; true; i++) {
    if (i > 3099) {
      printf("Took too long to optimise\n");
      return tdb_error();
    }

    t_double_result lon = solar_longitude(eph_fp, tdb);
    if (lon.err) return tdb_error();
    double diff = target - lon.f;
    if (diff > 180) diff = -target - lon.f;
    if (diff < -180) diff = target + lon.f;

    printf("(%f, %f)\n", tdb.ip + tdb.fp, diff);

    if (is_good_enough(diff, tdb)) break;

    if (prev_diff != 0.0) {
      slope = (diff - prev_diff) / tdb_diff(tdb, prev_tdb);
    }

    prev_diff = diff;
    prev_tdb = tdb;
    tdb.fp -= diff / slope;
  }
  return tdb;
}

t_year_ecliptic_dates year_ecliptic_dates_from(t_calcephbin *eph_fp, t_tdb start_tdb) {
  t_year_ecliptic_dates dates = { };
  start_tdb = tdb_balance(start_tdb);
  int target_lon = -90;
  for (int i = 0; i < 24; i++) {
    dates.tdb[i] = ecliptic_date(eph_fp, start_tdb, target_lon);
    // printf("time: %f (%ld)\n", dates.tdb[i].ip + dates.tdb[i].fp, day_of(dates.tdb[i]));
    if (dates.tdb[i].err) {
      dates.err = true;
      return dates;
    }
    target_lon += 15;
    if (target_lon > 180) target_lon -= 360;
    start_tdb.ip = dates.tdb[i].ip + 15;
    start_tdb.fp = dates.tdb[i].fp;
  }
  dates.err = false;
  return dates;
}

t_tdb reverse_years(t_calcephbin *eph_fp, t_tdb start_tdb, long years) {
  if (years <= 0) return start_tdb;
  start_tdb.ip -= 365;
  t_tdb prev = ecliptic_date(eph_fp, start_tdb, -90);
  return reverse_years(eph_fp, prev, years - 1);
}

t_year_pattern year_pattern_from(t_year_ecliptic_dates year_dates, t_tdb next_solstice) {
  t_year_pattern pattern = { };
  if (year_dates.err) {
    pattern.err = true;
    return pattern;
  }
  long cur = day_of(year_dates.tdb[0]);
  for (int i = 0; i < 23; i++) {
    long next = day_of(year_dates.tdb[i + 1]);
    pattern.s_days[i] = next - cur - 15;
    cur = next;
  }
  pattern.s_days[23] = day_of(next_solstice) - cur - 15;
  pattern.err = false;
  return pattern;
}

void write_year_pattern(FILE *fp, t_year_pattern year_pattern, long year) {
  fprintf(fp, "%ld", year);
  for (int i = 0; i < 24; i++) {
    fprintf(fp, " %d", year_pattern.s_days[i]);
  }
  fprintf(fp, "\n");
}

char chronal_digit(int d) {
  switch (d) {
  case 0: return '0';
  case 1: return '1';
  case 2: return '2';
  case 3: return '3';
  case 4: return '4';
  case 5: return '5';
  case 6: return '6';
  case 7: return '7';
  case 8: return '8';
  case 9: return '9';
  case 10: return 'a';
  case 11: return 'b';
  case 12: return 'c';
  }
}

char *long_to_chronal_recurse(char *buf, long n, bool neg);

char *add_chronal_digit(char *buf, long n, bool neg) {
  int d = n % 24;
  if (d > 12 || (d == 12 && !neg) || (d == 0 && neg)) {
    if (d != 0) d = 24 - d;
    *buf = 'n';
    buf++;
    n += d;
    *buf = chronal_digit(d);
    return long_to_chronal_recurse(buf + 1, n / 24, true);
  } else {
    n -= d;
    *buf = chronal_digit(d);
    return long_to_chronal_recurse(buf + 1, n / 24, false);
  }
}

char *long_to_chronal_recurse(char *buf, long n, bool neg) {
  if (n == 0) return buf;
  return add_chronal_digit(buf, n, neg);
}

char *add_unicode_chronal_digit(char *buf, char c) {
  switch (c) {
  case 'a':
    memcpy(buf, "\u218a", 3);
    return buf + 3;
  case 'b':
    memcpy(buf, "\u218b", 3);
    return buf + 3;
  case 'c':
    memcpy(buf, "\U0001f718", 4);
    return buf + 4;
  case 'n':
    memcpy(buf, "\u0305", 2);
    return buf + 2;
  default:
    *buf = c;
    return buf + 1;
  }
}

void long_to_chronal(char *buf, long n) {
  char newbuf[100];
  char *newbuf_end = add_chronal_digit(newbuf, n, n < 0);
  while (newbuf_end > newbuf) {
    newbuf_end--;
    buf = add_unicode_chronal_digit(buf, *newbuf_end);
  }
  *buf = '\0';
}

void long_to_chronal_latex(char *buf, long n) {
  char *old_buf = buf;
  char newbuf[100];
  char *newbuf_end = add_chronal_digit(newbuf, n, n < 0);
  while (newbuf_end >= newbuf) {
    newbuf_end--;
    if (*newbuf_end == 'n') {
      char d[4];
      for (int i = 0; i < buf - old_buf; i++) {
        d[i] = old_buf[i];
      }
      memcpy(old_buf, "\\bar", 4);
      buf += 4;
      memcpy(old_buf + 4, d, 4);
      continue;
    }
    old_buf = buf;
    buf = add_unicode_chronal_digit(buf, *newbuf_end);
  }
  *buf = '\0';
}

char *find_radix(char *buf) {
  while (*buf != '\0') {
    if (*buf == '.') return buf;
    buf++;
  }
  return NULL;
}

t_tdb strtotdb(char *s) {
  char *s2 = find_radix(s);
  if (s2) {
    double fp = strtod(s2, NULL);
    *s2 = '\n';
    long ip = strtol(s, NULL, 10);
    return tdb_from(ip, fp);
  } else {
    long ip = strtol(s, NULL, 10);
    return tdb_from(ip, 0);
  }
}

void run_convert(int argc, char **argv) {
  char buf[100];
  long n = strtol(argv[2], NULL, 10);
  long_to_chronal(buf, n);
  printf("chronal: %s\n", buf);
  long_to_chronal_latex(buf, n);
  printf("latex: $%s$\n", buf);
}

void run_ephemeris(int argc, char **argv) {
  char *ephemeris = argv[2];
  char *output_file = argv[3];
  t_tdb start_tdb = strtotdb(argv[4]);
  long prev_years = strtol(argv[5], NULL, 10);
  long start_year = strtol(argv[6], NULL, 10);
  long n_years = strtol(argv[7], NULL, 10);

  t_calcephbin *eph_fp = calceph_open(ephemeris);
  if (!eph_fp) {
    printf("Could not open ephemeris file: %s\n", ephemeris);
    exit(-2);
  }

  FILE *pat_fp = fopen(output_file, "a");
  if (!pat_fp) {
    printf("Could not open output file: %s", output_file);
    exit(-2);
  }

  calceph_prefetch(eph_fp);

  start_tdb = reverse_years(eph_fp, start_tdb, prev_years);
  t_year_ecliptic_dates year_dates = year_ecliptic_dates_from(eph_fp, start_tdb);
  t_year_ecliptic_dates next_year_dates;
  while (n_years) {
    t_tdb next_solstice = year_dates.tdb[23];
    next_solstice.ip += 15;
    next_year_dates = year_ecliptic_dates_from(eph_fp, next_solstice);
    next_solstice = next_year_dates.tdb[0];
    t_year_pattern year_pattern = year_pattern_from(year_dates, next_solstice);
    write_year_pattern(pat_fp, year_pattern, start_year);

    year_dates = next_year_dates;
    start_year++;
    n_years--;
  }

  fclose(pat_fp);
  calceph_close(eph_fp);
}

void ensure_pattern_file_is_valid(FILE *pat_fp) {
  char buf[100];
  printf("Checking pattern file: ");
  while (fgets(buf, sizeof(buf), pat_fp)) {
    char *token = strtok(buf, " ");
    for (int i = 0; i < 25; i++) {
      if (token == NULL) {
        printf("failed (too few tokens)\n");
        exit(-3);
      }
      long toklon = strtol(token, NULL, 10);
      if (toklon == 0 && token[0] != '0') {
        printf("failed (incorrect number format)\n");
        exit(-3);
      }
      token = strtok(NULL, " ");
    }
  }
  rewind(pat_fp);
  printf("done\n");
}

bool read_year_pattern(FILE *fp, t_year_pattern *pat, long *year) {
  char buf[100];
  if (!fgets(buf, sizeof(buf), fp)) return false;
  char *token = strtok(buf, " ");
  *year = strtol(token, NULL, 10);
  for (int i = 0; i < 24; i++) {
    token = strtok(NULL, " ");
    pat->s_days[i] = strtol(token, NULL, 10);
  }
  pat->err = false;
  return true;
}

void fprint_digit(FILE *fp, unsigned d) {
  switch (d) {
  case 0:
    fprintf(fp, "0");
    break;
  case 1:
    fprintf(fp, "1");
    break;
  case 2:
    fprintf(fp, "2");
    break;
  case 3:
    fprintf(fp, "3");
    break;
  case 4:
    fprintf(fp, "4");
    break;
  case 5:
    fprintf(fp, "5");
    break;
  case 6:
    fprintf(fp, "6");
    break;
  case 7:
    fprintf(fp, "7");
    break;
  case 8:
    fprintf(fp, "8");
    break;
  case 9:
    fprintf(fp, "9");
    break;
  case 10:
    fprintf(fp, "\u218a");
    break;
  case 11:
    fprintf(fp, "\u218b");
    break;
  case 12:
    fprintf(fp, "\U0001f718");
    break;
  }
}

void fprint_date(FILE *fp, char *chronal_year, unsigned term, bool sgn_term, unsigned day, bool sgn_day) {
  fprintf(fp, "%s.", chronal_year);
  fprint_digit(fp, term);
  if (sgn_term) fprintf(fp, "\u0305");
  fprint_digit(fp, day);
  if (sgn_day) fprintf(fp, "\u0305");
  fprintf(fp, "\n");
}

void fprint_term(FILE* fp, char *chronal_year, unsigned term1, unsigned term2, bool sgn_term, int s_days) {
  fprint_date(fp, chronal_year, term1, sgn_term, 0, false);
  int day = 1;
  while (day < 7) {
    fprint_date(fp, chronal_year, term1, sgn_term, day, false);
    fprint_date(fp, chronal_year, term1, sgn_term, day, false);
    day++;
  }
  switch (s_days) {
  case -1:
    fprint_date(fp, chronal_year, term1, sgn_term, day, false);
    fprint_date(fp, chronal_year, term2, sgn_term, day, true);
    day--;
    break;
  case 0:
    fprint_date(fp, chronal_year, term1, sgn_term, day, false);
    fprint_date(fp, chronal_year, term1, sgn_term, day, false);
    break;
  case 1:
    fprint_date(fp, chronal_year, term1, sgn_term, day, false);
    fprint_date(fp, chronal_year, term1, sgn_term, day, false);
    fprint_date(fp, chronal_year, term1, sgn_term, day + 1, false);
    fprint_date(fp, chronal_year, term2, sgn_term, day + 1, true);
    break;
  }
  while (day > 0) {
    fprint_date(fp, chronal_year, term2, sgn_term, day, true);
    fprint_date(fp, chronal_year, term2, sgn_term, day, true);
    day--;
  }
  fprint_date(fp, chronal_year, term2, sgn_term, 0, true);
}

void cal_generate(FILE *pat_fp, FILE *out_fp) {
  char chronal[100];
  t_year_pattern pattern;
  long year;
  while (read_year_pattern(pat_fp, &pattern, &year)) {
    long_to_chronal(chronal, year);

    for (unsigned i = 12; i > 0; i--) {
      fprint_term(out_fp, chronal, i, i - 1, true, pattern.s_days[12 - i]);
    }
    for (unsigned i = 0; i < 12; i++) {
      fprint_term(out_fp, chronal, i, i + 1, false, pattern.s_days[12 + i]);
    }
  }
}

void run_generate(int argc, char **argv) {
  char *pattern_file = argv[2];
  char *output_file = argv[3];

  FILE *pat_fp = fopen(pattern_file, "r");
  if (!pat_fp) {
    printf("Could not open pattern file: %s\n", pattern_file);
    exit(-2);
  }
  FILE *out_fp = fopen(output_file, "a");
  if (!out_fp) {
    printf("Could not open output file: %s\n", output_file);
    exit(-2);
  }

  ensure_pattern_file_is_valid(pat_fp);

  printf("Writing dates to output file: ");
  fflush(stdout);
  cal_generate(pat_fp, out_fp);
  printf("done\n");

  fclose(out_fp);
  fclose(pat_fp);
}

void run_latex(int argc, char **argv) {
  char *pattern_file = argv[2];
  char *output_file = argv[3];
  long start_gregorian = strtol(argv[4], NULL, 10);

  FILE *pat_fp = fopen(pattern_file, "r");
  if (!pat_fp) {
    printf("Could not open pattern file: %s\n", pattern_file);
    exit(-2);
  }
  FILE *out_fp = fopen(output_file, "a");
  if (!out_fp) {
    printf("Could not open output file: %s\n", output_file);
    exit(-2);
  }

  ensure_pattern_file_is_valid(pat_fp);

  char buf[100];
  char latex[100];
  printf("Writing LaTeX to output file: ");

  while (fgets(buf, sizeof(buf), pat_fp)) {
    buf[strlen(buf) - 1] = '\0';
    char *token = strtok(buf, " ");
    long i = strtol(token, NULL, 10);
    long_to_chronal_latex(latex, i);
    fprintf(out_fp, "%ld & $%s$", start_gregorian, latex);

    for (int i = 0; i < 24; i++) {
      token = strtok(NULL, " ");
      fprintf(out_fp, " & %s", token);
    }
    fprintf(out_fp, " \\\\\n");
    fprintf(out_fp, "\\hline\n");

    start_gregorian++;
  }
  printf("done\n");

  fclose(out_fp);
  fclose(pat_fp);
}
