

#if defined(SWAPBUFFER_THREATED) && !defined(PROFILER)

void swapth_init(void);
void swapth_wait(void);
void swapth_go(void);
void swapth_quit(void);

#else

#define swapth_init()
#define swapth_wait()
#define swapth_go() SDL_GL_SwapBuffers()
#define swapth_quit()
#endif
