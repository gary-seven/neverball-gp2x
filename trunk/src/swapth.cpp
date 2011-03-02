
#if defined(SWAPBUFFER_THREATED) && !defined(PROFILER)

#include<stdio.h>
#include<stdlib.h>
#include<SDL.h>
#include<SDL_thread.h>

static int running=1;
static SDL_sem *swaping_sem;
static SDL_sem *drawed_sem;

static int swapth_thread(void *data)
{
	while(running)
	{
		SDL_SemWait(swaping_sem);
		SDL_GL_SwapBuffers();
		SDL_SemPost(drawed_sem);
	}
	return 0;
}

void swapth_init(void)
{
	running=1;
	swaping_sem=SDL_CreateSemaphore(0);
	drawed_sem=SDL_CreateSemaphore(0);
	SDL_SemPost(drawed_sem);
	SDL_CreateThread ((int (*)(void *))swapth_thread, NULL);
}

void swapth_wait(void)
{
	SDL_SemWait(drawed_sem);
}

void swapth_go(void)
{
	SDL_SemPost(swaping_sem);
}

void swapth_quit(void)
{
	running=0;
	SDL_SemPost(swaping_sem);
}

#endif

