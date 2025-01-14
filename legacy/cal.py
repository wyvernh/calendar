#!/usr/bin/env python3

import astropy.time
import astropy.coordinates
import math
from scipy.optimize import minimize

def get_lon(t):
    time = astropy.time.Time(t, format="jd")
    sun = astropy.coordinates.get_body("sun", time=time)
    frame = astropy.coordinates.GeocentricTrueEcliptic(equinox=time)
    lon = sun.transform_to(frame).lon.deg
    return lon

def optimiseFunction(t_arr, offset):
    t = t_arr[0]
    lon = get_lon(t) - offset
    return math.sin(math.radians(abs(lon)/2))

def optimiseFunction2(t_arr, offset):
    t = t_arr[0]
    lon = get_lon(t) - offset
    return abs(abs(lon) - 180)

def eclipticDate(approx, deg):
    t_solv = minimize(optimiseFunction, [approx], args=(deg), tol=1e-3)
    return t_solv.x[0]

def printNextDates(print_function, approx, start_lon, step, n):
    jd = eclipticDate(approx, start_lon)
    print_function(jd)
    if n:
        printNextDates(print_function, jd + step, ( start_lon + step ) % 360, step, n-1)
    return

def print_jd(jd):
    print(jd, " ", get_lon(jd))
    return

def print_greg(jd):
    t = astropy.time.Time(jd, format='jd')
    utc = t.to_datetime()
    print(utc, get_lon(jd))
    return

def mjd(jd):
    return jd - 2400000.5

def print_both(jd):
    print(mjd(jd), " -- ", end="")
    print_greg(jd)
    return

def calculate_year_from(jd0):
    s_darray = []
    print_both(jd0)
    for i in range(1,25):
        jd1 = eclipticDate(jd0, (270 + i * 15) % 360 )
        print_both(jd1)
        diff = math.floor(mjd(jd1)) - math.floor(mjd(jd0))
        jd0 = jd1
        s_darray.append(diff - 15)
        #print(diff - 15)
    print(s_darray)
    sum = 0
    for i in s_darray:
        sum += i + 15
    print(sum)
    print(jd0)
    print(eclipticDate(jd0 + 365, 270))
    return s_darray

def print_sd(s_darray):
    for i in [0, 6, 12, 18]:
        print(s_darray[i:i+6])
    return

def print_latex_sd(s_darray):
    print("  \\begin{bmatrix}")
    for i in range(4):
        print("    ", end="")
        for j in range(5):
            print(s_darray[6*i + j], end=" & ")
        print(s_darray[6*i + 5], "\\\\")
    print("  \end{bmatrix}")
    return

def print_latex_years(solstice, year, n):
    print("\\begin{align*}")
    for i in range(n):
        #print(solstice)
        s_darray = calculate_year_from(solstice)
        print_latex_sd(s_darray)
        print("  \\atop", year, "\,\, (1\\bar400)", end = " ")
        if i % 4 == 3:
            print("\\\\")
        else:
            print("&")
        solstice = eclipticDate(solstice + 365, 270)
        year += 1
    print("\end{align*}")
    return

def print_latex_years_2(solstice, year, n):
    print("  \\hline")
    for i in range(n):
        print(" ", year, "& $1\\bar400$", end=" & ")
        s_darray = calculate_year_from(solstice)
        for element in s_darray[0:23]:
            print(element, end=" & ")
        print(s_darray[23], "\\\\\n  \\hline")
        solstice = eclipticDate(solstice + 365, 270)
        year += 1
    return

def calc_years(solstice, year, n):
    years = []
    solstice = eclipticDate(solstice, 270)
    for i in range(n):
        print("\n", year)
        s_darray = calculate_year_from(solstice)
        years.append(s_darray)
        solstice = eclipticDate(solstice + 365, 270)
        year += 1
        print(s_darray)
    print("\n")
    return years

def verify_year(year):
    for code in year:
        if code != -1 and code != 0 and code != 1:
            return "false"
    return "true"

def beg_month_append(year_dates, julian, year_name, month_name):
    for i in range(6, 0, -1):
        year_dates.append(str(julian) + "," + year_name + "." + month_name + "b" + str(i))
        year_dates.append(str(julian + 0.5) + "," + year_name + "." + month_name + "b" + str(i))
        julian += 1
    return julian

def mid_month_append(year_dates, julian, year_name, month_name):
    year_dates.append(str(julian) + "," + year_name + "." + month_name + "b0")
    year_dates.append(str(julian + 0.5) + "," + year_name + "." + month_name + "0")
    return julian + 1

def end_month_append(year_dates, code, julian, year_name, month_name, next_month_name):
    for i in range(1, 7):
        year_dates.append(str(julian) + "," + year_name + "." + month_name + str(i))
        year_dates.append(str(julian + 0.5) + "," + year_name + "." + month_name + str(i))
        julian += 1
    if code == -1:
        year_dates.append(str(julian) + "," + year_name + "." + month_name + "7")
        year_dates.append(str(julian + 0.5) + "," + year_name + "." + next_month_name + "b7")
    elif code == 0:
        year_dates.append(str(julian) + "," + year_name + "." + month_name + "7")
        year_dates.append(str(julian + 0.5) + "," + year_name + "." + month_name + "7")
        year_dates.append(str(julian + 1.0) + "," + year_name + "." + next_month_name + "b7")
        year_dates.append(str(julian + 1.5) + "," + year_name + "." + next_month_name + "b7")
        julian += 1
    elif code == 1:
        year_dates.append(str(julian) + "," + year_name + "." + month_name + "7")
        year_dates.append(str(julian + 0.5) + "," + year_name + "." + month_name + "7")
        year_dates.append(str(julian + 1.0) + "," + year_name + "." + month_name + "8")
        year_dates.append(str(julian + 1.5) + "," + year_name + "." + next_month_name + "b8")
        year_dates.append(str(julian + 2.0) + "," + year_name + "." + next_month_name + "b7")
        year_dates.append(str(julian + 2.5) + "," + year_name + "." + next_month_name + "b7")
        julian += 2
    return julian + 1

def calc_year_dates(year, year_name, second_year_name, julian):
    julian = round(julian) + 0.0
    year_dates = []
    year_dates.append(str(julian) + "," + year_name + ".bC0")
    julian += 0.5
    julian = end_month_append(year_dates, year[0], julian, year_name, "bC", "bB")

    m1 = ["bB", "bA", "b9", "b8", "b7", "b6", "b5", "b4", "b3", "b2", "b1", "b0"]
    m2 = ["1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C"]

    for m in range(11):
        julian = beg_month_append(year_dates, julian, year_name, m1[m])
        julian = mid_month_append(year_dates, julian, year_name, m1[m])
        julian = end_month_append(year_dates, year[m+1], julian, year_name, m1[m], m1[m+1])

    julian = beg_month_append(year_dates, julian, year_name, "b0")
    year_dates.append(str(julian) + "," + year_name + ".b0b0")
    year_dates.append(str(julian + 0.5) + "," + second_year_name + ".00")
    julian += 1
    julian = end_month_append(year_dates, year[12], julian, second_year_name, "0", "1")

    for m in range(11):
        julian = beg_month_append(year_dates, julian, second_year_name, m2[m])
        julian = mid_month_append(year_dates, julian, second_year_name, m2[m])
        julian = end_month_append(year_dates, year[m+13], julian, second_year_name, m2[m], m2[m+1])

    julian = beg_month_append(year_dates, julian, second_year_name, "C")
    year_dates.append(str(julian) + "," + second_year_name + ".Cb0")

    return year_dates

def file_write_year_dates(filename, year, year_name, second_year_name, julian):
    if verify_year == "false":
        return "true"
    year_dates = calc_year_dates(year, year_name, second_year_name, julian)
    f = open(filename, "a")
    for date in year_dates:
        f.write(date)
        f.write("\n")
    f.close()
    return "true"

def inc_two_year_digits(chron1, chron2, n):
    match chron1[n]:
        case "bC":
            chron1[n] = "bB"
            chron2[n] = "bB"
        case "bB":
            chron1[n] = "bA"
            chron2[n] = "bA"
        case "bA":
            chron1[n] = "b9"
            chron2[n] = "b9"
        case "b9":
            chron1[n] = "b8"
            chron2[n] = "b8"
        case "b8":
            chron1[n] = "b7"
            chron2[n] = "b7"
        case "b7":
            chron1[n] = "b6"
            chron2[n] = "b6"
        case "b6":
            chron1[n] = "b5"
            chron2[n] = "b5"
        case "b5":
            chron1[n] = "b4"
            chron2[n] = "b4"
        case "b4":
            chron1[n] = "b3"
            chron2[n] = "b3"
        case "b3":
            chron1[n] = "b2"
            chron2[n] = "b2"
        case "b2":
            chron1[n] = "b1"
            chron2[n] = "b1"
        case "b1":
            chron1[n] = "b0"
            chron2[n] = "0"
            if chron2[n-1] == "C":
                chron2[n-1] = "bC"
                if chron2[n-2] == "B":
                    chron2[n-2] = "C"
                elif chron2[n-2] == "b1":
                    chron2[n-2] = "b0"
                elif chron2[n-2] == "0":
                    chron2[n-2] = "1"
                else:
                    inc_two_year_digits(chron2, chron2, n-2)
            if chron2[n-1] == "b0":
                chron2[n-1] = "0"
                if chron2[n-2] == "C":
                    chron2[n-2] = "bC"
                    if chron2[n-3] == "B":
                        chron2[n-3] = "C"
                    elif chron2[n-3] == "b1":
                        chron2[n-3] = "b0"
                    elif chron2[n-3] == "0":
                        chron2[n-3] = "1"
                    else:
                        inc_two_year_digits(chron2, chron2, n-3)
                if chron2[n-2] == "b0":
                    chron2[n-2] = "0"
                    if chron2[n-3] == "b0":
                        chron2[n-3] = "0"
        case "b0":
            chron1[n] = "1"
            chron2[n] = "1"
            for i in range(n):
                chron1[i] = chron2[i]
        case "1":
            chron1[n] = "2"
            chron2[n] = "2"
        case "2":
            chron1[n] = "3"
            chron2[n] = "3"
        case "3":
            chron1[n] = "4"
            chron2[n] = "4"
        case "4":
            chron1[n] = "5"
            chron2[n] = "5"
        case "5":
            chron1[n] = "6"
            chron2[n] = "6"
        case "6":
            chron1[n] = "7"
            chron2[n] = "7"
        case "7":
            chron1[n] = "8"
            chron2[n] = "8"
        case "8":
            chron1[n] = "9"
            chron2[n] = "9"
        case "9":
            chron1[n] = "A"
            chron2[n] = "A"
        case "A":
            chron1[n] = "B"
            chron2[n] = "B"
        case "B":
            chron1[n] = "C"
            chron2[n] = "bC"
            if chron2[n-1] == "B":
                chron2[n-1] = "C"
            elif chron2[n-1] == "b1":
                chron2[n-1] = "b0"
            elif chron2[n-1] == "0":
                chron2[n-1] = "1"
            else:
                inc_two_year_digits(chron2, chron2, n-1)
        case "C":
            chron1[n] = "bB"
            chron2[n] = "bB"
            if chron1[n-1] == "B":
                chron1[n-1] = "C"
            elif chron1[n-1] == "b1":
                chron1[n-1] = "b0"
            elif chron1[n-1] == "0":
                chron1[n-1] = "1"
            else:
                inc_two_year_digits(chron1, chron1, n-1)

def inc_year_name(year_name_arr1, year_name_arr2):
    inc_two_year_digits(year_name_arr1, year_name_arr2, 3)

def file_write_n_year_dates(filename, n, year_name_arr1, year_name_arr2, julian):
    for i in range(n):
        year = calculate_year_from(julian)
        year_name1 = year_name_arr1[0] + year_name_arr1[1] + year_name_arr1[2] + year_name_arr1[3]
        year_name2 = year_name_arr2[0] + year_name_arr2[1] + year_name_arr2[2] + year_name_arr2[3]
        if file_write_year_dates(filename, year, year_name1, year_name2, julian) == "false":
            return
        inc_year_name(year_name_arr1, year_name_arr2)
        julian = eclipticDate(julian + 365, 270)

def file_write_years(years, filename):
    f = open(filename, "a")
    for year in years:
        for term in year:
            if term == 0 or term == 1:
                f.write(str(term))
            else:
                if term == -1:
                    f.write("P")
                else:
                    f.close()
                    return "error"
        f.write("\n")
    f.close()
    return "written"


def matchchron(chron, n):
    match chron[n]:
        case "\\bar{C}":
            chron[n] = "\\bar{B}"
        case "\\bar{B}":
            chron[n] = "\\bar{A}"
        case "\\bar{A}":
            chron[n] = "\\bar9"
        case "\\bar9":
            chron[n] = "\\bar8"
        case "\\bar8":
            chron[n] = "\\bar7"
        case "\\bar7":
            chron[n] = "\\bar6"
        case "\\bar6":
            chron[n] = "\\bar5"
        case "\\bar5":
            chron[n] = "\\bar4"
        case "\\bar4":
            chron[n] = "\\bar3"
        case "\\bar3":
            chron[n] = "\\bar2"
        case "\\bar2":
            chron[n] = "\\bar1"
        case "\\bar1":
            chron[n] = "0"
            if n != 3:
                if chron[n+1][0] == "\\":
                    chron[n] = "\\bar0"
            if n != 0:
                if chron[n-1] == "\\bar0":
                    chron[n-1] == "0"
                elif chron[n-1] == "C":
                    chron[n-1] == "\\bar{C}"
                    matchchron(chron, n-2)
        case "0":
            chron[n] = "1"
        case "1":
            chron[n] = "2"
        case "2":
            chron[n] = "3"
        case "3":
            chron[n] = "4"
        case "4":
            chron[n] = "5"
        case "5":
            chron[n] = "6"
        case "6":
            chron[n] = "7"
        case "7":
            chron[n] = "8"
        case "8":
            chron[n] = "9"
        case "9":
            chron[n] = "A"
        case "A":
            chron[n] = "B"
        case "B":
            if n == 3:
                chron[n] = "\\bar{C}"
                matchchron(chron, n-1)
            else:
                chron[n] = "C"


def print_latex_table_of(years):
    startyear = 1957
    chron = [ "1", "\\bar4", "0", "0" ]
    for s_darray in years:
        print(" ", startyear, "& $" + chron[0] + chron[1] + chron[2] + chron[3] + "$", end=" & ")
        for element in s_darray[0:23]:
            print(element, end=" & ")
        print(s_darray[23], "\\\\\n  \\hline")
        startyear += 1
        # increment chron
        matchchron(chron, 3)
    return
