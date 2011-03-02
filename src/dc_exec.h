
#if defined(DREAMCAST) && !defined(PROFILER) 
void get_dc_video_hz(void);
void set_dc_video_hz(void);
#else
#define get_dc_video_hz()
#define set_dc_video_hz()
#endif
