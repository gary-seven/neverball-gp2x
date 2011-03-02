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

#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <string.h>
#include <math.h>

#include "glext.h"
#include "image.h"
#include "config.h"

/*---------------------------------------------------------------------------*/

void image_snap(char *filename)
{
    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);
    int i;

    SDL_Surface *buf = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 24,
                                            RMASK, GMASK, BMASK, 0);
    SDL_Surface *img = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 24,
                                            RMASK, GMASK, BMASK, 0);

    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buf->pixels);

    for (i = 0; i < h; i++)
        memcpy((GLubyte *) img->pixels + 3 * w * i,
               (GLubyte *) buf->pixels + 3 * w * (h - i), 3 * w);

    SDL_SaveBMP(img, filename);

    SDL_FreeSurface(img);
    SDL_FreeSurface(buf);
}

void image_size(int *W, int *H, int w, int h)
{
    *W = 1;
    *H = 1;

    while (*W < w) *W *= 2;
    while (*H < h) *H *= 2;
}

/*---------------------------------------------------------------------------*/

void image_swab(SDL_Surface *src)
{
    int i, j, b = (src->format->BitsPerPixel == 32) ? 4 : 3;
    
    SDL_LockSurface(src);
    {
        unsigned char *s = (unsigned char *) src->pixels;
        unsigned char  t;

        /* Iterate over each pixel of the image. */

        for (i = 0; i < src->h; i++)
            for (j = 0; j < src->w; j++)
            {
                int k = (i * src->w + j) * b;

                /* Swap the red and blue channels of each. */

                t        = s[k + 2];
                s[k + 2] = s[k + 0];
                s[k + 0] =        t;
            }
    }
    SDL_UnlockSurface(src);
}

void image_white(SDL_Surface *src)
{
    int i, j, b = (src->format->BitsPerPixel == 32) ? 4 : 3;
    
    SDL_LockSurface(src);
    {
        unsigned char *s = (unsigned char *) src->pixels;

        /* Iterate over each pixel of the image. */

        for (i = 0; i < src->h; i++)
            for (j = 0; j < src->w; j++)
            {
                int k = (i * src->w + j) * b;

                /* Whiten the RGB channels without touching any Alpha. */

                s[k + 0] = 0xFF;
                s[k + 1] = 0xFF;
                s[k + 2] = 0xFF;
            }
    }
    SDL_UnlockSurface(src);
}

SDL_Surface *image_scale(SDL_Surface *src, int n)
{
    int si, di;
    int sj, dj;
    int k, b = (src->format->BitsPerPixel == 32) ? 4 : 3;

    SDL_Surface *dst = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                            src->w / n,
                                            src->h / n,
                                            src->format->BitsPerPixel,
                                            src->format->Rmask,
                                            src->format->Gmask,
                                            src->format->Bmask,
                                            src->format->Amask);
    if (dst)
    {
        SDL_LockSurface(src);
        SDL_LockSurface(dst);
        {
            unsigned char *s = (unsigned char *) src->pixels;
            unsigned char *d = (unsigned char *) dst->pixels;

            /* Iterate each component of each distination pixel. */

            for (di = 0; di < src->h / n; di++)
                for (dj = 0; dj < src->w / n; dj++)
                    for (k = 0; k < b; k++)
                    {
                        int c = 0;

                        /* Average the NxN source pixel block for each. */

                        for (si = di * n; si < (di + 1) * n; si++)
                            for (sj = dj * n; sj < (dj + 1) * n; sj++)
                                c += s[(si * src->w + sj) * b + k];

                        d[(di * dst->w + dj) * b + k] =
                            (unsigned char) (c / (n * n));
                    }
        }
        SDL_UnlockSurface(dst);
        SDL_UnlockSurface(src);
    }

    return dst;
}

/*---------------------------------------------------------------------------*/

/*
 * Create on  OpenGL texture  object using the  given SDL  surface and
 * format,  scaled  using the  current  scale  factor.  When  scaling,
 * assume dimensions are used only for layout and lie about the size.
 */
GLuint make_image_from_surf(int *w, int *h, SDL_Surface *s)
{
    int    t = config_get_d(CONFIG_TEXTURES);
    GLuint o = 0;

    glGenTextures(1, &o);
    glBindTexture(GL_TEXTURE_2D, o);

#if 0
    if (t > 1)
    {
        SDL_Surface *d = image_scale(s, t);

        /* Load the scaled image. */

        if (d->format->BitsPerPixel == 32)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, d->w, d->h, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, d->pixels);
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,  d->w, d->h, 0,
                         GL_RGB,  GL_UNSIGNED_BYTE, d->pixels);

        SDL_FreeSurface(d);
    }
    else
#endif
    {
        /* Load the unscaled image. */

        if (s->format->BitsPerPixel == 32)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s->w, s->h, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,  s->w, s->h, 0,
                         GL_RGB,  GL_UNSIGNED_BYTE, s->pixels);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (w) *w = s->w;
    if (h) *h = s->h;

    return o;
}

/*---------------------------------------------------------------------------*/

/*
 * Load  an image  from the  named file.   If the  image is  not RGBA,
 * convert it to RGBA.  Return an OpenGL texture object.
 */
GLuint make_image_from_file(int *W, int *H,
                            int *w, int *h, const char *name)
{
    SDL_Surface *src=NULL;
    SDL_Surface *dst;
    SDL_Rect rect;
    int len=strlen(name);

    GLuint o = 0;

    /* Load the file. */

    if (len>4)
	    if (name[len-4]=='.')
	    {
		    char *n=(char *)calloc(1,len+1);
		    strncpy(n,name,len);
		    n[len-3]='b'; n[len-2]='m'; n[len-1]='p';
		    src=SDL_LoadBMP(config_data(n));
		    if (src)
		    {
			    int i,len=src->w*src->h;
			    unsigned char *b=(unsigned char *)src->pixels;
			    for(i=0;i<len;i++)
			    {
				    register char t=b[0];
				    b[0]=b[2];
				    b[2]=t;
				    b+=3;
			    }
		    }
		    else
		    {
		    	n[len-3]='p'; n[len-2]='n'; n[len-1]='g';
		    	src=IMG_Load(config_data(n));
		    }
		    free(n);
	    }
    if (!src)
		src = IMG_Load(config_data(name));
    if (src)
    {
        int w2;
        int h2;

        image_size(&w2, &h2, src->w, src->h);

        if (w) *w = src->w;
        if (h) *h = src->h;

        /* Create a new destination surface. */
        
        if ((dst = SDL_CreateRGBSurface(SDL_SWSURFACE, w2, h2, 32,
                                        RMASK, GMASK, BMASK, AMASK)))
        {
            /* Copy source pixels to the center of the destination. */

            rect.x = (Sint16) (w2 - src->w) / 2;
            rect.y = (Sint16) (h2 - src->h) / 2;

            SDL_SetAlpha(src, 0, 0);
            SDL_BlitSurface(src, NULL, dst, &rect);

            o = make_image_from_surf(W, H, dst);

            SDL_FreeSurface(dst);
        }
        SDL_FreeSurface(src);
    }
    return o;
}

/*---------------------------------------------------------------------------*/

/*
 * Render the given  string using the given font.   Transfer the image
 * to a  surface of  power-of-2 size large  enough to fit  the string.
 * Return an OpenGL texture object.
 */
GLuint make_image_from_font(int *W, int *H,
                            int *w, int *h, const char *text, TTF_Font *font, int k)
{
    SDL_Color fg = { 0xFF, 0xFF, 0xFF, 0xFF };

    SDL_Surface *src;
    SDL_Surface *dst;
    SDL_Rect rect;

    GLuint o = 0;

    /* Render the text. */

    if (text && strlen(text) > 0)
    {
// #ifdef DREAMCAST
#if 0
        if ((src = TTF_RenderText_Solid(font, text, fg)))
#else
        if ((src = TTF_RenderText_Blended(font, text, fg)))
#endif
        {
            int w2;
            int h2;

            image_size(&w2, &h2, src->w, src->h);

            if (w) *w = src->w;
            if (h) *h = src->h;

            /* Create a new destination surface. */
            
            if ((dst = SDL_CreateRGBSurface(SDL_SWSURFACE, w2, h2, 32,
                                            RMASK, GMASK, BMASK, AMASK)))
            {
                /* Copy source pixels to the center of the destination. */

                rect.x = (Sint16) (w2 - src->w) / 2;
                rect.y = (Sint16) (h2 - src->h) / 2;

                SDL_SetAlpha(src, 0, 0);
                SDL_BlitSurface(src, NULL, dst, &rect);

                image_white(dst);

                o = make_image_from_surf(W, H, dst);

                SDL_FreeSurface(dst);
            }
            SDL_FreeSurface(src);
        }

        if (W) *W *= k;
        if (H) *H *= k;
        if (w) *w *= k;
        if (h) *h *= k;
    }
    else
    {
        if (W) *W = 0;
        if (H) *H = 0;
        if (w) *w = 0;
        if (h) *h = 0;
    }

    return o;
}

/*---------------------------------------------------------------------------*/
