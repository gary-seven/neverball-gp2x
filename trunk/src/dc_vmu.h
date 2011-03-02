
#ifdef DREAMCAST
void ram2vmu(char *);
void vmu2ram(char *);
#else
#define ram2vmu(FN)
#define vmu2ram(FN)
#endif
