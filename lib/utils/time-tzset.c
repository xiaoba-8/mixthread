/*
 * time-tzset.c
 *
 *  Created on: 2019-4-11
 *      Author: xiaoba-8
 */

#include <errno.h>
#include <time.h>

#include <mix/utils/time-tzset.h>

#define	SECS_PER_HOUR	(60 * 60)
#define	SECS_PER_DAY	(SECS_PER_HOUR * 24)

const unsigned short int my_mon_yday[2][13] =
  {
    /* Normal years.  */
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    /* Leap years.  */
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
  };


int get_local_timezone()
{
	static int s_timezone = 0;
	static int s_bZoneInitialized = 0;
	if (!s_bZoneInitialized)
	{
		s_bZoneInitialized = 1;
		tzset();
		s_timezone = timezone;
	}

	return s_timezone;
}

int my_offtime(const time_t *t, long int offset, struct tm *tp)
{
	long int days, rem, y;
	const unsigned short int *ip;

	days = *t / SECS_PER_DAY;
	rem = *t % SECS_PER_DAY;
	rem += offset;
	while (rem < 0)
	{
		rem += SECS_PER_DAY;
		--days;
	}
	while (rem >= SECS_PER_DAY)
	{
		rem -= SECS_PER_DAY;
		++days;
	}
	tp->tm_hour = rem / SECS_PER_HOUR;
	rem %= SECS_PER_HOUR;
	tp->tm_min = rem / 60;
	tp->tm_sec = rem % 60;
	/* January 1, 1970 was a Thursday.  */
	tp->tm_wday = (4 + days) % 7;
	if (tp->tm_wday < 0)
		tp->tm_wday += 7;
	y = 1970;

#define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
#define LEAPS_THRU_END_OF(y) (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))

	while (days < 0 || days >= (__isleap(y) ? 366 : 365))
	{
		/* Guess a corrected year, assuming 365 days per year.  */
		long int yg = y + days / 365 - (days % 365 < 0);

		/* Adjust DAYS and Y to match the guessed year.  */
		days -= ((yg - y) * 365 + LEAPS_THRU_END_OF (yg - 1)
				- LEAPS_THRU_END_OF (y - 1));
		y = yg;
	}
	tp->tm_year = y - 1900;
	if (tp->tm_year != y - 1900)
	{
		/* The year cannot be represented due to overflow.  */errno = EOVERFLOW;
		return 0;
	}
	tp->tm_yday = days;
	ip = my_mon_yday[__isleap(y)];
	for (y = 11; days < (long int) ip[y]; --y)
		continue;
	days -= ip[y];
	tp->tm_mon = y;
	tp->tm_mday = days + 1;
	return 1;
}

struct tm *my_gmtime_r (const time_t *__restrict __timer,
			    struct tm *__restrict __tp)
{
	if(my_offtime(__timer, 0, __tp))
	{
		return __tp;
	}
	else
	{
		__tp = NULL;
		return __tp;
	}
}

struct tm *my_localtime_r (const time_t *__restrict __timer, int timezone,
			    struct tm *__restrict __tp)
{
	if(my_offtime(__timer, timezone, __tp))
	{
		return __tp;
	}
	else
	{
		__tp = NULL;
		return __tp;
	}
}
