
#if defined(DREAMCAST) && !defined(PROFILER) 

#include<kos.h>

#include<stdio.h>
#include<stdlib.h>
#include<SDL.h>
#include<SDL_dreamcast.h>

#define DC_VIDEO_HZ_MEM_POSICION 0xcfbfa65

static unsigned char dc_video_hz_copy=0;

void get_dc_video_hz(void)
{
	unsigned char *mihz=(unsigned char *)DC_VIDEO_HZ_MEM_POSICION;
	dcache_inval_range(DC_VIDEO_HZ_MEM_POSICION&0xFFFF0000,0x10000);
	dc_video_hz_copy=*mihz;
	switch (dc_video_hz_copy)
	{
		case 2:
			SDL_DC_Default60Hz(SDL_TRUE);
		case 1:
			SDL_DC_ShowAskHz(SDL_FALSE);
			break;
	}
}

void set_dc_video_hz(void)
{
	unsigned int *p=(unsigned int *)0xcff00ff;
	unsigned char *mihz=(unsigned char *)DC_VIDEO_HZ_MEM_POSICION;
	FILE *f=fopen("/cd/menu.bin","rb");
	if (f!=NULL)
	{
		void *mem;
		long len;
		fseek(f,0,SEEK_END);
		len=ftell(f);
		fseek(f,0,SEEK_SET);
		mem=malloc(len);
		fread(mem,1,len,f);
		fclose(f);
		*mihz=dc_video_hz_copy;
		dcache_flush_range(DC_VIDEO_HZ_MEM_POSICION&0xFFFF0000,0x10000);
		arch_exec(mem,len);
		free(mem); // THIS NEVER MUST EXECUTE IT
	}
	*p=0x12345678; // FORCE EXCEPTION
}
#endif
