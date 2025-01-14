# Introduction
This calendar uses the balanced base twenty-four number system, with the Pitman (↊, ↋)
digits for ten and eleven, and (for the time being, until augmented system fonts are
developed) the alchemical symbol U{1f718} for the digit twelve. (U{1f718} is an
approximation of the corresponding Argam numeral, created by Micheal de Vlieger of the
Dozenal Society of America). This project is primarily inspired by the Dozenal Solstice
Calendar (with which it shares its epoch and season day distribution concept) and the
traditional Chinese calendar (from which it borrows the fifteen-day solar term, and the
calibration of these terms to the solar longitude as observed by the earth). The calendar
can be reproduced from the following constraints:

* The zeroth hour of the day is the one centred at noon. Times are then described by the
positional balanced base twenty-four division of the day.
* The days of the solar terms (of approximately fifteen days each), are numbered from
negative seven to seven.
* The zeroth day of each solar term is the day, beginning and ending at midnight
Barycentric Dynamical Time (TDB), during which the solar longitude of the sun (as observed
from the centre of the earth) passes through an exact multiple of fifteen degrees. The
0-degree longitude mark is taken to be the traditional March equinox and the longitude
increases as time increases.
* The solar terms themselves are numbered from negative twelve to positive twelve. The
positive and negative twelfth terms are only half terms: the negative twelfth term begins
with the positive half of the zeroth day, and the positive twelfth term ends with the
negative half of the negative zeroth day.
* The traditional June solstice as measured in TDB falls on the zeroth day of the zeroth
solar term.
* The fifteen days of each solar term are split into three five consecutive day pentads,
the zeroth of which contains the zeroth day of the term.
* When the number of days between zero-days (as required by the solar longitude constraint)
is not fifteen, days are added or subtracted between solar terms as necessary. These extra
days are called "season days" or "S-days". Season days are evenly split between the two
terms: the first term takes the negative half of the S-day as day eight and the second term
takes the positive half as its day negative eight. If a day needs to be subtracted, the
seventh and negative seventh days of each term, respectively, are shared. (In the first
1000<sub>c</sub> years of the calendar, there is at most one and at least negative one
season day between consecutive terms.)
* The distribution of season days within the calendar year is completely determined by the
solar longitude in ecliptic coordinates as observed by the centre of the earth, and the
definition of TDB. Due to the eccentricity of the earth's orbit, and the unsolvability of
the n-body problem, this means there is no predictable S-day pattern for any given year.
* The epoch of the calendar is defined as noon TDB of the zeroth day of the zeroth solar
term of the year corresponding to 9564 BCE in the Gregorian calendar. This is the most
recent time that the perihelion occurred on the June solstice, or the zeroth day of the
year in our calendar.
* The zeroth year of the calendar begins on the epoch. No negative term in year zero, nor
any term in any negative year, is required to conform to the definition of the solar terms
given above. This means that there are no season days before the zeroth term of the zeroth
year, and that no exact date is required to be expressible in the calendar if it predates
the epoch. However, in order to keep negative years meaningful for large-scale timekeeping,
the solstice is still required to fall within the zeroth term of each year.
* The calendar year rolls over at the transition between the positive twelfth term and the
negative twelfth term; that is, at noon on the December solstice.
