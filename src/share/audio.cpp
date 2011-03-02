/*   
 * Copyright (C) 2003 Robert Kooima
 *
 * NEVERBALL is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

#include <SDL.h>
#include <SDL_mixer.h>
#include <string.h>

#include "config.h"
#include "audio.h"

/*---------------------------------------------------------------------------*/

static int audio_state = 0;

static char       name[MAXSND][MAXSTR];
static int        chan[MAXSND];
static Mix_Chunk *buff[MAXSND];
static Mix_Music *song;

static char  curr_bgm[MAXSTR];
static char  next_bgm[MAXSTR];

static float fade_volume = 1.0f;
static float fade_rate   = 0.0f;

static int actual_volume_music=0;
/*---------------------------------------------------------------------------*/

void audio_init(void)
{
#ifndef NO_AUDIO
    int r = config_get_d(CONFIG_AUDIO_RATE);
    int b = config_get_d(CONFIG_AUDIO_BUFF);
    int i;
#endif

#ifdef DEBUG_AUDIO
    puts("audio: audio_init()");fflush(stdout);
#endif

    memset(curr_bgm, 0, MAXSTR);
    memset(next_bgm, 0, MAXSTR);


    if (audio_state == 0)
    {
#ifndef NO_AUDIO
        if (Mix_OpenAudio(r, MIX_DEFAULT_FORMAT, 1, b) == 0)
        {
            for (i = 0; i < MAXSND; i++)
                if (chan[i])
                    buff[i] = Mix_LoadWAV(config_data(name[i]));

            audio_state = 1;

            audio_volume(config_get_d(CONFIG_SOUND_VOLUME),
                         config_get_d(CONFIG_MUSIC_VOLUME));
        }
        else
#endif
        {
            fprintf(stderr, "Sound disabled\n");
            audio_state = 0;
        }
    }
}

void audio_free(void)
{
    int i;

#ifdef DEBUG_AUDIO
    puts("audio: audio_free()");fflush(stdout);
#endif
    if (audio_state == 1)
    {
        Mix_CloseAudio();

        for (i = 0; i < MAXSND; i++)
            if (buff[i])
            {
                Mix_FreeChunk(buff[i]);

                buff[i] = NULL;
                chan[i] = 0;
            }

        audio_state = 0;
    }
}

void audio_bind(int i, int c, const char *filename)
{
#ifdef DEBUG_AUDIO
    printf("audio: audio_bind(%i,%i,'%s')\n",i,c,filename);fflush(stdout);
#endif
    strncpy(name[i], filename, MAXSTR);
    chan[i] = c;
}

void audio_play(int i, float v)
{
#ifdef DEBUG_AUDIO
    printf("audio: audio_play(%i,%f)\n",i,(double)v);fflush(stdout);
#endif
    if (audio_state == 1 && buff[i])
    {
        Mix_VolumeChunk(buff[i], (int) (v * MIX_MAX_VOLUME));
        Mix_PlayChannel(chan[i], buff[i], 0);
    }
}

/*---------------------------------------------------------------------------*/

void audio_music_play(const char *filename)
{
#ifdef DEBUG_AUDIO
    printf("audio: audio_music_play('%s')\n",filename);fflush(stdout);
#endif
    if (audio_state)
    {
        audio_music_stop();

        if ((config_get_d(CONFIG_MUSIC_VOLUME) > 0) &&
            (song = Mix_LoadMUS(config_data(filename))))
        {
#ifdef DEBUG_AUDIO
            printf("audio: audio_music_play, OK '%s'\n",config_data(filename));fflush(stdout);
#endif
            Mix_PlayMusic(song, -1);
            strcpy(curr_bgm, filename);
        }
#ifdef DEBUG_AUDIO
	else if (config_get_d(CONFIG_MUSIC_VOLUME) > 0)
	{
	    FILE *f=fopen(config_data(filename),"rb");
            printf("audio: audio_music_play, ERROR '%s' %s (%s)\n",config_data(filename),f?"Exists":"Not open",SDL_GetError());fflush(stdout);
	    if (f) fclose(f);
	}
#endif
    }
}

void audio_music_queue(const char *filename)
{
#ifdef DEBUG_AUDIO
    printf("audio: audio_music_queue('%s')\n",filename);fflush(stdout);
#endif
    if (audio_state)
    {
        if (strlen(curr_bgm) == 0 || strcmp(filename, curr_bgm) != 0)
        {
	    actual_volume_music=0;
            Mix_VolumeMusic(0);
            fade_volume = 0.0f;

            audio_music_play(filename);
            strcpy(curr_bgm, filename);

            Mix_PauseMusic();
        }
    }
}

void audio_music_stop(void)
{
#ifdef DEBUG_AUDIO
    puts("audio: audio_music_stop()");fflush(stdout);
#endif
    if (audio_state)
    {
        if (Mix_PlayingMusic())
            Mix_HaltMusic();

        if (song)
            Mix_FreeMusic(song);

        song = NULL;
    }
}

/*---------------------------------------------------------------------------*/
/*
 * SDL_mixer already provides music fading.  Unfortunately, it halts playback
 * at the end of a fade.  We need to be able to fade music back in from the
 * point where it stopped.  So, we reinvent this wheel.
 */

void audio_timer(float dt)
{
#ifdef DEBUG_AUDIO
//    printf("audio: audio_timer(%f)\n",(double)dt);fflush(stdout);
#endif
    if (audio_state)
    {
        if (fade_rate > 0.0f || fade_rate < 0.0f)
	{
            fade_volume += dt / fade_rate;
#ifdef DEBUG_AUDIO
	    printf("audio: audio_timer, fade_volume=%f\n",(double)fade_volume);fflush(stdout);
#endif
	}

        if (fade_volume < 0.0f)
        {
            fade_volume = 0.0f;

            if (strlen(next_bgm) == 0)
            {
#ifdef DEBUG_AUDIO
		puts("audio: audio_timer, fade_volume < 0.0f && !next_bgm");fflush(stdout);
#endif
                fade_rate = 0.0f;
                if (Mix_PlayingMusic())
                    Mix_PauseMusic();
            }
            else
            {
                fade_rate = -fade_rate;
#ifdef DEBUG_AUDIO
		printf("audio: audio_timer, fade_volume < 0.0f, fade_rate=%f\n",(double)fade_rate);fflush(stdout);
#endif
                audio_music_queue(next_bgm);
            }
        }

        if (fade_volume > 1.0f)
        {
#ifdef DEBUG_AUDIO
	    puts("audio: audio_timer, fade_volume > 1.0f");fflush(stdout);
#endif
            fade_rate   = 0.0f;
            fade_volume = 1.0f;
        }   

        if (Mix_PausedMusic() && fade_rate > 0.0f)
	{
#ifdef DEBUG_AUDIO
	    puts("audio: audio_timer, fade_volume > 0.0f, Pause->Resume");fflush(stdout);
#endif
            Mix_ResumeMusic();
	    Mix_VolumeMusic(actual_volume_music);
	}
        else
   	{
		int new_volume=config_get_d(CONFIG_MUSIC_VOLUME) *(int) (fade_volume * MIX_MAX_VOLUME) / 10;
		if (actual_volume_music!=new_volume)
		{
#ifdef DEBUG_AUDIO
			printf("audio: audio_timer, actual_volume_music=%i != new_volume=%i\n",actual_volume_music,new_volume);fflush(stdout);
#endif
			Mix_VolumeMusic(new_volume);
			actual_volume_music=new_volume;
		}
	}
	{
		static unsigned retry=0;
		if (!(retry&0x0F))
		{
			Mix_ResumeMusic();
	    		Mix_VolumeMusic(actual_volume_music);
		}
		retry++;
	}
    }
}

void audio_music_fade_out(float t)
{
#ifdef DEBUG_AUDIO
    printf("audio: audio_music_fade_out(%f)\n",(double)t);fflush(stdout);
#endif
    fade_rate = -t;
    strcpy(next_bgm, "");
}

void audio_music_fade_in(float t)
{
#ifdef DEBUG_AUDIO
    printf("audio: audio_music_fade_in(%f)\n",(double)t);fflush(stdout);
#endif
    fade_rate = +t;
    strcpy(next_bgm, "");
}

void audio_music_fade_to(float t, const char *filename)
{
#ifdef DEBUG_AUDIO
    printf("audio: audio_music_fade_to(%f,'%s')\n",(double)t,filename);fflush(stdout);
#endif
    if (fade_volume > 0)
    {
        if (strlen(curr_bgm) == 0 || strcmp(filename, curr_bgm) != 0)
        {
            strcpy(next_bgm, filename);
            fade_rate = -t;
        }
        else fade_rate = t;
    }
    else
    {
        audio_music_queue(filename);
        audio_music_fade_in(t);
    }
}

void audio_volume(int s, int m)
{
#ifdef DEBUG_AUDIO
    printf("audio: audio_volume(%i,%i)\n",s,m);fflush(stdout);
#endif
    if (audio_state)
    {
        Mix_Volume(-1, s * MIX_MAX_VOLUME / 10);
        Mix_VolumeMusic(m * MIX_MAX_VOLUME / 10);
	actual_volume_music=(m * MIX_MAX_VOLUME / 10);
    }
}

/*---------------------------------------------------------------------------*/
