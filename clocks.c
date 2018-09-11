/* Penguin */

/*
 *         Copyright (C) Argonne National Laboratory
 *
 *   Argonne does not guarantee this software in any manner and is
 *   not responsible for any damages that may result from its use.
 *   Furthermore, Argonne does not provide any formal support for this
 *   software.  This is an experimental program.  This software
 *   or any part of it may be freely copied and redistributed,
 *   provided that this paragraph is included in each source file.
 *
 */

/*
 *  clocks.c -- This file has various timing routines.  (Some of them
 *  have been moved to macros.h.)
 *
 */

#include "header.h"

/*************
 *
 *    clock_init() - Initialize all clocks.
 *
 *************/

void clock_init()
{
    int i;

#ifdef THINK_C  /* kludge for mac: see run_time */
    long l;
    l = run_time();
#endif

    for (i=0; i<MAX_CLOCKS; i++)
	clock_reset(i);
}  /* clock_init */

/*
 *
 *    CPU_TIME(sec, usec) - It has been sec seconds + usec microseconds
 *    since the start of this process.
 *
 */

/* This routine has been made into a macro. */

/*
 *
 *    CLOCK_START(clock_num) - Start or continue timing.
 *
 *    If the clock is already running, a warning message is printed.
 *
 */

/* This routine has been made into a macro. */

/*
 *
 *    CLOCK_STOP(clock_num) - Stop timing and add to accumulated total.
 *
 *    If the clock not running, a warning message is printed.
 *
 */

/* This routine has been made into a macro. */

/*************
 *
 *    long clock_val(clock_num) - Returns accumulated time in milliseconds.
 *
 *    Clock need not be stopped.
 *
 *************/

long clock_val(c)
int c;
{
    long sec, usec, i, j;

    i = (Clocks[c].accum_sec * 1000) + (Clocks[c].accum_usec / 1000);
    if (Clocks[c].curr_sec == -1)
	return(i);
    else {
	CPU_TIME(sec, usec)
	j = ((sec - Clocks[c].curr_sec) * 1000) + 
	    ((usec - Clocks[c].curr_usec) / 1000);
	return(i+j);
	}
}  /* clock_val */

/*************
 *
 *    clock_reset(clock_num) - Clocks must be reset before being used.
 *
 *************/

void clock_reset(c)
int c;
{
    Clocks[c].accum_sec = Clocks[c].accum_usec = 0;
    Clocks[c].curr_sec = Clocks[c].curr_usec = -1;
}  /* clock_reset */

/*************
 *
 *   char *get_time() - get a string representation of current date and time
 *
 *************/

char *get_time()
{
#ifdef TP_UNIX
    long i;
    i = time((long *) NULL);
#else
    time_t i;
    i = time((time_t *) NULL);
#endif

    return(asctime(localtime(&i)));

}  /* get_time */

/*************
 *
 *    long system_time() - Return system time in milliseconds.
 *
 *************/

long system_time()
{
#ifdef TP_UNIX
    struct rusage r;
    long sec, usec;

    getrusage(RUSAGE_SELF, &r);
    sec = r.ru_stime.tv_sec;
    usec = r.ru_stime.tv_usec;

    return((sec * 1000) + (usec / 1000));
#else
    return(0);
#endif
}  /* system_time */

/*************
 *
 *    long run_time() - Return run time in milliseconds.
 *
 *    This is used instead of the normal clock routines in case
 *    program is compiled with NO_CLOCK.
 *
 *************/

long run_time()
{
#ifdef TURBO_C  /* PC */
    clock_t ticks;
    long sec;

    ticks = clock();
    sec = ticks / CLK_TCK * 1000;
    return(sec);
#else
#ifdef THINK_C  /* Macintosh */
    clock_t ticks;
    long sec;

    /* following kludge is because mac gives ticks since */
    /* power up, instead of ticks since start of process. */
    static int first_call = 1;
    static clock_t start;

    if (first_call) {
        first_call = 0;
        start = clock();
        }

    ticks = clock();
sec = (ticks - start) / CLOCKS_PER_SEC * 1000;
    return(sec);
#else
#ifdef TP_UNIX
    struct rusage r;
    long sec, usec;

    getrusage(RUSAGE_SELF, &r);
    sec = r.ru_utime.tv_sec;
    usec = r.ru_utime.tv_usec;

    return((sec * 1000) + (usec / 1000));
#else
    return((long) 0);
#endif
#endif
#endif
}  /* run_time */

