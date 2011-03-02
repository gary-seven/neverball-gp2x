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

/*---------------------------------------------------------------------------*/

#ifdef WIN32
#pragma comment(lib, "SDL_ttf.lib")
#pragma comment(lib, "SDL_image.lib")
#pragma comment(lib, "SDL_mixer.lib")
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")
#pragma comment(lib, "opengl32.lib")
#endif

/*---------------------------------------------------------------------------*/

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vec3.h"
#include "glext.h"
#include "solid.h"
#include "config.h"

#define MAXSTR 256
#define MAXKEY 16
#define SCALE  64.f
#define SMALL  0.0005f

/*
 * The overall design  of this map converter is  very stupid, but very
 * simple. It  begins by assuming  that every mtrl, vert,  edge, side,
 * and texc  in the map is  unique.  It then makes  an optimizing pass
 * that discards redundant information.  The result is optimal, though
 * the process is terribly inefficient.
 */

/*---------------------------------------------------------------------------*/

/* Ohhhh... arbitrary! */

#define MAXM	256
#define MAXV    32767
#define MAXE	32767
#define MAXS	32767
#define MAXT	32767
#define MAXG	32767
#define MAXL	1024
#define MAXN	1024
#define MAXP	512
#define MAXB	512
#define MAXC	1024
#define MAXZ    16
#define MAXJ	32
#define MAXX	16
#define MAXR	1024
#define MAXU	16
#define MAXW    32
#define MAXA	512
#define MAXI	32767

static int overflow(const char *s)
{
    printf("%s overflow\n", s);
    exit(1);
    return 0;
}

static int incm(struct s_file *fp)
{
    return (fp->mc < MAXM) ? fp->mc++ : overflow("mtrl");
}

static int incv(struct s_file *fp)
{
    return (fp->vc < MAXV) ? fp->vc++ : overflow("vert");
}

static int ince(struct s_file *fp)
{
    return (fp->ec < MAXE) ? fp->ec++ : overflow("edge");
}

static int incs(struct s_file *fp)
{
    return (fp->sc < MAXS) ? fp->sc++ : overflow("side");
}

static int inct(struct s_file *fp)
{
    return (fp->tc < MAXT) ? fp->tc++ : overflow("texc");
}

static int incg(struct s_file *fp)
{
    return (fp->gc < MAXG) ? fp->gc++ : overflow("geom");
}

static int incl(struct s_file *fp)
{
    return (fp->lc < MAXL) ? fp->lc++ : overflow("lump");
}

static int incn(struct s_file *fp)
{
    return (fp->nc < MAXN) ? fp->nc++ : overflow("node");
}

static int incp(struct s_file *fp)
{
    return (fp->pc < MAXP) ? fp->pc++ : overflow("path");
}

static int incb(struct s_file *fp)
{
    return (fp->bc < MAXB) ? fp->bc++ : overflow("body");
}

static int incc(struct s_file *fp)
{
    return (fp->cc < MAXC) ? fp->cc++ : overflow("coin");
}

static int incz(struct s_file *fp)
{
    return (fp->zc < MAXZ) ? fp->zc++ : overflow("geol");
}

static int incj(struct s_file *fp)
{
    return (fp->jc < MAXJ) ? fp->jc++ : overflow("jump");
}

static int incx(struct s_file *fp)
{
    return (fp->xc < MAXX) ? fp->xc++ : overflow("swch");
}

static int incr(struct s_file *fp)
{
    return (fp->rc < MAXR) ? fp->rc++ : overflow("bill");
}

static int incu(struct s_file *fp)
{
    return (fp->uc < MAXU) ? fp->uc++ : overflow("ball");
}

static int incw(struct s_file *fp)
{
    return (fp->wc < MAXW) ? fp->wc++ : overflow("view");
}

static int inci(struct s_file *fp)
{
    return (fp->ic < MAXI) ? fp->ic++ : overflow("indx");
}

static void init_file(struct s_file *fp)
{
    fp->mc = 0;
    fp->vc = 0;
    fp->ec = 0;
    fp->sc = 0;
    fp->tc = 0;
    fp->gc = 0;
    fp->lc = 0;
    fp->nc = 0;
    fp->pc = 0;
    fp->bc = 0;
    fp->cc = 0;
    fp->zc = 0;
    fp->jc = 0;
    fp->xc = 0;
    fp->rc = 0;
    fp->uc = 0;
    fp->wc = 0;
    fp->ac = 0;
    fp->ic = 0;

    fp->mv = (struct s_mtrl *) calloc(MAXM, sizeof (struct s_mtrl));
    fp->vv = (struct s_vert *) calloc(MAXV, sizeof (struct s_vert));
    fp->ev = (struct s_edge *) calloc(MAXE, sizeof (struct s_edge));
    fp->sv = (struct s_side *) calloc(MAXS, sizeof (struct s_side));
    fp->tv = (struct s_texc *) calloc(MAXT, sizeof (struct s_texc));
    fp->gv = (struct s_geom *) calloc(MAXG, sizeof (struct s_geom));
    fp->lv = (struct s_lump *) calloc(MAXL, sizeof (struct s_lump));
    fp->nv = (struct s_node *) calloc(MAXN, sizeof (struct s_node));
    fp->pv = (struct s_path *) calloc(MAXP, sizeof (struct s_path));
    fp->bv = (struct s_body *) calloc(MAXB, sizeof (struct s_body));
    fp->cv = (struct s_coin *) calloc(MAXC, sizeof (struct s_coin));
    fp->zv = (struct s_goal *) calloc(MAXZ, sizeof (struct s_goal));
    fp->jv = (struct s_jump *) calloc(MAXJ, sizeof (struct s_jump));
    fp->xv = (struct s_swch *) calloc(MAXX, sizeof (struct s_swch));
    fp->rv = (struct s_bill *) calloc(MAXR, sizeof (struct s_bill));
    fp->uv = (struct s_ball *) calloc(MAXU, sizeof (struct s_ball));
    fp->wv = (struct s_view *) calloc(MAXW, sizeof (struct s_view));
    fp->av = (char          *) calloc(MAXA, sizeof (char));
    fp->iv = (short         *) calloc(MAXI, sizeof (short));
}

/*---------------------------------------------------------------------------*/

/*
 * The following is a small  symbol table data structure.  Symbols and
 * their integer  values are collected  in symv and  valv.  References
 * and pointers  to their unsatisfied integer values  are collected in
 * refv and pntv.  The resolve procedure matches references to symbols
 * and fills waiting ints with the proper values.
 */

#define MAXSYM 1024

static char   symv[MAXSYM][MAXSTR];
static short  valv[MAXSYM];

static char   refv[MAXSYM][MAXSTR];
static short *pntv[MAXSYM];

static int strc;
static int refc;

static void make_sym(const char *s, short  v)
{
    strncpy(symv[strc], s, MAXSTR - 1);
    valv[strc] = v;
    strc++;
}

static void make_ref(const char *r, short *p)
{
    strncpy(refv[refc], r, MAXSTR - 1);
    pntv[refc] = p;
    refc++;
}

static void resolve(void)
{
    int i, j;

    for (i = 0; i < refc; i++)
        for (j = 0; j < strc; j++)
            if (strncmp(refv[i], symv[j], MAXSTR) == 0)
            {
                *(pntv[i]) = valv[j];
                break;
            }
}

/*---------------------------------------------------------------------------*/

/*
 * The following globals are used to cache target_positions.  They are
 * targeted by various entities and must be resolved in a second pass.
 */

static float targ_p [MAXW][3];
static short targ_wi[MAXW];
static short targ_ji[MAXW];
static short targ_n;

static void targets(struct s_file *fp)
{
    short i;

    for (i = 0; i < fp->wc; i++)
        v_cpy(fp->wv[i].q, targ_p[targ_wi[i]]);

    for (i = 0; i < fp->jc; i++)
        v_cpy(fp->jv[i].q, targ_p[targ_ji[i]]);
}

/*---------------------------------------------------------------------------*/

/*
 * The following code caches  image sizes.  Textures are referenced by
 * name,  but  their  sizes   are  necessary  when  computing  texture
 * coordinates.  This code  allows each file to be  accessed only once
 * regardless of the number of surfaces refering to it.
 */

static char *image_s[MAXM];
static int   image_w[MAXM];
static int   image_h[MAXM];
static int   image_n;

static int size_load(const char *file, int *w, int *h)
{
    SDL_Surface *src=NULL;
    int len=strlen(file);

    if (len>4)
	    if (file[len-4]=='.')
	    {
		    char *n=(char *)calloc(1,len+1);
		    strncpy(n,file,len);
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
		    free(n);
	    }
    if (!src)
	src = IMG_Load(file);
    if (src)
    {
        *w = src->w;
        *h = src->h;

        SDL_FreeSurface(src);

        return 1;
    }
    return 0;
}

static void size_image(const char *name, int *w, int *h)
{
    char jpg[MAXSTR];
    char tga[MAXSTR];
    char png[MAXSTR];
    int i;

    for (i = 0; i < image_n; i++)
        if (strncmp(image_s[i], name, MAXSTR) == 0)
        {
            *w = image_w[i];
            *h = image_h[i];

            return;
        }

    *w = 0;
    *h = 0;

    strcpy(jpg, name); strcat(jpg, ".jpg");
    strcpy(tga, name); strcat(tga, ".tga");
    strcpy(png, name); strcat(png, ".png");

    if (size_load(config_data(png), w, h) ||
        size_load(config_data(tga), w, h) ||
        size_load(config_data(jpg), w, h))
    {
        image_s[image_n] = (char *) calloc(strlen(name) + 1, 1);
        image_w[image_n] = *w;
        image_h[image_n] = *h;

        strcpy(image_s[image_n], name);
        image_n++;
    }
}

/*---------------------------------------------------------------------------*/

/* Read the given material file, adding a new material to the solid.  */

static int read_mtrl(struct s_file *fp, const char *name)
{
    struct s_mtrl *mp;
    FILE *fin;
    short mi;

    for (mi = 0; mi < fp->mc; mi++)
        if (strncmp(name, fp->mv[mi].f, MAXSTR) == 0)
            return mi;

    mp = fp->mv + incm(fp);

    strncpy(mp->f, name, PATHMAX - 1);

    mp->a[0] = mp->a[1] = mp->a[2] = mp->a[3] = 1.0f;
    mp->d[0] = mp->d[1] = mp->d[2] = mp->d[3] = 1.0f;
    mp->s[0] = mp->s[1] = mp->s[2] = mp->s[3] = 1.0f;
    mp->e[0] = mp->e[1] = mp->e[2] = mp->e[3] = 1.0f;
    mp->h[0] = 0.0f;
    mp->fl   = 0;

    if ((fin = fopen(config_data(name), "r")))
    {
        fscanf(fin,
               "%f %f %f %f "
               "%f %f %f %f "
               "%f %f %f %f "
               "%f %f %f %f "
               "%f %hd ",
               mp->d, mp->d + 1, mp->d + 2, mp->d + 3,
               mp->a, mp->a + 1, mp->a + 2, mp->a + 3,
               mp->s, mp->s + 1, mp->s + 2, mp->s + 3,
               mp->e, mp->e + 1, mp->e + 2, mp->e + 3,
               mp->h, &mp->fl);
        fclose(fin);
    }

    return mi;
}

/*---------------------------------------------------------------------------*/

/*
 * All bodies with an associated  path are assumed to be positioned at
 * the  beginning of that  path.  These  bodies must  be moved  to the
 * origin  in order  for their  path transforms  to  behave correctly.
 * This is  how we get away  with defining func_trains  with no origin
 * specification.
 */

static void move_side(struct s_side *sp, const float p[3])
{
    sp->d -= v_dot(sp->n, p);
}

static void move_vert(struct s_vert *vp, const float p[3])
{
    v_sub(vp->p, vp->p, p);
}

static void move_lump(struct s_file *fp,
                      struct s_lump *lp, const float p[3])
{
    short i;

    for (i = 0; i < lp->sc; i++)
        move_side(fp->sv + fp->iv[lp->s0 + i], p);
    for (i = 0; i < lp->vc; i++)
        move_vert(fp->vv + fp->iv[lp->v0 + i], p);
}

static void move_body(struct s_file *fp,
                      struct s_body *bp)
{
    short i;

    for (i = 0; i < bp->lc; i++)
        move_lump(fp, fp->lv + bp->l0 + i, fp->pv[bp->pi].p);
}

static void move_file(struct s_file *fp)
{
    short i;

    for (i = 0; i < fp->bc; i++)
        if (fp->bv[i].pi >= 0)
            move_body(fp, fp->bv + i);
}

/*---------------------------------------------------------------------------*/

static void read_vt(struct s_file *fp, const char *line)
{
    struct s_texc *tp = fp->tv + inct(fp);

    sscanf(line, "%f %f", tp->u, tp->u + 1);
}

static void read_vn(struct s_file *fp, const char *line)
{
    struct s_side *sp = fp->sv + incs(fp);

    sscanf(line, "%f %f %f", sp->n, sp->n + 1, sp->n + 2);
}

static void read_v(struct s_file *fp, const char *line)
{
    struct s_vert *vp = fp->vv + incv(fp);

    sscanf(line, "%f %f %f", vp->p, vp->p + 1, vp->p + 2);
}

static void read_f(struct s_file *fp, const char *line,
                   short v0, short t0, short s0, short mi)
{
    struct s_geom *gp = fp->gv + incg(fp);

    char c1;
    char c2;

    /*
     * FIXME: All faces must be triangles and must include a normal
     * and texture coordinate.
     */

    sscanf(line, "%hd%c%hd%c%hd %hd%c%hd%c%hd %hd%c%hd%c%hd",
           &gp->vi, &c1, &gp->ti, &c2, &gp->si,
           &gp->vj, &c1, &gp->tj, &c2, &gp->sj,
           &gp->vk, &c1, &gp->tk, &c2, &gp->sk);

    gp->vi += (v0 - 1);
    gp->vj += (v0 - 1);
    gp->vk += (v0 - 1);
    gp->ti += (t0 - 1);
    gp->tj += (t0 - 1);
    gp->tk += (t0 - 1);
    gp->si += (s0 - 1);
    gp->sj += (s0 - 1);
    gp->sk += (s0 - 1);
    
    gp->mi  = mi;
}

static void read_obj(struct s_file *fp, const char *name)
{
    char line[MAXSTR];
    char mtrl[MAXSTR];
    FILE *fin;

    short v0 = fp->vc;
    short t0 = fp->tc;
    short s0 = fp->sc;
    short mi = 0;

    if ((fin = fopen(config_data(name), "r")))
    {
        while (fgets(line, MAXSTR, fin))
        {
            if (strncmp(line, "usemtl", 6) == 0)
            {
                sscanf(line + 6, "%s", mtrl);
                mi = read_mtrl(fp, mtrl);
            }

            else if (strncmp(line, "f", 1) == 0)
            {
                if (fp->mv[mi].d[3] > 0)
                    read_f(fp, line + 1, v0, t0, s0, mi);
            }

            else if (strncmp(line, "vt", 2) == 0) read_vt(fp, line + 2);
            else if (strncmp(line, "vn", 2) == 0) read_vn(fp, line + 2);
            else if (strncmp(line, "v",  1) == 0) read_v (fp, line + 1);
        }
        fclose(fin);
    }
}

/*---------------------------------------------------------------------------*/

static float plane_d[MAXS];
static float plane_n[MAXS][3];
static float plane_p[MAXS][3];
static float plane_u[MAXS][3];
static float plane_v[MAXS][3];
static short plane_f[MAXS];
static short plane_m[MAXS];

static void make_plane(short pi, short x0, short y0, short z0,
                       short x1, short y1, short z1,
                       short x2, short y2, short z2,
                       short tu, short tv, short r,
                       float su, float sv, int fl, const char *s)
{
    static const float base[6][3][3] = {
        {{  0,  0,  1 }, {  1,  0,  0 }, {  0, -1,  0 }},
        {{  0,  0, -1 }, {  1,  0,  0 }, {  0, -1,  0 }},
        {{  1,  0,  0 }, {  0,  0, -1 }, {  0, -1,  0 }},
        {{ -1,  0,  0 }, {  0,  0, -1 }, {  0, -1,  0 }},
        {{  0,  1,  0 }, {  1,  0,  0 }, {  0,  0,  1 }},
        {{  0, -1,  0 }, {  1,  0,  0 }, {  0,  0,  1 }},
    };

    float R[16];
    float p0[3], p1[3], p2[3];
    float u[3],  v[3],  p[3];
    float k, d = 0.f;
    short i, n = 0;
    int   w, h;

    size_image(s, &w, &h);

    plane_f[pi] = fl ? L_DETAIL : 0;

    p0[0] = +(float) x0 / SCALE;
    p0[1] = +(float) z0 / SCALE;
    p0[2] = -(float) y0 / SCALE;

    p1[0] = +(float) x1 / SCALE;
    p1[1] = +(float) z1 / SCALE;
    p1[2] = -(float) y1 / SCALE;

    p2[0] = +(float) x2 / SCALE;
    p2[1] = +(float) z2 / SCALE;
    p2[2] = -(float) y2 / SCALE;

    v_sub(u, p0, p1);
    v_sub(v, p2, p1);

    v_crs(plane_n[pi], u, v);
    v_nrm(plane_n[pi], plane_n[pi]);
	
    plane_d[pi] = v_dot(plane_n[pi], p1);

    for (i = 0; i < 6; i++)
        if ((k = v_dot(plane_n[pi], base[i][0])) >= d)
        {
            d = k;
            n = i;
        }

    p[0] = 0.f;
    p[1] = 0.f;
    p[2] = 0.f;

    m_rot(R, base[n][0], V_RAD(r));

    v_mad(p, p, base[n][1], su * tu / SCALE);
    v_mad(p, p, base[n][2], sv * tv / SCALE);

    m_vxfm(plane_u[pi], R, base[n][1]);
    m_vxfm(plane_v[pi], R, base[n][2]);
    m_vxfm(plane_p[pi], R, p);

    v_scl(plane_u[pi], plane_u[pi], 64.f / w);
    v_scl(plane_v[pi], plane_v[pi], 64.f / h);

    v_scl(plane_u[pi], plane_u[pi], 1.f / su);
    v_scl(plane_v[pi], plane_v[pi], 1.f / sv);
}

/*---------------------------------------------------------------------------*/

#define T_EOF 0
#define T_BEG 1
#define T_CLP 2
#define T_KEY 3
#define T_END 4
#define T_NOP 5

static int map_token(FILE *fin, short pi, char key[MAXSTR], char val[MAXSTR])
{
    char buf[MAXSTR];

    if (fgets(buf, MAXSTR, fin))
    {
        char c;
        short x0, y0, z0;
        short x1, y1, z1;
        short x2, y2, z2;
        short tu, tv, r;
        float su, sv;
        int fl;

        /* Scan the beginning or end of a block. */

        if (buf[0] == '{') return T_BEG;
        if (buf[0] == '}') return T_END;

        /* Scan a key-value pair. */

        if (buf[0] == '"')
        {
            strcpy(key, strtok(buf,  "\""));
            (void)      strtok(NULL, "\"");
            strcpy(val, strtok(NULL, "\""));

            return T_KEY;
        }

        /* Scan a plane. */

        if (sscanf(buf,
                   "%c %hd %hd %hd %c "
                   "%c %hd %hd %hd %c "
                   "%c %hd %hd %hd %c "
                   "%s %hd %hd %hd %f %f %d",
                   &c, &x0, &y0, &z0, &c,
                   &c, &x1, &y1, &z1, &c,
                   &c, &x2, &y2, &z2, &c,
                   key, &tu, &tv, &r, &su, &sv, &fl) == 22)
        {
            make_plane(pi, x0, y0, z0,
                       x1, y1, z1,
                       x2, y2, z2,
                       tu, tv, r, su, sv, fl, key);
            return T_CLP;
        }

        /* If it's not recognized, it must be uninteresting. */

        return T_NOP;
    }
    return T_EOF;
}

/*---------------------------------------------------------------------------*/

/* Parse a lump from  the given file and add it to  the solid. */

static void read_lump(struct s_file *fp, FILE *fin)
{
    char k[MAXSTR];
    char v[MAXSTR];
    int t;

    struct s_lump *lp = fp->lv + incl(fp);

    lp->s0 = fp->ic;

    while ((t = map_token(fin, fp->sc, k, v)))
    {
        if (t == T_CLP)
        {
            fp->sv[fp->sc].n[0] = plane_n[fp->sc][0];
            fp->sv[fp->sc].n[1] = plane_n[fp->sc][1];
            fp->sv[fp->sc].n[2] = plane_n[fp->sc][2];
            fp->sv[fp->sc].d    = plane_d[fp->sc];

            plane_m[fp->sc] = read_mtrl(fp, k);

            fp->iv[fp->ic] = fp->sc;
            inci(fp);
            incs(fp);
            lp->sc++;
        }
        if (t == T_END)
            break;
    }
}

/*---------------------------------------------------------------------------*/

static void make_path(struct s_file *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], short c)
{
    short i, pi = incp(fp);

    struct s_path *pp = fp->pv + pi;

    pp->p[0] = 0.f;
    pp->p[1] = 0.f;
    pp->p[2] = 0.f;
    pp->t    = 1.f;
    pp->pi   = pi;
    pp->f    = 1;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "targetname") == 0)
            make_sym(v[i], pi);

        if (strcmp(k[i], "target") == 0)
            make_ref(v[i], &pp->pi);

        if (strcmp(k[i], "state") == 0)
            pp->f = atoi(v[i]);

        if (strcmp(k[i], "speed") == 0)
            sscanf(v[i], "%f", &pp->t);

        if (strcmp(k[i], "origin") == 0)
        {
            short x = 0, y = 0, z = 0;

            sscanf(v[i], "%hd %hd %hd", &x, &y, &z);

            pp->p[0] = +(float) x / SCALE;
            pp->p[1] = +(float) z / SCALE;
            pp->p[2] = -(float) y / SCALE;
        }
    }
}

static void make_body(struct s_file *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], short c, short l0)
{
    short i, bi = incb(fp);

    short g0 = fp->gc;
    short v0 = fp->vc;

    float p[3];

    short x = 0;
    short y = 0;
    short z = 0;

    struct s_body *bp = fp->bv + bi;

    bp->t  = 0.f;
    bp->pi = -1;
    bp->ni = -1;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "targetname") == 0)
            make_sym(v[i], bi);

        if (strcmp(k[i], "target") == 0)
            make_ref(v[i], &bp->pi);

        if (strcmp(k[i], "model") == 0)
            read_obj(fp, v[i]);

        if (strcmp(k[i], "origin") == 0)
            sscanf(v[i], "%hd %hd %hd", &x, &y, &z);

        if (strcmp(k[i], "message") == 0)
        {
            strcpy(fp->av, v[i]);
            fp->ac = (short) (strlen(v[i]) + 1);
        }
    }

    bp->l0 = l0;
    bp->lc = fp->lc - l0;
    bp->g0 = fp->ic;
    bp->gc = fp->gc - g0;

    for (i = 0; i < bp->gc; i++)
        fp->iv[inci(fp)] = g0++;

    p[0] = +(float) x / SCALE;
    p[1] = +(float) z / SCALE;
    p[2] = -(float) y / SCALE;

    for (i = v0; i < fp->vc; i++)
        v_add(fp->vv[i].p, fp->vv[i].p, p);
}

static void make_coin(struct s_file *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], short c)
{
    short i, ci = incc(fp);

    struct s_coin *cp = fp->cv + ci;

    cp->p[0] = 0.f;
    cp->p[1] = 0.f;
    cp->p[2] = 0.f;
    cp->n    = 1;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "light") == 0)
            sscanf(v[i], "%hd", &cp->n);

        if (strcmp(k[i], "origin") == 0)
        {
            short x = 0, y = 0, z = 0;

            sscanf(v[i], "%hd %hd %hd", &x, &y, &z);

            cp->p[0] = +(float) x / SCALE;
            cp->p[1] = +(float) z / SCALE;
            cp->p[2] = -(float) y / SCALE;
        }
    }
}

static void make_bill(struct s_file *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], short c)
{
    short i, ri = incr(fp);

    struct s_bill *rp = fp->rv + ri;

    memset(rp, 0, sizeof (struct s_bill));
    rp->t = 1.0f;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "width") == 0)
            sscanf(v[i], "%f %f %f", rp->w, rp->w + 1, rp->w + 2);
        if (strcmp(k[i], "height") == 0)
            sscanf(v[i], "%f %f %f", rp->h, rp->h + 1, rp->h + 2);

        if (strcmp(k[i], "xrot") == 0)
            sscanf(v[i], "%f %f %f", rp->rx, rp->rx + 1, rp->rx + 2);
        if (strcmp(k[i], "yrot") == 0)
            sscanf(v[i], "%f %f %f", rp->ry, rp->ry + 1, rp->ry + 2);
        if (strcmp(k[i], "zrot") == 0)
            sscanf(v[i], "%f %f %f", rp->rz, rp->rz + 1, rp->rz + 2);

        if (strcmp(k[i], "time") == 0)
            sscanf(v[i], "%f", &rp->t);
        if (strcmp(k[i], "dist") == 0)
            sscanf(v[i], "%f", &rp->d);
        if (strcmp(k[i], "flag") == 0)
            sscanf(v[i], "%hd", &rp->fl);

        if (strcmp(k[i], "image") == 0)
        {
            rp->mi = read_mtrl(fp, v[i]);
            fp->mv[rp->mi].fl |= M_CLAMPED;
        }

        if (strcmp(k[i], "origin") == 0)
        {
            short x = 0, y = 0, z = 0;
            float p[3];

            sscanf(v[i], "%hd %hd %hd", &x, &y, &z);

            p[0] = +(float) x / SCALE;
            p[1] = +(float) z / SCALE;
            p[2] = -(float) y / SCALE;

            rp->d     = v_len(p);
            rp->rx[0] = V_DEG(fatan2f(+p[1], rp->d));
            rp->ry[0] = V_DEG(fatan2f(+p[0], -p[2]));
        }
    }

    if (rp->fl & B_ADDITIVE)
        fp->mv[rp->mi].fl |= M_ADDITIVE;
}

static void make_goal(struct s_file *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], short c)
{
    short i, zi = incz(fp);

    struct s_goal *zp = fp->zv + zi;

    zp->p[0] = 0.f;
    zp->p[1] = 0.f;
    zp->p[2] = 0.f;
    zp->r    = 0.75;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "radius") == 0)
            sscanf(v[i], "%f", &zp->r);

        if (strcmp(k[i], "origin") == 0)
        {
            short x = 0, y = 0, z = 0;

            sscanf(v[i], "%hd %hd %hd", &x, &y, &z);

            zp->p[0] = +(float) (x)      / SCALE;
            zp->p[1] = +(float) (z - 24) / SCALE;
            zp->p[2] = -(float) (y)      / SCALE;
        }
    }
}

static void make_view(struct s_file *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], short c)
{
    short i, wi = incw(fp);

    struct s_view *wp = fp->wv + wi;

    wp->p[0] = 0.f;
    wp->p[1] = 0.f;
    wp->p[2] = 0.f;
    wp->q[0] = 0.f;
    wp->q[1] = 0.f;
    wp->q[2] = 0.f;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "target") == 0)
            make_ref(v[i], targ_wi + wi);

        if (strcmp(k[i], "origin") == 0)
        {
            short x = 0, y = 0, z = 0;

            sscanf(v[i], "%hd %hd %hd", &x, &y, &z);

            wp->p[0] = +(float) x / SCALE;
            wp->p[1] = +(float) z / SCALE;
            wp->p[2] = -(float) y / SCALE;
        }
    }
}

static void make_jump(struct s_file *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], short c)
{
    short i, ji = incj(fp);

    struct s_jump *jp = fp->jv + ji;

    jp->p[0] = 0.f;
    jp->p[1] = 0.f;
    jp->p[2] = 0.f;
    jp->q[0] = 0.f;
    jp->q[1] = 0.f;
    jp->q[2] = 0.f;
    jp->r    = 0.5;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "radius") == 0)
            sscanf(v[i], "%f", &jp->r);

        if (strcmp(k[i], "target") == 0)
            make_ref(v[i], targ_ji + ji);

        if (strcmp(k[i], "origin") == 0)
        {
            short x = 0, y = 0, z = 0;

            sscanf(v[i], "%hd %hd %hd", &x, &y, &z);

            jp->p[0] = +(float) x / SCALE;
            jp->p[1] = +(float) z / SCALE;
            jp->p[2] = -(float) y / SCALE;
        }
    }
}

static void make_swch(struct s_file *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], short c)
{
    short i, xi = incx(fp);

    struct s_swch *xp = fp->xv + xi;

    xp->p[0] = 0.f;
    xp->p[1] = 0.f;
    xp->p[2] = 0.f;
    xp->r    = 0.5;
    xp->pi   = 0;
    xp->t0   = 0;
    xp->t    = 0;
    xp->f0   = 0;
    xp->f    = 0;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "radius") == 0)
            sscanf(v[i], "%f", &xp->r);

        if (strcmp(k[i], "target") == 0)
            make_ref(v[i], &xp->pi);

        if (strcmp(k[i], "timer") == 0)
            sscanf(v[i], "%f", &xp->t0);

        if (strcmp(k[i], "state") == 0)
            xp->f = atoi(v[i]);

        if (strcmp(k[i], "origin") == 0)
        {
            short x = 0, y = 0, z = 0;

            sscanf(v[i], "%hd %hd %hd", &x, &y, &z);

            xp->p[0] = +(float) x / SCALE;
            xp->p[1] = +(float) z / SCALE;
            xp->p[2] = -(float) y / SCALE;
        }
    }
}

static void make_targ(struct s_file *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], short c)
{
    short i;

    targ_p[targ_n][0] = 0.f;
    targ_p[targ_n][1] = 0.f;
    targ_p[targ_n][3] = 0.f;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "targetname") == 0)
            make_sym(v[i], targ_n);

        if (strcmp(k[i], "origin") == 0)
        {
            short x = 0, y = 0, z = 0;

            sscanf(v[i], "%hd %hd %hd", &x, &y, &z);

            targ_p[targ_n][0] = +(float) x / SCALE;
            targ_p[targ_n][1] = +(float) z / SCALE;
            targ_p[targ_n][2] = -(float) y / SCALE;
        }
    }

    targ_n++;
}

static void make_ball(struct s_file *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], short c)
{
    short i, ui = incu(fp);

    struct s_ball *up = fp->uv + ui;

    up->p[0] = 0.f;
    up->p[1] = 0.f;
    up->p[2] = 0.f;
    up->r    = 0.25;

    up->e[0][0] = 1.f;
    up->e[0][1] = 0.f;
    up->e[0][2] = 0.f;
    up->e[1][0] = 0.f;
    up->e[1][1] = 1.f;
    up->e[1][2] = 0.f;
    up->e[2][0] = 0.f;
    up->e[2][1] = 0.f;
    up->e[2][2] = 1.f;

    up->v[0] = 0.f;
    up->v[1] = 0.f;
    up->v[2] = 0.f;
    up->w[0] = 0.f;
    up->w[1] = 0.f;
    up->w[2] = 0.f;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "radius") == 0)
            sscanf(v[i], "%f", &up->r);

        if (strcmp(k[i], "origin") == 0)
        {
            short x = 0, y = 0, z = 0;

            sscanf(v[i], "%hd %hd %hd", &x, &y, &z);

            up->p[0] = +(float) (x)      / SCALE;
            up->p[1] = +(float) (z - 24) / SCALE;
            up->p[2] = -(float) (y)      / SCALE;
        }
    }

    up->p[1] += up->r + SMALL;
}

/*---------------------------------------------------------------------------*/

static void read_ent(struct s_file *fp, FILE *fin)
{
    char k[MAXKEY][MAXSTR];
    char v[MAXKEY][MAXSTR];
    short t, i = 0, c = 0;

    short l0 = fp->lc;

    while ((t = map_token(fin, -1, k[c], v[c])))
    {
        if (t == T_KEY)
        {
            if (strcmp(k[c], "classname") == 0)
                i = c;
            c++;
        }
        if (t == T_BEG) read_lump(fp, fin);
        if (t == T_END) break;
    }

    if (!strcmp(v[i], "light"))                    make_coin(fp, k, v, c);
    if (!strcmp(v[i], "info_camp"))                make_swch(fp, k, v, c);
    if (!strcmp(v[i], "info_null"))                make_bill(fp, k, v, c);
    if (!strcmp(v[i], "path_corner"))              make_path(fp, k, v, c);
    if (!strcmp(v[i], "info_player_start"))        make_ball(fp, k, v, c);
    if (!strcmp(v[i], "info_player_intermission")) make_view(fp, k, v, c);
    if (!strcmp(v[i], "info_player_deathmatch"))   make_goal(fp, k, v, c);
    if (!strcmp(v[i], "target_teleporter"))        make_jump(fp, k, v, c);
    if (!strcmp(v[i], "target_position"))          make_targ(fp, k, v, c);
    if (!strcmp(v[i], "worldspawn"))               make_body(fp, k, v, c, l0);
    if (!strcmp(v[i], "func_train"))               make_body(fp, k, v, c, l0);
    if (!strcmp(v[i], "misc_model"))               make_body(fp, k, v, c, l0);
}

static void read_map(struct s_file *fp, FILE *fin)
{
    char k[MAXSTR];
    char v[MAXSTR];
    int t;

    while ((t = map_token(fin, -1, k, v)))
        if (t == T_BEG)
            read_ent(fp, fin);
}

/*---------------------------------------------------------------------------*/

/* Test the location of a point with respect to a side plane. */

static int fore_side(const float p[3], const struct s_side *sp)
{
    return (v_dot(p, sp->n) - sp->d > +SMALL) ? 1 : 0;
}

static int on_side(const float p[3], const struct s_side *sp)
{
    float d = v_dot(p, sp->n) - sp->d;

    return (-SMALL < d && d < +SMALL) ? 1 : 0;
}

/*---------------------------------------------------------------------------*/
/*
 * Confirm  that  the addition  of  a vert  would  not  result in  degenerate
 * geometry.
 */

static int ok_vert(const struct s_file *fp,
                   const struct s_lump *lp, const float p[3])
{
    float r[3];
    short i;

    for (i = 0; i < lp->vc; i++)
    {
        float *q = fp->vv[fp->iv[lp->v0 + i]].p;

        v_sub(r, p, q);

        if (v_len(r) < SMALL)
            return 0;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

/*
 * The following functions take the  set of planes defining a lump and
 * find the verts, edges, and  geoms that describe its boundaries.  To
 * do this, they first find the verts, and then search these verts for
 * valid edges and  geoms.  It may be more  efficient to compute edges
 * and  geoms directly  by clipping  down infinite  line  segments and
 * planes,  but this  would be  more  complex and  prone to  numerical
 * error.
 */

/*
 * Given 3  side planes,  compute the point  of intersection,  if any.
 * Confirm that this point falls  within the current lump, and that it
 * is unique.  Add it as a vert of the solid.
 */
static void clip_vert(struct s_file *fp,
                      struct s_lump *lp, short si, short sj, short sk)
{
    float M[16], X[16], I[16];
    float d[3],  p[3];
    short i;

    d[0] = fp->sv[si].d;
    d[1] = fp->sv[sj].d;
    d[2] = fp->sv[sk].d;

    m_basis(M, fp->sv[si].n, fp->sv[sj].n, fp->sv[sk].n);
    m_xps(X, M);
	
    if (m_inv(I, X))
    {
        m_vxfm(p, I, d);

        for (i = 0; i < lp->sc; i++)
        {
            short si = fp->iv[lp->s0 + i];

            if (fore_side(p, fp->sv + si))
                return;
        }

        if (ok_vert(fp, lp, p))
        {
            v_cpy(fp->vv[fp->vc].p, p);

            fp->iv[fp->ic] = fp->vc;
            inci(fp);
            incv(fp);
            lp->vc++;
        }
    }
}

/*
 * Given two  side planes,  find an edge  along their  intersection by
 * finding a pair of vertices that fall on both planes.  Add it to the
 * solid.
 */
static void clip_edge(struct s_file *fp,
                      struct s_lump *lp, short si, short sj)
{
    short i, j;

    for (i = 1; i < lp->vc; i++)
        for (j = 0; j < i; j++)
        {
            short vi = fp->iv[lp->v0 + i];
            short vj = fp->iv[lp->v0 + j];

            if (on_side(fp->vv[vi].p, fp->sv + si) &&
                on_side(fp->vv[vj].p, fp->sv + si) &&
                on_side(fp->vv[vi].p, fp->sv + sj) &&
                on_side(fp->vv[vj].p, fp->sv + sj))
            {
                fp->ev[fp->ec].vi = vi;
                fp->ev[fp->ec].vj = vj;

                fp->iv[fp->ic] = fp->ec;

                inci(fp);
                ince(fp);
                lp->ec++;
            }
        }
}

/*
 * Find all verts that lie on  the given side of the lump.  Sort these
 * verts to  have a counter-clockwise winding about  the plane normal.
 * Create geoms to tessalate the resulting convex polygon.
 */
static void clip_geom(struct s_file *fp,
                      struct s_lump *lp, short si)
{
    short   m[256], t[256], d, i, j, n = 0;
    float u[3];
    float v[3];
    float w[3];

    struct s_side *sp = fp->sv + si;

    /* Find em. */

    for (i = 0; i < lp->vc; i++)
    {
        short vi = fp->iv[lp->v0 + i];

        if (on_side(fp->vv[vi].p, sp))
        {
            m[n] = vi;
            t[n] = inct(fp);

            v_add(v, fp->vv[vi].p, plane_p[si]);

            fp->tv[t[n]].u[0] = v_dot(v, plane_u[si]);
            fp->tv[t[n]].u[1] = v_dot(v, plane_v[si]);

            n++;
        }
    }

    /* Sort em. */

    for (i = 1; i < n; i++)
        for (j = i + 1; j < n; j++)
        {
            v_sub(u, fp->vv[m[i]].p, fp->vv[m[0]].p);
            v_sub(v, fp->vv[m[j]].p, fp->vv[m[0]].p);
            v_crs(w, u, v);

            if (v_dot(w, sp->n) < 0.f)
            {
                d     = m[i];
                m[i]  = m[j];
                m[j]  =    d;

                d     = t[i];
                t[i]  = t[j];
                t[j]  =    d;
            }
        }

    /* Index em. */

    for (i = 0; i < n - 2; i++)
    {
        fp->gv[fp->gc].mi = plane_m[si];

        fp->gv[fp->gc].ti = t[0];
        fp->gv[fp->gc].tj = t[i + 1];
        fp->gv[fp->gc].tk = t[i + 2];

        fp->gv[fp->gc].si = si;
        fp->gv[fp->gc].sj = si;
        fp->gv[fp->gc].sk = si;

        fp->gv[fp->gc].vi = m[0];
        fp->gv[fp->gc].vj = m[i + 1];
        fp->gv[fp->gc].vk = m[i + 2];

        fp->iv[fp->ic] = fp->gc;
        inci(fp);
        incg(fp);
        lp->gc++;
    }
}

/*
 * Iterate the sides of the lump, attemping to generate a new vert for
 * each trio of planes, a new edge  for each pair of planes, and a new
 * set of geom for each visible plane.
 */
static void clip_lump(struct s_file *fp, struct s_lump *lp)
{
    short i, j, k;

    lp->v0 = fp->ic;
    lp->vc = 0;

    for (i = 2; i < lp->sc; i++)
        for (j = 1; j < i; j++)
            for (k = 0; k < j; k++)
                clip_vert(fp, lp,
                          fp->iv[lp->s0 + i],
                          fp->iv[lp->s0 + j],
                          fp->iv[lp->s0 + k]);

    lp->e0 = fp->ic;
    lp->ec = 0;

    for (i = 1; i < lp->sc; i++)
        for (j = 0; j < i; j++)
            clip_edge(fp, lp,
                      fp->iv[lp->s0 + i],
                      fp->iv[lp->s0 + j]);

    lp->g0 = fp->ic;
    lp->gc = 0;

    for (i = 0; i < lp->sc; i++)
        if (fp->mv[plane_m[fp->iv[lp->s0 + i]]].d[3] > 0)
            clip_geom(fp, lp,
                      fp->iv[lp->s0 + i]);

    for (i = 0; i < lp->sc; i++)
        if (plane_f[fp->iv[lp->s0 + i]])
            lp->fl |= L_DETAIL;
}

static void clip_file(struct s_file *fp)
{
    short i;

    for (i = 0; i < fp->lc; i++)
        clip_lump(fp, fp->lv + i);
}

/*---------------------------------------------------------------------------*/

/*
 * For each body element type,  determine if element 'p' is equivalent
 * to element  'q'.  This  is more than  a simple memory  compare.  It
 * effectively  snaps mtrls and  verts togather,  and may  reverse the
 * winding of  an edge or a geom.   This is done in  order to maximize
 * the number of elements that can be eliminated.
 */

static int comp_mtrl(const struct s_mtrl *mp, const struct s_mtrl *mq)
{
    if (fabs(mp->d[0] - mq->d[0]) > SMALL) return 0;
    if (fabs(mp->d[1] - mq->d[1]) > SMALL) return 0;
    if (fabs(mp->d[2] - mq->d[2]) > SMALL) return 0;
    if (fabs(mp->d[3] - mq->d[3]) > SMALL) return 0;

    if (fabs(mp->a[0] - mq->a[0]) > SMALL) return 0;
    if (fabs(mp->a[1] - mq->a[1]) > SMALL) return 0;
    if (fabs(mp->a[2] - mq->a[2]) > SMALL) return 0;
    if (fabs(mp->a[3] - mq->a[3]) > SMALL) return 0;

    if (fabs(mp->s[0] - mq->s[0]) > SMALL) return 0;
    if (fabs(mp->s[1] - mq->s[1]) > SMALL) return 0;
    if (fabs(mp->s[2] - mq->s[2]) > SMALL) return 0;
    if (fabs(mp->s[3] - mq->s[3]) > SMALL) return 0;

    if (fabs(mp->e[0] - mq->e[0]) > SMALL) return 0;
    if (fabs(mp->e[1] - mq->e[1]) > SMALL) return 0;
    if (fabs(mp->e[2] - mq->e[2]) > SMALL) return 0;
    if (fabs(mp->e[3] - mq->e[3]) > SMALL) return 0;

    if (fabs(mp->h[0] - mq->h[0]) > SMALL) return 0;

    if (strncmp(mp->f, mq->f, PATHMAX)) return 0;

    return 1;
}

static int comp_vert(const struct s_vert *vp, const struct s_vert *vq)
{
    if (fabs(vp->p[0] - vq->p[0]) > SMALL) return 0;
    if (fabs(vp->p[1] - vq->p[1]) > SMALL) return 0;
    if (fabs(vp->p[2] - vq->p[2]) > SMALL) return 0;

    return 1;
}

static int comp_edge(const struct s_edge *ep, const struct s_edge *eq)
{
    if (ep->vi != eq->vi && ep->vi != eq->vj) return 0;
    if (ep->vj != eq->vi && ep->vj != eq->vj) return 0;

    return 1;
}

static int comp_side(const struct s_side *sp, const struct s_side *sq)
{
    if  (fabs(sp->d - sq->d) > SMALL)  return 0;
    if (v_dot(sp->n,  sq->n) < 0.9999) return 0;

    return 1;
}

static int comp_texc(const struct s_texc *tp, const struct s_texc *tq)
{
    if (fabs(tp->u[0] - tq->u[0]) > SMALL) return 0;
    if (fabs(tp->u[1] - tq->u[1]) > SMALL) return 0;

    return 1;
}

static int comp_geom(const struct s_geom *gp, const struct s_geom *gq)
{
    if (gp->mi != gq->mi) return 0;

    if (gp->ti != gq->ti) return 0;
    if (gp->si != gq->si) return 0;
    if (gp->vi != gq->vi) return 0;

    if (gp->tj != gq->tj) return 0;
    if (gp->sj != gq->sj) return 0;
    if (gp->vj != gq->vj) return 0;

    if (gp->tk != gq->tk) return 0;
    if (gp->sk != gq->sk) return 0;
    if (gp->vk != gq->vk) return 0;

    return 1;
}

/*---------------------------------------------------------------------------*/

/*
 * For each file  element type, replace all references  to element 'i'
 * with a  reference to element  'j'.  These are used  when optimizing
 * and sorting the file.
 */

static void swap_mtrl(struct s_file *fp, short mi, short mj)
{
    short i;

    for (i = 0; i < fp->gc; i++)
        if (fp->gv[i].mi == mi) fp->gv[i].mi = mj;
    for (i = 0; i < fp->rc; i++)
        if (fp->rv[i].mi == mi) fp->rv[i].mi = mj;
}

static void swap_vert(struct s_file *fp, short vi, short vj)
{
    short i, j;

    for (i = 0; i < fp->ec; i++)
    {
        if (fp->ev[i].vi == vi) fp->ev[i].vi = vj;
        if (fp->ev[i].vj == vi) fp->ev[i].vj = vj;
    }

    for (i = 0; i < fp->gc; i++)
    {
        if (fp->gv[i].vi == vi) fp->gv[i].vi = vj;
        if (fp->gv[i].vj == vi) fp->gv[i].vj = vj;
        if (fp->gv[i].vk == vi) fp->gv[i].vk = vj;
    }

    for (i = 0; i < fp->lc; i++)
        for (j = 0; j < fp->lv[i].vc; j++)
            if (fp->iv[fp->lv[i].v0 + j] == vi)
                fp->iv[fp->lv[i].v0 + j]  = vj;
}

static void swap_edge(struct s_file *fp, short ei, short ej)
{
    short i, j;

    for (i = 0; i < fp->lc; i++)
        for (j = 0; j < fp->lv[i].ec; j++)
            if (fp->iv[fp->lv[i].e0 + j] == ei)
                fp->iv[fp->lv[i].e0 + j]  = ej;
}

static void swap_side(struct s_file *fp, short si, short sj)
{
    short i, j;

    for (i = 0; i < fp->gc; i++)
    {
        if (fp->gv[i].si == si) fp->gv[i].si = sj;
        if (fp->gv[i].sj == si) fp->gv[i].sj = sj;
        if (fp->gv[i].sk == si) fp->gv[i].sk = sj;
    }
    for (i = 0; i < fp->nc; i++)
        if (fp->nv[i].si == si) fp->nv[i].si = sj;

    for (i = 0; i < fp->lc; i++)
        for (j = 0; j < fp->lv[i].sc; j++)
            if (fp->iv[fp->lv[i].s0 + j] == si)
                fp->iv[fp->lv[i].s0 + j]  = sj;
}

static void swap_texc(struct s_file *fp, short ti, short tj)
{
    short i;

    for (i = 0; i < fp->gc; i++)
    {
        if (fp->gv[i].ti == ti) fp->gv[i].ti = tj;
        if (fp->gv[i].tj == ti) fp->gv[i].tj = tj;
        if (fp->gv[i].tk == ti) fp->gv[i].tk = tj;
    }
}


static void swap_geom(struct s_file *fp, short gi, short gj)
{
    short i, j;

    for (i = 0; i < fp->lc; i++)
        for (j = 0; j < fp->lv[i].gc; j++)
            if (fp->iv[fp->lv[i].g0 + j] == gi)
                fp->iv[fp->lv[i].g0 + j]  = gj;

    for (i = 0; i < fp->bc; i++)
        for (j = 0; j < fp->bv[i].gc; j++)
            if (fp->iv[fp->bv[i].g0 + j] == gi)
                fp->iv[fp->bv[i].g0 + j]  = gj;
}

/*---------------------------------------------------------------------------*/

static void uniq_mtrl(struct s_file *fp)
{
    short i, j, k = 0;

    for (i = 0; i < fp->mc; i++)
    {
        for (j = 0; j < k; j++)
            if (comp_mtrl(fp->mv + i, fp->mv + j))
            {
                swap_mtrl(fp, i, j);
                break;
            }

        if (j == k)
        {
            if (i != k)
            {
                fp->mv[k] = fp->mv[i];
                swap_mtrl(fp, i, k);
            }
            k++;
        }
    }

    fp->mc = k;
}

static void uniq_vert(struct s_file *fp)
{
    short i, j, k = 0;

    for (i = 0; i < fp->vc; i++)
    {
        for (j = 0; j < k; j++)
            if (comp_vert(fp->vv + i, fp->vv + j))
            {
                swap_vert(fp, i, j);
                break;
            }

        if (j == k)
        {
            if (i != k)
            {
                fp->vv[k] = fp->vv[i];
                swap_vert(fp, i, k);
            }
            k++;
        }
    }

    fp->vc = k;
}

static void uniq_edge(struct s_file *fp)
{
    short i, j, k = 0;

    for (i = 0; i < fp->ec; i++)
    {
        for (j = 0; j < k; j++)
            if (comp_edge(fp->ev + i, fp->ev + j))
            {
                swap_edge(fp, i, j);
                break;
            }

        if (j == k)
        {
            if (i != k)
            {
                fp->ev[k] = fp->ev[i];
                swap_edge(fp, i, k);
            }
            k++;
        }
    }

    fp->ec = k;
}

static void uniq_geom(struct s_file *fp)
{
    short i, j, k = 0;

    for (i = 0; i < fp->gc; i++)
    {
        for (j = 0; j < k; j++)
            if (comp_geom(fp->gv + i, fp->gv + j))
            {
                swap_geom(fp, i, j);
                break;
            }

        if (j == k)
        {
            if (i != k)
            {
                fp->gv[k] = fp->gv[i];
                swap_geom(fp, i, k);
            }
            k++;
        }
    }

    fp->gc = k;
}

static void uniq_texc(struct s_file *fp)
{
    short i, j, k = 0;

    for (i = 0; i < fp->tc; i++)
    {
        for (j = 0; j < k; j++)
            if (comp_texc(fp->tv + i, fp->tv + j))
            {
                swap_texc(fp, i, j);
                break;
            }

        if (j == k)
        {
            if (i != k)
            {
                fp->tv[k] = fp->tv[i];
                swap_texc(fp, i, k);
            }
            k++;
        }
    }

    fp->tc = k;
}

static void uniq_side(struct s_file *fp)
{
    short i, j, k = 0;

    for (i = 0; i < fp->sc; i++)
    {
        for (j = 0; j < k; j++)
            if (comp_side(fp->sv + i, fp->sv + j))
            {
                swap_side(fp, i, j);
                break;
            }

        if (j == k)
        {
            if (i != k)
            {
                fp->sv[k] = fp->sv[i];
                swap_side(fp, i, k);
            }
            k++;
        }
    }

    fp->sc = k;
}

static void uniq_file(struct s_file *fp)
{
    uniq_mtrl(fp);
    uniq_vert(fp);
    uniq_edge(fp);
    uniq_side(fp);
    uniq_texc(fp);
    uniq_geom(fp);
}

/*---------------------------------------------------------------------------*/

static void sort_file(struct s_file *fp)
{
    int i, j;

    /* Sort billboards farthest to nearest. */

    for (i = 0; i < fp->rc; i++)
        for (j = i + 1; j < fp->rc; j++)
            if (fp->rv[j].d > fp->rv[i].d)
            {
                struct s_bill t;

                t         = fp->rv[i];
                fp->rv[i] = fp->rv[j];
                fp->rv[j] =         t;
            }

    /* Ensure the first vertex is the lowest. */

    for (i = 0; i < fp->vc; i++)
        if (fp->vv[0].p[1] > fp->vv[i].p[1])
        {
            struct s_vert t;

            t         = fp->vv[0];
            fp->vv[0] = fp->vv[i];
            fp->vv[i] =         t;

            swap_vert(fp,  0, -1);
            swap_vert(fp,  i,  0);
            swap_vert(fp, -1,  i);
        }
}

/*---------------------------------------------------------------------------*/

static int test_lump_side(const struct s_file *fp,
                          const struct s_lump *lp,
                          const struct s_side *sp)
{
    short si;
    short vi;

    short f = 0;
    short b = 0;

    /* If the given side is part of the given lump, then the lump is behind. */

    for (si = 0; si < lp->sc; si++)
        if (fp->sv + fp->iv[lp->s0 + si] == sp)
            return -1;

    /* Check if each lump vertex is in front of, behind, on the side. */

    for (vi = 0; vi < lp->vc; vi++)
    {
        float d = v_dot(fp->vv[fp->iv[lp->v0 + vi]].p, sp->n) - sp->d;

        if (d > 0) f++;
        if (d < 0) b++;
    }

    /* If no verts are behind, the lump is in front, and vice versa. */

    if (f > 0 && b == 0) return +1;
    if (b > 0 && f == 0) return -1;

    /* Else, the lump crosses the side. */

    return 0;
}

static int node_node(struct s_file *fp, short l0, short lc)
{
    if (lc < 8)
    {
        /* Base case.  Dump all given lumps into a leaf node. */

        fp->nv[fp->nc].si = -1;
        fp->nv[fp->nc].ni = -1;
        fp->nv[fp->nc].nj = -1;
        fp->nv[fp->nc].l0 = l0;
        fp->nv[fp->nc].lc = lc;

        return incn(fp);
    }
    else
    {
        short sj  = 0;
        short sjd = lc;
        short sjo = lc;
        short si;
        short li = 0, lic = 0;
        short lj = 0, ljc = 0;
        short lk = 0, lkc = 0;
        short i;

        /* Find the side that most evenly splits the given lumps. */

        for (si = 0; si < fp->sc; si++)
        {
            short o = 0;
            short d = 0;
            short k = 0;

            for (li = 0; li < lc; li++)
                if ((k = test_lump_side(fp, fp->lv + l0 + li, fp->sv + si)))
                    d += k;
                else
                    o++;

            d = abs(d);

            if ((d < sjd) || (d == sjd && o < sjo))
            {
                sj  = si;
                sjd = d;
                sjo = o;
            }
        }

        /* Flag each lump with its position WRT the side. */

        for (li = 0; li < lc; li++)
            switch (test_lump_side(fp, fp->lv + l0 + li, fp->sv + sj))
            {
            case +1: fp->lv[l0+li].fl = (fp->lv[l0+li].fl & 1) | 0x10; break;
            case  0: fp->lv[l0+li].fl = (fp->lv[l0+li].fl & 1) | 0x20; break;
            case -1: fp->lv[l0+li].fl = (fp->lv[l0+li].fl & 1) | 0x40; break;
            }

        /* Sort all lumps in the range by their flag values. */
        
        for (li = 1; li < lc; li++)
            for (lj = 0; lj < li; lj++)
                if (fp->lv[l0 + li].fl < fp->lv[l0 + lj].fl)
                {
                    struct s_lump l;

                    l               = fp->lv[l0 + li];
                    fp->lv[l0 + li] = fp->lv[l0 + lj];
                    fp->lv[l0 + lj] =               l;
                }

        /* Establish the in-front, on, and behind lump ranges. */

        li = lic = 0;
        lj = ljc = 0;
        lk = lkc = 0;

        for (i = lc - 1; i >= 0; i--)
            switch (fp->lv[l0 + i].fl & 0xf0)
            {
            case 0x10: li = l0 + i; lic++; break;
            case 0x20: lj = l0 + i; ljc++; break;
            case 0x40: lk = l0 + i; lkc++; break;
            }

        /* Add the lumps on the side to the node. */

        i = incn(fp);

        fp->nv[i].si = sj;
        fp->nv[i].ni = node_node(fp, li, lic);

        fp->nv[i].nj = node_node(fp, lk, lkc);
        fp->nv[i].l0 = lj;
        fp->nv[i].lc = ljc;

        return i;
    }
}

static void node_file(struct s_file *fp)
{
    short bi;

    /* Sort the lumps of each body into BSP nodes. */

    for (bi = 0; bi < fp->bc; bi++)
        fp->bv[bi].ni = node_node(fp, fp->bv[bi].l0, fp->bv[bi].lc);
}

/*---------------------------------------------------------------------------*/

static void dump_file(struct s_file *p, const char *name)
{
    short i, j;
    int c = 0;
    int n = 0;
    int m = p->rc + p->cc * 128 + (p->zc * p->jc + p->xc) * 32;

    /* Count the number of solid lumps. */

    for (i = 0; i < p->lc; i++)
        if ((p->lv[i].fl & 1) == 0)
            n++;

    /* Count the number of visible geoms. */

    for (i = 0; i < p->bc; i++)
    {
        for (j = 0; j < p->bv[i].lc; j++)
            m += p->lv[p->bv[i].l0 + j].gc;
        m += p->bv[i].gc;
    }

    /* Count the total value of all coins. */

    for (i = 0; i < p->cc; i++)
        c += p->cv[i].n;

    printf("%s (%d/%d/$%d)\n"
           "  mtrl  vert  edge  side  texc"
           "  geom  lump  path  node  body\n"
           "%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d\n"
           "  coin  goal  view  jump  swch"
           "  bill  ball  char  indx\n"
           "%6d%6d%6d%6d%6d%6d%6d%6d%6d\n",
           name, n, m, c,
           p->mc, p->vc, p->ec, p->sc, p->tc,
           p->gc, p->lc, p->pc, p->nc, p->bc,
           p->cc, p->zc, p->wc, p->jc, p->xc,
           p->rc, p->uc, p->ac, p->ic);
}

int main(int argc, char *argv[])
{
    char src[MAXSTR];
    char dst[MAXSTR];
    struct s_file f;
    FILE *fin;

    if (argc > 2)
    {
        if (config_data_path(argv[2], NULL))
        {
            strncpy(src,  argv[1], MAXSTR);
            strncpy(dst,  argv[1], MAXSTR);

            if (strcmp(dst + strlen(dst) - 4, ".map") == 0)
                strcpy(dst + strlen(dst) - 4, ".sol");
            else
                strcat(dst, ".sol");

            if ((fin = fopen(src, "r")))
            {
                init_file(&f);
                read_map(&f, fin);

                resolve();
                targets(&f);

                clip_file(&f);
                move_file(&f);
                uniq_file(&f);
                sort_file(&f);
                node_file(&f);
                dump_file(&f, dst);

                sol_stor(&f, dst);

                fclose(fin);
            }
        }
        else fprintf(stderr, "Failure to establish data directory\n");
    }
    else fprintf(stderr, "Usage: %s <map> [data]\n", argv[0]);

    return 0;
}

