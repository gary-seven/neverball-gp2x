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
#include "part.h"
#include "vec3.h"
#include "image.h"
#include "elements.h"

/*---------------------------------------------------------------------------*/

struct part
{
    float t;
    float a;
    float w;
    float c[3];
    float p[3];
    float v[3];
};

static struct part part_coin[PART_MAX_COIN];
static struct part part_goal[PART_MAX_GOAL];
static GLuint      part_text;
static GLuint      part_list;

/*---------------------------------------------------------------------------*/

#define PI 3.1415927f

static float rnd(float l, float h)
{
    return l + (h - l) * rand() / RAND_MAX;
}

/*---------------------------------------------------------------------------*/

void part_reset(float h)
{
    int i;

    for (i = 0; i < PART_MAX_GOAL; i++)
    {
        float t = rnd(+0.1f,      +1.0f);
        float a = rnd(-1.0f * PI, +1.0f * PI);
        float w = rnd(-2.0f * PI, +2.0f * PI);

        part_goal[i].t = t;
        part_goal[i].a = V_DEG(a);
        part_goal[i].w = V_DEG(w);

        part_goal[i].c[0] = 1.0f;
        part_goal[i].c[1] = 1.0f;
        part_goal[i].c[2] = 0.0f;

        part_goal[i].p[0] = fsinf(a);
        part_goal[i].p[1] = (1.f - t) * h;
        part_goal[i].p[2] = fcosf(a);

        part_goal[i].v[0] = 0.f;
        part_goal[i].v[1] = 0.f;
        part_goal[i].v[2] = 0.f;

        part_coin[i].t    = 0.0f;
    }
}

void part_init(float h)
{
    memset(part_coin, 0, PART_MAX_COIN * sizeof (struct part));
    memset(part_goal, 0, PART_MAX_GOAL * sizeof (struct part));

    part_text = make_image_from_file(NULL, NULL, NULL, NULL, IMG_PART);
    part_list = elementsGenLists(1);

    elementsNewList(part_list, GL_COMPILE);
    {
        elementsBindTexture(GL_TEXTURE_2D, part_text);
        elementsBegin(GL_QUADS);
        {
            elementsTexCoord2f(0.f, 0.f);
            elementsVertex2f(-PART_SIZE, -PART_SIZE);

            elementsTexCoord2f(1.f, 0.f);
            elementsVertex2f(+PART_SIZE, -PART_SIZE);

            elementsTexCoord2f(1.f, 1.f);
            elementsVertex2f(+PART_SIZE, +PART_SIZE);

            elementsTexCoord2f(0.f, 1.f);
            elementsVertex2f(-PART_SIZE, +PART_SIZE);
        }
        elementsEnd();
    }
    elementsEndList();

    part_reset(h);
}

void part_free(void)
{
    if (elementsIsList(part_list))
        elementsDeleteLists(part_list, 1);

    if (part_text)
    {
        glDeleteTextures(1, &part_text);
	part_text=0;
    }
}

/*---------------------------------------------------------------------------*/

void part_burst(const float *p, const float *c)
{
    int i, n = 0;

    for (i = 0; n < 10 && i < PART_MAX_COIN; i++)
        if (part_coin[i].t <= 0.f)
        {
            float a = rnd(-1.0f * PI, +1.0f * PI);
            float b = rnd(+0.3f * PI, +0.5f * PI);
            float w = rnd(-4.0f * PI, +4.0f * PI);

            part_coin[i].p[0] = p[0];
            part_coin[i].p[1] = p[1];
            part_coin[i].p[2] = p[2];

            part_coin[i].v[0] = 4.f * fcosf(a) * fcosf(b);
            part_coin[i].v[1] = 4.f *            fsinf(b);
            part_coin[i].v[2] = 4.f * fsinf(a) * fcosf(b);

            part_coin[i].c[0] = c[0];
            part_coin[i].c[1] = c[1];
            part_coin[i].c[2] = c[2];

            part_coin[i].t = 1.f;
            part_coin[i].a = 0.f;
            part_coin[i].w = V_DEG(w);

            n++;
        }
}

/*---------------------------------------------------------------------------*/

static void part_fall(struct part *part, int n, const float *g, float dt)
{
    int i;

    for (i = 0; i < n; i++)
        if (part[i].t > 0.f)
        {
            part[i].t -= dt;

            part[i].v[0] += (g[0] * dt);
            part[i].v[1] += (g[1] * dt);
            part[i].v[2] += (g[2] * dt);

            part[i].p[0] += (part[i].v[0] * dt);
            part[i].p[1] += (part[i].v[1] * dt);
            part[i].p[2] += (part[i].v[2] * dt);
        }
}

static void part_spin(struct part *part, int n, const float *g, float dt)
{
    int i;

    for (i = 0; i < n; i++)
        if (part[i].t > 0.f)
        {
            part[i].a += 30.f * dt;

            part[i].p[0] = fsinf(V_RAD(part[i].a));
            part[i].p[2] = fcosf(V_RAD(part[i].a));
        }
}

void part_step(const float *g, float dt)
{
    part_fall(part_coin, PART_MAX_COIN, g, dt);

    if (g[1] > 0.f)
        part_fall(part_goal, PART_MAX_GOAL, g, dt);
    else
        part_spin(part_goal, PART_MAX_GOAL, g, dt);
}

/*---------------------------------------------------------------------------*/

static void part_draw(const float p[3], const float c[3],
                      float a, float r, float rx, float ry, float rz)
{
    glPushMatrix();
    {
        glTranslatef(r * p[0], p[1], r * p[2]);
        glRotatef(ry, 0.f, 1.f, 0.f);
        glRotatef(rx, 1.f, 0.f, 0.f);
        glRotatef(rz, 0.f, 0.f, 1.f);

        glColor4f(c[0], c[1], c[2], a);

        elementsCallList(part_list);
    }
    glPopMatrix();
}

void part_draw_coin(float rx, float ry)
{
    float r = (float) SDL_GetTicks() / 1000.f;
    int i;

#ifndef NO_LIGHT
    glPushAttrib(GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT);
#else
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
#endif
    {
#ifndef NO_LIGHT
        glDisable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
#endif

        glDepthMask(GL_FALSE);
        glBindTexture(GL_TEXTURE_2D, part_text);

        for (i = 0; i < PART_MAX_COIN; i++)
            if (part_coin[i].t > 0.f)
                part_draw(part_coin[i].p,
                          part_coin[i].c,
                          part_coin[i].t,
                          1.f, rx, ry, r * part_coin[i].w);
    }
    glPopAttrib();
}

void part_draw_goal(float rx, float ry, float radius, float a)
{
    float r = (float) SDL_GetTicks() / 1000.f;
    int i;

#ifndef NO_LIGHT
    glPushAttrib(GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT);
#else
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
#endif
    {
#ifndef NO_LIGHT
        glDisable(GL_LIGHTING);

        glEnable(GL_COLOR_MATERIAL);
#endif

        glDepthMask(GL_FALSE);
        glBindTexture(GL_TEXTURE_2D, part_text);

        for (i = 0; i < PART_MAX_GOAL; i++)
            if (part_goal[i].t > 0.f)
                part_draw(part_goal[i].p,
                          part_goal[i].c, a,
                          radius - 0.05f, rx, ry, r * part_goal[i].w);
    }
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/
