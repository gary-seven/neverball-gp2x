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
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "glext.h"
#include "geom.h"
#include "part.h"
#include "vec3.h"
#include "solid.h"
#include "image.h"
#include "config.h"
#include "profiler.h"
#include "elements.h"

#define PI 3.1415926535897932

/*---------------------------------------------------------------------------*/

static GLuint ball_list,ball_list2;
static GLuint ball_text=0;

void ball_init(int b)
{
    char name[MAXSTR];
    int i, slices = b ? 32 : 12; //16;
    int j, stacks = b ? 16 :  8;

    config_get_s(CONFIG_BALL, name, MAXSTR);

    ball_text = make_image_from_file(NULL, NULL, NULL, NULL, name);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#ifdef DREAMCAST
    glTexEnvi(GL_TEXTURE_2D,GL_TEXTURE_ENV_MODE,GL_MODULATEALPHA);
#endif

    ball_list = elementsGenLists(1);
    
    elementsNewList(ball_list, GL_COMPILE);
    {
        for (i = 0; i < stacks; i++)
        {
            float k0 = (float)  i      / stacks;
            float k1 = (float) (i + 1) / stacks;

            float s0 = fsinf(V_PI * (k0 - 0.5));
            float c0 = fcosf(V_PI * (k0 - 0.5));
            float s1 = fsinf(V_PI * (k1 - 0.5));
            float c1 = fcosf(V_PI * (k1 - 0.5));

            elementsBindTexture(GL_TEXTURE_2D, ball_text);
            elementsBegin(GL_QUAD_STRIP);
            {
                for (j = 0; j <= slices; j++)
                {
                    float k = (float) j / slices;
                    float s = fsinf(V_PI * k * 2.0);
                    float c = fcosf(V_PI * k * 2.0);

                    elementsTexCoord2f(k, k0);
#ifndef NO_LIGHT
                    glNormal3f(s * c0, c * c0, s0);
#endif
                    elementsVertex3f(s * c0, c * c0, s0);

                    elementsTexCoord2f(k, k1);
#ifndef NO_LIGHT
                    glNormal3f(s * c1, c * c1, s1);
#endif
                    elementsVertex3f(s * c1, c * c1, s1);
                }
            }
            elementsEnd();
        }
    }
    elementsEndList();
#ifndef NO_LIGHT
    ball_list2=ball_list;
#else
    ball_list2 = elementsGenLists(1);
    
    elementsNewList(ball_list2, GL_COMPILE);
    {
        for (i = 0; i < stacks; i++)
        {
            float k0 = (float)  i      / stacks;
            float k1 = (float) (i + 1) / stacks;

            float s0 = fsinf(V_PI * (k0 - 0.5));
            float c0 = fcosf(V_PI * (k0 - 0.5));
            float s1 = fsinf(V_PI * (k1 - 0.5));
            float c1 = fcosf(V_PI * (k1 - 0.5));

            elementsBegin(GL_QUAD_STRIP);
            {
                for (j = 0; j <= slices; j++)
                {
                    float k = (float) j / slices;
                    float s = fsinf(V_PI * k * 2.0);
                    float c = fcosf(V_PI * k * 2.0);

                    elementsVertex3f(s * c0, c * c0, s0);
                    elementsVertex3f(s * c1, c * c1, s1);
                }
            }
            elementsEnd();
        }
    }
    elementsEndList();
#endif
}

void ball_free(void)
{
    if (elementsIsList(ball_list))
        elementsDeleteLists(ball_list, 1);
#ifdef NO_LIGHT
    if (elementsIsList(ball_list2))
        elementsDeleteLists(ball_list2, 1);
#endif

    if (ball_text)
    {
        glDeleteTextures(1, &ball_text);
	ball_text=0;
    }

    ball_list = ball_list2 = 0;
    ball_text = 0;
}

void ball_draw(void)
{
#ifndef NO_LIGHT
    glPushAttrib(GL_POLYGON_BIT | GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT);
#else
    glPushAttrib(GL_POLYGON_BIT | GL_DEPTH_BUFFER_BIT);
#endif
    {
#ifndef NO_LIGHT
        static const float  s[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        static const float  e[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
        static const float  h[1] = { 64.0f };

        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  s);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  e);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, h);

        glEnable(GL_COLOR_MATERIAL);

        glBindTexture(GL_TEXTURE_2D, ball_text);
        /* Render the ball back to front in case it is translucent. */
        glDepthMask(GL_FALSE);

        glCullFace(GL_FRONT);
        glCallList(ball_list);
        glCullFace(GL_BACK);
        glCallList(ball_list);

        /* Render the ball into the depth buffer. */

        glDepthMask(GL_TRUE);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        {
            glCallList(ball_list);
        }
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        /* Ensure the ball is visible even when obscured by geometry. */

        glDisable(GL_DEPTH_TEST);

        glColor4f(1.0f, 1.0f, 1.0f, 0.1f);
        glCallList(ball_list);
#else
        glDepthMask(GL_FALSE);
	glDepthFunc(GL_ALWAYS);
	glDisable(GL_TEXTURE_2D);
        glColor4f(1.0f, 0.9f, 0.9f, 0.2f);
        elementsCallList(ball_list2);

	glDepthFunc(GL_LESS);
        glColor4f(1.0f, 0.9f, 0.9f, 0.3f);
        elementsCallList(ball_list2);

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glDepthMask(GL_TRUE);
	glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, ball_text);
        glCullFace(GL_FRONT);
        elementsCallList(ball_list);
        glCullFace(GL_BACK);
        elementsCallList(ball_list);
#endif
    }
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

static GLuint mark_list;

void mark_init(int b)
{
    int i, slices = b ? 32 : 16;

    mark_list = glGenLists(1);
    
    glNewList(mark_list, GL_COMPILE);
    {
        glBegin(GL_TRIANGLE_FAN);
        {
#ifndef NO_LIGHT
            glNormal3f(0.f, 1.f, 0.f);
#endif

            for (i = 0; i < slices; i++)
            {
                float x = fcosf(-2.f * PI * i / slices);
                float y = fsinf(-2.f * PI * i / slices);

                glVertex3f(x, 0, y);
            }
        }
        glEnd();
    }
    glEndList();
}

void mark_draw(void)
{
#ifndef NO_LIGHT
    glPushAttrib(GL_TEXTURE_BIT | GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT);
#else
    glPushAttrib(GL_TEXTURE_BIT | GL_DEPTH_BUFFER_BIT);
#endif
    {
#ifndef NO_LIGHT
        glEnable(GL_COLOR_MATERIAL);
#endif
        glDisable(GL_TEXTURE_2D);
        glDepthMask(GL_FALSE);

        glCallList(mark_list);
    }
    glPopAttrib();
}

void mark_free(void)
{
    if (glIsList(mark_list))
        glDeleteLists(mark_list, 1);

    mark_list = 0;
}

/*---------------------------------------------------------------------------*/

static GLuint coin_text;
static GLuint coin_list;

static void coin_head(int n, float radius, float thick)
{
    int i;

    elementsBegin(GL_TRIANGLE_FAN);
    {
#ifndef NO_LIGHT
        glNormal3f(0.f, 0.f, +1.f);
#endif

        for (i = 0; i < n; i++)
        {
            float x = fcosf(+2.f * PI * i / n);
            float y = fsinf(+2.f * PI * i / n);

            elementsTexCoord2f(-x * 0.5f + 0.5f, +y * 0.5f + 0.5f);
            elementsVertex3f(radius * x, radius * y, +thick);
        }
    }
    elementsEnd();
}

static void coin_tail(int n, float radius, float thick)
{
    int i;

    elementsBegin(GL_TRIANGLE_FAN);
    {
#ifndef NO_LIGHT
        glNormal3f(0.f, 0.f, -1.f);
#endif

        for (i = 0; i < n; i++)
        {
            float x = fcosf(-2.f * PI * i / n);
            float y = fsinf(-2.f * PI * i / n);

            elementsTexCoord2f(+x * 0.5f + 0.5f, +y * 0.5f + 0.5f);
            elementsVertex3f(radius * x, radius * y, -thick);
        }
    }
    elementsEnd();
}

static void coin_edge(int n, float radius, float thick)
{
    int i;

    elementsBegin(GL_QUAD_STRIP);
    {
        for (i = 0; i <= n; i++)
        {
            float x = fcosf(2.f * PI * i / n);
            float y = fsinf(2.f * PI * i / n);

#ifndef NO_LIGHT
            glNormal3f(x, y, 0.f);
#endif
            elementsVertex3f(radius * x, radius * y, +thick);
            elementsVertex3f(radius * x, radius * y, -thick);
        }
    }
    elementsEnd();
}

static float coin_color_array_0[3]={1.0f,1.0f,0.2f};
static float coin_color_array_1[3]={1.0f,0.2f,0.2f};
static float coin_color_array_2[3]={0.2f,0.2f,1.0f};

static float *coin_color_array[16]=
{
	(float *)&coin_color_array_0[0], // 0
	(float *)&coin_color_array_0[0], // 1 
	(float *)&coin_color_array_0[0], // 2
	(float *)&coin_color_array_0[0], // 3
	(float *)&coin_color_array_0[0], // 4
	(float *)&coin_color_array_1[0], // 5
	(float *)&coin_color_array_1[0], // 6
	(float *)&coin_color_array_1[0], // 7
	(float *)&coin_color_array_1[0], // 8
	(float *)&coin_color_array_1[0], // 9
	(float *)&coin_color_array_2[0], // 10
	(float *)&coin_color_array_2[0], // 11
	(float *)&coin_color_array_2[0], // 12
	(float *)&coin_color_array_2[0], // 13
	(float *)&coin_color_array_2[0], // 14
	(float *)&coin_color_array_2[0], // 15
};

void coin_color(float *c, int n)
{
    if (n >= 1)
    {
        c[0] = 1.0f;
        c[1] = 1.0f;
        c[2] = 0.2f;
    }
    if (n >= 5)
    {
        c[0] = 1.0f;
        c[1] = 0.2f;
        c[2] = 0.2f;
    }
    if (n >= 10)
    {
        c[0] = 0.2f;
        c[1] = 0.2f;
        c[2] = 1.0f;
    }
}

void coin_init(int b)
{
    char name[MAXSTR];
    int n = b ? 32 : 12; //8;

    config_get_s(CONFIG_COIN, name, MAXSTR);

    coin_text = make_image_from_file(NULL, NULL, NULL, NULL, name);
    coin_list = elementsGenLists(1);

    elementsNewList(coin_list, GL_COMPILE);
    {
    	elementsBindTexture(GL_TEXTURE_2D, coin_text);
        coin_edge(n, COIN_RADIUS, COIN_THICK);
        coin_head(n, COIN_RADIUS, COIN_THICK);
        coin_tail(n, COIN_RADIUS, COIN_THICK);
    }
    elementsEndList();
}

void coin_free(void)
{
    if (elementsIsList(coin_list))
        elementsDeleteLists(coin_list, 1);

    if (coin_text)
    {
        glDeleteTextures(1, &coin_text);
	coin_text=0;
    }

    coin_list = 0;
    coin_text = 0;
}

void coin_push(void)
{
#ifndef NO_LIGHT
    static const float  a[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    static const float  s[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    static const float  e[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    static const float  h[1] = { 32.0f };

    glPushAttrib(GL_LIGHTING_BIT);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   a);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  s);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  e);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, h);

    glEnable(GL_COLOR_MATERIAL);
#endif
    glBindTexture(GL_TEXTURE_2D, coin_text);
}

void coin_draw(int n, float r)
{
#if 0
    float c[3];

    coin_color(c, n);

    glColor3fv(c);
#else
    glColor3fv(coin_color_array[n&15]);
#endif
    elementsCallList(coin_list);
}

void coin_pull(void)
{
#ifndef NO_LIGHT
    glPopAttrib();
#endif
}

/*---------------------------------------------------------------------------*/

static GLuint goal_list;

void goal_init(int b)
{
    int i, n = b ? 32 : 8;

    goal_list = glGenLists(1);

    glNewList(goal_list, GL_COMPILE);
    {
        {
            glBegin(GL_QUAD_STRIP);
            {
                for (i = 0; i <= n; i++)
                {
                    float x = fcosf(2.f * PI * i / n);
                    float y = fsinf(2.f * PI * i / n);
            
                    glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
                    glVertex3f(x, 0.0f, y);

                    glColor4f(1.0f, 1.0f, 0.0f, 0.0f);
                    glVertex3f(x, GOAL_HEIGHT, y);
                }
            }
            glEnd();
        }
    }
    glEndList();
}

void goal_free(void)
{
    if (glIsList(goal_list))
        glDeleteLists(goal_list, 1);

    goal_list = 0;
}

void goal_draw(void)
{
#ifndef NO_LIGHT
	glPushAttrib(GL_TEXTURE_BIT | GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT);
#else
        glPushAttrib(GL_TEXTURE_BIT | GL_DEPTH_BUFFER_BIT);
#endif
	glDisable(GL_TEXTURE_2D);
	glDepthMask(GL_FALSE);
	glCallList(goal_list);
	glPopAttrib();
}

/*---------------------------------------------------------------------------*/

static GLuint jump_list;

void jump_init(int b)
{
    int i, n = b ? 32 : 8;

    jump_list = glGenLists(1);

    glNewList(jump_list, GL_COMPILE);
    {
        {
            glBegin(GL_QUAD_STRIP);
            {
                for (i = 0; i <= n; i++)
                {
                    float x = fcosf(2.f * PI * i / n);
                    float y = fsinf(2.f * PI * i / n);
            
                    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
                    glVertex3f(x, 0.0f, y);

                    glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
                    glVertex3f(x, JUMP_HEIGHT, y);
                }
            }
            glEnd();
        }
    }
    glEndList();
}

void jump_free(void)
{
    if (glIsList(jump_list))
        glDeleteLists(jump_list, 1);

    jump_list = 0;
}

void jump_draw(void)
{
#ifndef NO_LIGHT
        glPushAttrib(GL_TEXTURE_BIT | GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
#else
        glPushAttrib(GL_TEXTURE_BIT | GL_DEPTH_BUFFER_BIT);
#endif
	glDisable(GL_TEXTURE_2D);
	glDepthMask(GL_FALSE);

	glCallList(jump_list);
        glPopAttrib();
}

/*---------------------------------------------------------------------------*/

static GLuint swch_list;

void swch_init(int b)
{
    int i, n = b ? 32 : 8;

    swch_list = glGenLists(2);

    /* Create the ON display list. */

    glNewList(swch_list, GL_COMPILE);
    {
        {
            glBegin(GL_QUAD_STRIP);
            {
                for (i = 0; i <= n; i++)
                {
                    float x = fcosf(2.f * PI * i / n);
                    float y = fsinf(2.f * PI * i / n);
            
                    glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
                    glVertex3f(x, 0.0f, y);

                    glColor4f(1.0f, 0.0f, 0.0f, 0.0f);
                    glVertex3f(x, SWCH_HEIGHT, y);
                }
            }
            glEnd();
        }
    }
    glEndList();

    /* Create the OFF display list. */

    glNewList(swch_list + 1, GL_COMPILE);
    {
        {
            glBegin(GL_QUAD_STRIP);
            {
                for (i = 0; i <= n; i++)
                {
                    float x = fcosf(2.f * PI * i / n);
                    float y = fsinf(2.f * PI * i / n);
            
                    glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
                    glVertex3f(x, 0.0f, y);

                    glColor4f(0.0f, 1.0f, 0.0f, 0.0f);
                    glVertex3f(x, SWCH_HEIGHT, y);
                }
            }
            glEnd();
        }
    }
    glEndList();
}

void swch_free(void)
{
    if (glIsList(swch_list))
        glDeleteLists(swch_list, 2);

    swch_list = 0;
}

void swch_draw(int b)
{
#ifndef NO_LIGHT
        glPushAttrib(GL_TEXTURE_BIT | GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
#else
        glPushAttrib(GL_TEXTURE_BIT | GL_DEPTH_BUFFER_BIT);
#endif
	glDisable(GL_TEXTURE_2D);
	glDepthMask(GL_FALSE);

	if (b)
		glCallList(swch_list + 1);
	else
		glCallList(swch_list);

	glPopAttrib();
}

/*---------------------------------------------------------------------------*/

static GLuint flag_list;

void flag_init(int b)
{
    int i, n = b ? 8 : 4;

    flag_list = glGenLists(1);

    glNewList(flag_list, GL_COMPILE);
    {
        {
            glBegin(GL_QUAD_STRIP);
            {
                for (i = 0; i <= n; i++)
                {
                    float x = fcosf(2.f * PI * i / n) * 0.01f;
                    float y = fsinf(2.f * PI * i / n) * 0.01f;
            
                    glColor3f(1.0f, 1.0f, 1.0f);
                    glVertex3f(x, 0.0f,        y);
                    glVertex3f(x, GOAL_HEIGHT, y);
                }
            }
            glEnd();

            glBegin(GL_TRIANGLES);
            {
                glColor3f(1.0f, 0.0f, 0.0f);

                glVertex3f(              0.0f, GOAL_HEIGHT,        0.0f);
                glVertex3f(GOAL_HEIGHT * 0.2f, GOAL_HEIGHT * 0.9f, 0.0f);
                glVertex3f(              0.0f, GOAL_HEIGHT * 0.8f, 0.0f);

                glVertex3f(              0.0f, GOAL_HEIGHT,        0.0f);
                glVertex3f(              0.0f, GOAL_HEIGHT * 0.8f, 0.0f);
                glVertex3f(GOAL_HEIGHT * 0.2f, GOAL_HEIGHT * 0.9f, 0.0f);
            }
            glEnd();
        }
    }
    glEndList();
}

void flag_free(void)
{
    if (glIsList(flag_list))
        glDeleteLists(flag_list, 1);

    flag_list = 0;
}

void flag_draw(void)
{
#ifndef NO_LIGHT
        glPushAttrib(GL_TEXTURE_BIT | GL_LIGHTING_BIT);
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
#else
        glPushAttrib(GL_TEXTURE_BIT);
#endif
	glDisable(GL_TEXTURE_2D);
	glCallList(flag_list);
        glPopAttrib();
}

/*---------------------------------------------------------------------------*/
/*
 * A note about lighting and shadow: technically speaking, it's wrong.
 * The  light  position  and   shadow  projection  behave  as  if  the
 * light-source rotates with the  floor.  However, the skybox does not
 * rotate, thus the light should also remain stationary.
 *
 * The  correct behavior  would eliminate  a significant  3D  cue: the
 * shadow of  the ball indicates  the ball's position relative  to the
 * floor even  when the ball is  in the air.  This  was the motivating
 * idea  behind the  shadow  in  the first  place,  so correct  shadow
 * projection would only magnify the problem.
 */

static GLuint shad_text;

void shad_init(void)
{
    shad_text = make_image_from_file(NULL, NULL, NULL, NULL, IMG_SHAD);

    if (config_get_d(CONFIG_SHADOW) == 2)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#ifdef DREAMCAST
        glTexEnvi(GL_TEXTURE_2D,GL_TEXTURE_ENV_MODE,GL_MODULATEALPHA);
#endif
    }
}

void shad_free(void)
{
    if (shad_text)
    {
        glDeleteTextures(1, &shad_text);
	shad_text=0;
    }
}

void shad_draw_set(const float *p, float r)
{
    glMatrixMode(GL_TEXTURE);
    {
        float k = 0.25f / r;

        glBindTexture(GL_TEXTURE_2D, shad_text);

        glLoadIdentity();
        glTranslatef(0.5f - k * p[0],
                     0.5f - k * p[2], 0.f);
        glScalef(k, k, 1.0f);
    }
    glMatrixMode(GL_MODELVIEW);
}

void shad_draw_clr(void)
{
    glMatrixMode(GL_TEXTURE);
    {
        glLoadIdentity();
    }
    glMatrixMode(GL_MODELVIEW);
}

/*---------------------------------------------------------------------------*/

void fade_draw(float k)
{
    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);

    if (k > 0.0f)
    {
        config_push_ortho();
#ifndef NO_LIGHT
        glPushAttrib(GL_TEXTURE_BIT | GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
        glPushAttrib(GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
        {
#ifndef NO_LIGHT
            glEnable(GL_COLOR_MATERIAL);
            glDisable(GL_LIGHTING);
#endif
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_TEXTURE_2D);
            
            glColor4f(0.0f, 0.0f, 0.0f, k);

            glBegin(GL_QUADS);
            {
                glVertex2i(0, 0);
                glVertex2i(w, 0);
                glVertex2i(w, h);
                glVertex2i(0, h);
            }
            glEnd();
        }
        glPopAttrib();
        config_pop_matrix();
    }
}

/*---------------------------------------------------------------------------*/
