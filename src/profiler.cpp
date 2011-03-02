
#ifdef PROFILER

#include "profiler.h"

static unsigned prof_total_initial=0;
unsigned prof_total=0;
static char *prof_msg[PROFILER_MAX];
unsigned long long prof_initial[PROFILER_MAX];
unsigned long long prof_sum[PROFILER_MAX];
unsigned long long prof_executed[PROFILER_MAX];
int prof_started[PROFILER_MAX];

void prof_reset(void)
{
	int i;
	unsigned long long s=get_time_in_micros();
	for(i=0;i<PROFILER_MAX;i++)
	{
		prof_initial[i]=s;
		prof_sum[i]=0;
		if (prof_started[i])
			prof_executed[i]=1;
		else
			prof_executed[i]=0;
	}
	prof_total_initial=s;
}

void prof_init(void)
{
	unsigned i;
	unsigned long long s=get_time_in_micros();
	for(i=0;i<PROFILER_MAX;i++)
	{
		prof_initial[i]=s;
		prof_sum[i]=0;
		if (!prof_total)
		{
			prof_executed[i]=0;
			prof_started[i]=0;
			prof_msg[i]=NULL;
		}
		else
		{
			if (prof_started[i])
			        prof_executed[i]=1;
			else
			        prof_executed[i]=0;
		}
	}
	prof_total_initial=s;
	prof_add("Main");		// PROFILER_TOTAL
	prof_add("Blit");		// PROFILER_BLIT
	prof_add("Timer");		// PROFILER_TIMER
	prof_add("Paint");		// PROFILER_PAINT
	prof_add("R0");		// PROFILER_R0
	prof_add("R1");		// PROFILER_R1
	prof_add("R2");		// PROFILER_R2
	prof_add("R3");		// PROFILER_R3
	prof_add("R4");		// PROFILER_R4
	prof_add("R5");		// PROFILER_R5
	prof_add("R6");		// PROFILER_R6
	prof_add("R7");		// PROFILER_R7
}

void prof_add(char *msg)
{
	if (prof_total<PROFILER_MAX)
	{
		prof_msg[prof_total]=msg;
		prof_total++;
	}
}

void prof_show(void)
{
	int i;
	double toper=0;
	unsigned long long to;
	for(i=prof_total-1;i>=0;i--)
		prof_end(i);
	to=prof_sum[0];
	puts("\n\n\n\n");
	puts("--------------------------------------------");
	for(i=0;i<prof_total;i++)
	{
		unsigned long long t0=prof_sum[i];
		double percent=(double)t0;
		percent*=100.0;
		percent/=(double)to;
		toper+=percent;
#ifndef PROFILER_SDL
		t0/=1000;
#endif
		printf("%s: %.2f%% -> Ticks=%i ->",prof_msg[i],percent,((unsigned)t0));
		if (prof_executed[i]>3000LL)
			printf("%iK veces\n",(unsigned)(prof_executed[i]/1000LL));
		else
			printf("%i veces\n",(unsigned)(prof_executed[i]));
	}
//      printf("TOTAL: %.2f%% -> Ticks=%i\n",toper,to);
	puts("--------------------------------------------"); fflush(stdout);
}

void prof_setmsg(int n, char *msg)
{
	prof_msg[n]=msg;
}

#endif

