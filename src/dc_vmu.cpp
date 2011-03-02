
#ifdef DREAMCAST

#include<kos.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<zlib.h>

#include "dc_icon.h"

#define MAXBUF (1024*64)

static char filename_ram[64];
static char filename_vmu[64];
static unsigned char buf1[MAXBUF];
static unsigned char buf2[MAXBUF];

static void setname(char *filename)
{
	int i,len=strlen(filename);
	char fn[64];
	if (len>63) len=63;
	memset(fn,0,64);
	strncpy(fn,filename,len);
	strcpy(filename_ram,"/ram/");
	strcat(filename_ram,filename);
	strcpy(filename_vmu,"/vmu/a1/");
	for(i=0;i<len;i++)
		if (fn[i]=='-')
			break;
	if (i<len)
	{
		if (i>6)
		{
			fn[i-0]='l';
			fn[i-1]='l';
			fn[i-2]='b';
			fn[i-3]='r';
			fn[i-4]='v';
			fn[i-5]='n';
			strcat(filename_vmu,(char *)&fn[i-5]);
		}
		else
			strcat(filename_vmu,(char *)&fn[i+1]);
	}
	else
		strcat(filename_vmu,fn);
}

static int readit(char *filename, void *buf,int pad)
{
	int ret=0;
	FILE *f=fopen(filename,"rb");
	if (f)
	{
		fseek(f,0,SEEK_END);
		ret=ftell(f)-pad;
		fseek(f,pad,SEEK_SET);
		if (ret>MAXBUF)
			ret=MAXBUF;
		if (ret>0)
			fread(buf,1,ret,f);
		fclose(f);
	}
	return ret;
}

static void writeit(char *filename, void *buf, int len)
{
	FILE *f=fopen(filename,"wb");
	if (f)
	{
		if (len>MAXBUF)
			len=MAXBUF;
		fwrite(buf,1,len,f);
		fclose(f);
	}
}

void ram2vmu(char *filename)
{
	int len;
	setname(filename);
	len=readit((char *)&filename_ram[0],(void *)&buf1[0],0);
	if (len)
	{
		int clen=MAXBUF;
		unsigned char *paquete;
		vmu_pkg_t       pkg;
		compress2((Bytef*)&buf2[0],(uLongf*)&clen,(Bytef*)&buf1[0],len,9);
		memset(&pkg, 0, sizeof(pkg));
		strcpy(pkg.desc_short, "NEVERBALL");
		strcpy(pkg.desc_long, filename);
		strcpy(pkg.app_id, "NEVERBALL");
		pkg.icon_cnt = 1;
		pkg.icon_anim_speed = 0;
		pkg.eyecatch_type = VMUPKG_EC_NONE;
		pkg.eyecatch_data = NULL;
		pkg.data_len = clen;
		pkg.data = (const uint8*)&buf2[0];
		memcpy((void *)&pkg.icon_pal[0],(void *)&vmu_savestate_icon_pal,32);
		pkg.icon_data = (const uint8*)&vmu_savestate_icon_data;
		vmu_pkg_build(&pkg, &paquete, &clen);
		if (paquete!=NULL)
		{
			unlink((char *)&filename_vmu[0]);
			writeit((char *)&filename_vmu[0],(void *)paquete,clen);
			free(paquete);
		}
	}
}

void vmu2ram(char *filename)
{
	int len;
	setname(filename);
	len=readit((char *)&filename_vmu[0],(void *)&buf1[0],128+512);
	if (len)
	{
		int clen=MAXBUF;
		uncompress((Bytef*)&buf2[0],(uLongf*)&clen,(Bytef*)&buf1[0],len);
		writeit((char *)&filename_ram[0],(void *)&buf2[0],clen);
	}
}
#endif
