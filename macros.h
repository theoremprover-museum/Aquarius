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
 *  macros.h -- This file contains some #define preprocessor macros
 *
 */

/* Penguin */

#define minimum(x,y) ((x <= y) ? (x) : (y))

/* Penguin */

/*************
 *
 *    CPU_TIME(sec, usec) - It has been sec seconds + usec microseconds
 *        since the start of this process.
 *
 *************/

#ifdef TURBO_C  /* PC */
#define CPU_TIME(sec, usec)  \
{  \
    struct timeb r;  \
    ftime(&r);  \
    sec = r.time;  \
    usec = (long) r.millitm * 1000;  \
}  /* CPU_TIME */
#else

#ifdef THINK_C  /* Macintosh */
#define CPU_TIME(sec, usec) \
{ \
    clock_t ticks; \
    ticks = clock(); \
    sec = ticks / CLOCKS_PER_SEC; \
    usec = 0; \
}  /* CPU_TIME */
#else

#ifdef TP_UNIX
#define CPU_TIME(sec, usec)  \
{  \
    struct rusage r;  \
    getrusage(RUSAGE_SELF, &r);  \
    sec = r.ru_utime.tv_sec;  \
    usec = r.ru_utime.tv_usec;  \
}  /* CPU_TIME */

#else
#define CPU_TIME(sec, usec) {sec = usec = 0;}
#endif
#endif
#endif

/*************
 *
 *    CLOCK_START(clock_num) - Start or continue timing.
 *
 *        If the clock is already running, a warning message is printed.
 *
 *************/

#ifdef NO_CLOCK
#define CLOCK_START(c)   /* empty string */
#else
#define CLOCK_START(c)  \
{  \
    struct clock *cp;  \
  \
    cp = &Clocks[c];  \
    if (cp->curr_sec != -1) {  \
	fprintf(Fderr, "WARNING, CLOCK_START: clock %d already on.\n", c);  \
	printf("WARNING, CLOCK_START: clock %d already on.\n", c);  \
	}  \
    else  \
	CPU_TIME(cp->curr_sec, cp->curr_usec) \
}  /* CLOCK_START */
#endif

/*************
 *
 *    CLOCK_STOP(clock_num) - Stop timing and add to accumulated total.
 *
 *        If the clock not running, a warning message is printed.
 *
 *************/

#ifdef NO_CLOCK
#define CLOCK_STOP(c)   /* empty string */
#else
#define CLOCK_STOP(c)  \
{  \
    long sec, usec;  \
    struct clock *cp;  \
  \
    cp = &Clocks[c];  \
    if (cp->curr_sec == -1) {  \
	fprintf(Fderr, "WARNING, CLOCK_STOP: clock %d already off.\n", c);  \
	printf("WARNING, CLOCK_STOP: clock %d already off.\n", c);  \
	}  \
    else {  \
	CPU_TIME(sec, usec)  \
	cp->accum_sec += sec - cp->curr_sec;  \
	cp->accum_usec += usec - cp->curr_usec;  \
	cp->curr_sec = -1;  \
	cp->curr_usec = -1;  \
	}  \
}  /* CLOCK_STOP */
#endif

/*************
 *
 *    SET_BIT, CLEAR_BIT, BIT.
 *
 *************/

#define SCRATCH_BIT   01
#define NEW_DEMOD_BIT 02
#define LINEAR_BIT    04
#define GROUND_BIT   010

#define SET_BIT(vec, val)    (vec = vec | val)
#define CLEAR_BIT(vec, val)  (vec = vec & ~val)
#define TP_BIT(vec, val)        (vec & val)
