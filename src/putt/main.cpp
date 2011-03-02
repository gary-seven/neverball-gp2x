/*   
 * Copyright (C) 2003 Robert Kooima
 *
 * NEVERPUTT is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

/*---------------------------------------------------------------------------*/
#ifdef DREAMCAST
#include <kos.h>
//extern uint8 romdisk[];
KOS_INIT_FLAGS(INIT_DEFAULT);
//KOS_INIT_ROMDISK(romdisk);
#endif

#ifdef WIN32
#pragma comment(lib, "SDL_ttf.lib")
#pragma comment(lib, "SDL_mixer.lib")
#pragma comment(lib, "SDL_image.lib")
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")
#pragma comment(lib, "opengl32.lib")
#endif

/*---------------------------------------------------------------------------*/

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glext.h"
#include "audio.h"
#include "image.h"
#include "state.h"
#include "config.h"
#include "course.h"
#include "hole.h"
#include "game.h"
#include "gui.h"
#include "profiler.h"
#include "swapth.h"
#include "dc_exec.h"

#include "st_conf.h"
#include "st_all.h"

#define TITLE "Neverputt"

/*---------------------------------------------------------------------------*/

static int shot(void)
{
    static char filename[MAXSTR];
    static int  num = 0;

    sprintf(filename, "screen%02d.bmp", num++);

    image_snap(filename);

    return 1;
}

/*---------------------------------------------------------------------------*/

static void toggle_wire(void)
{
    static int wire = 0;

    if (wire)
    {
#ifndef DREAMCAST_KGL_NOT_IMPLEMENT
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
        glEnable(GL_TEXTURE_2D);
#ifndef NO_LIGHT
        glEnable(GL_LIGHTING);
#endif
        wire = 0;
    }
    else
    {
#ifndef DREAMCAST_KGL_NOT_IMPLEMENT
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
        glDisable(GL_TEXTURE_2D);
#ifndef NO_LIGHT
        glDisable(GL_LIGHTING);
#endif
        wire = 1;
    }
}
/*---------------------------------------------------------------------------*/

static int loop(void)
{
    SDL_Event e;
    int d = 1;
#ifndef AUTO_EVENTS

    while (d && SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            return 0;

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN)
            config_tgl_pause();

        if (!config_get_pause())
            switch (e.type)
            {
            case SDL_MOUSEMOTION:
                st_point(+e.motion.x,
#ifdef __APPLE__
                         +e.motion.y,
#else
                         -e.motion.y + config_get_d(CONFIG_HEIGHT),
#endif
                         +e.motion.xrel,
                         -e.motion.yrel);
                break;

            case SDL_MOUSEBUTTONDOWN:
                d = st_click((e.button.button == SDL_BUTTON_LEFT) ? -1 : 1, 1);
                break;

            case SDL_MOUSEBUTTONUP:
                d = st_click((e.button.button == SDL_BUTTON_LEFT) ? -1 : 1, 0);
                break;

	    case SDL_KEYUP:
                switch (e.key.keysym.sym)
                {
//		case SDLK_SPACE:
		case SDLK_LCTRL:
			d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 0);
			break;
		case SDLK_LEFT:
		case SDLK_RIGHT:
			st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_X), 1);
			break;
		case SDLK_UP:
		case SDLK_DOWN:
			st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_Y), 1);
			break;
                
                default:
                    d = st_keybd(e.key.keysym.sym, 0);
		}
		break;

            case SDL_KEYDOWN:
                switch (e.key.keysym.sym)
                {
                case SDLK_F10: d = shot();                break;
                case SDLK_F9:  config_tgl_d(CONFIG_FPS);  break;
                case SDLK_F8:  config_tgl_d(CONFIG_NICE); break;
                case SDLK_F7:  toggle_wire();             break;

		case SDLK_SPACE:
		case SDLK_LCTRL:
			d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 1);
			break;
		case SDLK_LEFT:
			st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_X), -JOY_MAX);
			break;
		case SDLK_RIGHT:
			st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_X), +JOY_MAX);
			break;
		case SDLK_UP:
			st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_Y), -JOY_MAX);
			break;
		case SDLK_DOWN:
			st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_Y), +JOY_MAX);
			break;
                
                default:
                    d = st_keybd(e.key.keysym.sym, 1);
                }
                break;

            case SDL_ACTIVEEVENT:
                if (e.active.state == SDL_APPINPUTFOCUS)
                {
                    if (e.active.gain == 0)
                        config_set_pause();
                }
                break;
	    case SDL_JOYAXISMOTION:
#ifdef DREAMCAST
	    	if (e.jaxis.value>JOY_DREAMCAST_MAX) e.jaxis.value=JOY_DREAMCAST_MAX;
		else if (e.jaxis.value<-JOY_DREAMCAST_MAX) e.jaxis.value=-JOY_DREAMCAST_MAX;
		e.jaxis.value*=JOY_MAX/JOY_DREAMCAST_MAX;
		if (e.jaxis.axis<2)
#endif
		st_stick(e.jaxis.axis, e.jaxis.value);
		break;
            }
    }
#else
    static unsigned count=0;
    while (d && SDL_PollEvent(&e)) if (e.type == SDL_QUIT) return 0;
    count++;
    if (count>AUTO_EVENTS)
	    d=0;
    else if (count&1)
	    st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 1);
    else
	    st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 0);
#endif
    return d;
}

#ifdef PROFILER
static Uint32 tticks=0;
#define ticks() (tticks+=20)
#else
#define ticks() SDL_GetTicks()
#endif

int main(int argc, char *argv[])
{
  int camera = 0;

	get_dc_video_hz();
	puts("MAIN!!");
    srand((int) time(NULL));

    if (config_data_path((argc > 1 ? argv[1] : NULL), COURSE_FILE))
    {
        if (config_user_path(NULL))
        {
            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_CDROM | SDL_INIT_JOYSTICK) == 0)
            {
                SDL_Joystick *joy = NULL;

                config_init();
                config_load();

                /* Initialize the joystick. */

                if (SDL_NumJoysticks() > 0)
                {
                    joy=SDL_JoystickOpen(config_get_d(CONFIG_JOYSTICK_DEVICE));
                    if (joy)
                        SDL_JoystickEventState(SDL_ENABLE);
                }


                /* Cache Neverball's camera setting. */

                camera = config_get_d(CONFIG_CAMERA);

                /* Initialize the audio. */

                audio_bind(AUD_BIRDIE,  1, "snd/birdie.wav");
                audio_bind(AUD_BOGEY,   1, "snd/bogey.wav");
                audio_bind(AUD_BUMP,    1, "snd/bink.wav");
                audio_bind(AUD_DOUBLE,  1, "snd/double.wav");
                audio_bind(AUD_EAGLE,   1, "snd/eagle.wav");
                audio_bind(AUD_JUMP,    2, "snd/jump.wav");
                audio_bind(AUD_MENU,    2, "snd/menu.wav");
                audio_bind(AUD_ONE,     1, "snd/one.wav");
                audio_bind(AUD_PAR,     1, "snd/par.wav");
                audio_bind(AUD_PENALTY, 1, "snd/penalty.wav");
                audio_bind(AUD_PLAYER1, 1, "snd/player1.wav");
                audio_bind(AUD_PLAYER2, 1, "snd/player2.wav");
                audio_bind(AUD_PLAYER3, 1, "snd/player3.wav");
                audio_bind(AUD_PLAYER4, 1, "snd/player4.wav");
                audio_bind(AUD_SWITCH,  2, "snd/switch.wav");
                audio_bind(AUD_SUCCESS, 1, "snd/success.wav");

                audio_init();

                /* Require 16-bit double buffer with 16-bit depth buffer. */

                SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
                SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
                SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
                SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
                SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

                /* Initialize the video. */

                if (config_mode(config_get_d(CONFIG_FULLSCREEN),
                                config_get_d(CONFIG_WIDTH),
                                config_get_d(CONFIG_HEIGHT)))
                {
                    int t1, t0 = ticks();

                    SDL_WM_SetCaption(TITLE, TITLE); 
		    prof_init();

                    /* Run the main game loop. */

                    init_state(&st_null);
                    goto_state(&st_title);

		    swapth_init();
		    prof_start(PROFILER_TOTAL);

                    while (loop())
                        if ((t1 = ticks()) > t0)
                        {
                           if (config_get_pause())
                            {
				swapth_wait();
                                st_paint();
                                gui_blank();
                            }
                            else
                            {
                                st_timer((t1 - t0) / 1000.f);
				swapth_wait();
                                st_paint();
                            }
		            prof_start(PROFILER_BLIT);
			    swapth_go();
		            prof_end(PROFILER_BLIT);

                            t0 = t1;

#if !defined(DREAMCAST) && !defined(PROFILER)
                            if (config_get_d(CONFIG_NICE))
                                SDL_Delay(1);
#endif
                        }
		    prof_end(PROFILER_TOTAL);
		    swapth_quit();
                }
                else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());

                /* Restore Neverball's camera setting. */

                config_set_d(CONFIG_CAMERA, camera);
                config_save();

		prof_show();
		set_dc_video_hz();
                SDL_Quit();
            }
            else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());
        }
        else fprintf(stderr, "Failure to establish config directory\n");
    }
    else fprintf(stderr, "Failure to establish game data directory\n");

    return 0;
}

/*---------------------------------------------------------------------------*/

