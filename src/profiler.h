#ifndef _PROFILER_H_
#define _PROFILER_H_

enum{
	PROFILER_TOTAL = 0,
	PROFILER_BLIT,
	PROFILER_TIMER,
	PROFILER_PAINT,
	PROFILER_R0,
	PROFILER_R1,
	PROFILER_R2,
	PROFILER_R3,
	PROFILER_R4,
	PROFILER_R5,
	PROFILER_R6,
	PROFILER_R7,
};



#ifndef PROFILER

#define prof_start(A)
#define prof_end(A)
#define prof_show()
#define prof_init()
#define prof_reset()

#else

#include <stdio.h>

#ifdef PROFILER_SDL
#include <SDL.h>
#define PROFILER__REDUCTION__ 0
#define PROFILER__ADJUST__ 0
#define get_time_in_micros() (unsigned long long)SDL_GetTicks()
#else
#ifndef DREAMCAST
#include <sys/time.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <time.h>

#ifndef __GNUC__
#define EPOCHFILETIME (116444736000000000i64)
#else
#define EPOCHFILETIME (116444736000000000LL)
#endif


static __inline__ int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME        ft;
    LARGE_INTEGER   li;
    __int64         t;
    static int      tzflag;

    if (tv)
    {
        GetSystemTimeAsFileTime(&ft);
        li.LowPart  = ft.dwLowDateTime;
        li.HighPart = ft.dwHighDateTime;
        t  = li.QuadPart;       /* In 100-nanosecond intervals */
        t -= EPOCHFILETIME;     /* Offset to the Epoch time */
        t /= 10;                /* In microseconds */
        tv->tv_sec  = (long)(t / 1000000);
        tv->tv_usec = (long)(t % 1000000);
    }

    return 0;
}

#endif

static __inline__ unsigned long long get_time_in_micros(void)
{
	unsigned long long ret;
	struct timeval tm;
	gettimeofday(&tm,NULL);
	ret=tm.tv_sec;
	ret*=1000000LL;
	ret+=tm.tv_usec;
	return ret;
}
#define PROFILER__REDUCTION__ 0
#define PROFILER__ADJUST__ 0
#else
#include <kos.h>
#define PROFILER__REDUCTION__ 2
#define PROFILER__ADJUST__ 1
#define get_time_in_micros() timer_us_gettime64()
#endif
#endif

#define PROFILER_MAX 64

extern unsigned long long prof_initial[PROFILER_MAX];
extern unsigned long long prof_sum[PROFILER_MAX];
extern unsigned long long prof_executed[PROFILER_MAX];
extern int prof_started[PROFILER_MAX];

static __inline__ void prof_start(unsigned a)
{
	if (prof_started[a])
		return;
	prof_executed[a]++;
	prof_initial[a]=get_time_in_micros();
	prof_started[a]=1;
}


static __inline__ void prof_end(unsigned a)
{
	if (!prof_started[a])
		return;

	extern unsigned prof_total;
	unsigned i;
	for(i=0;i<prof_total;i++)
		prof_initial[i]+=PROFILER__REDUCTION__;
	prof_sum[a]+=get_time_in_micros()-prof_initial[a]+PROFILER__ADJUST__;
	prof_started[a]=0;
}

#undef PROFILER__REDUCTION__
#undef PROFILER__ADJUST__

void prof_init(void);
void prof_add(char *msg);
void prof_show(void);
void prof_setmsg(int n, char *msg);
void prof_reset(void);


#endif





#endif


