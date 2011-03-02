#ifdef USE_DRAWELEMENTS

#include "elements.h"

typedef struct internal_elements {
	char iselement;
	float vertex[(MAX_ELEMENTS+3+2)*3];
	float uv[(MAX_ELEMENTS+3+2)*2];
#if 0
	float color[MAX_ELEMENTS*4];
#endif
	int nvertices;
	int nuvs;
#if 0
	int ncolors;
#endif
	GLuint texture;
	GLenum mode;
	GLenum sfactor;
       	GLenum dfactor;
	struct internal_elements *next;
	
} internal_elements;

static internal_elements *compiling=NULL;
static int initted=0;
static unsigned ele[MAX_ELEMENTS];

GLboolean elementsIsList( GLuint list )
{
	internal_elements *e=(internal_elements *)list;
	if (e && e->iselement==123)
			return GL_TRUE;
	return GL_FALSE;
}

void elementsDeleteLists( GLuint list, GLsizei range )
{
	internal_elements *e=(internal_elements *)list;
	while(e && e->iselement==123)
	{
		void *tofree=e;
		if (compiling==e)
			compiling=NULL;
		e->iselement=0;

		e=e->next;
		free(tofree);
	}
}

GLuint elementsGenLists( GLsizei range )
{
	internal_elements *e=(internal_elements *)calloc(1,sizeof(internal_elements));
	if (e)
		e->iselement=123;
	if (!initted)
	{
		unsigned i;
		for(i=0;i<MAX_ELEMENTS;i++)
			ele[i]=i;
		initted=1;
	}
	return (GLuint)e;
}

void elementsNewList( GLuint list, GLenum mode )
{
	internal_elements *e=(internal_elements *)list;
	if (e && e->iselement==123)
	{
#if 0
		e->nvertices=e->nuvs=0=e->ncolors=0;
#else
		e->nvertices=e->nuvs=0;
#endif
		e->next=NULL;
		compiling=e;
	}
}

void elementsEndList( void )
{
	compiling=NULL;
}

void elementsCallList( GLuint list )
{
	internal_elements *e=(internal_elements *)list;
	GLboolean tx2d=glIsEnabled(GL_TEXTURE_2D);
	while(e && e->iselement==123 && e->nvertices)
	{
		if (e->texture)
		{
			glBindTexture(GL_TEXTURE_2D,e->texture);
			glEnable(GL_TEXTURE_2D);
		}
		else
			glDisable(GL_TEXTURE_2D);
		if (e->sfactor || e->dfactor)
		{
			glPushAttrib(GL_COLOR_BUFFER_BIT);
			glBlendFunc(e->sfactor,e->dfactor);
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3,GL_FLOAT,sizeof(float)*3,&e->vertex);
#if 0
		if (e->ncolors==e->nvertices)
		{
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(4,GL_FLOAT,sizeof(float)*4,&e->color);
		}
		else
#endif
			glDisableClientState(GL_COLOR_ARRAY);
		if (e->nuvs==e->nvertices)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2,GL_FLOAT,sizeof(float)*2,&e->uv);
		}
		else
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDrawElements(e->mode,e->nvertices,GL_UNSIGNED_INT,&ele[0]);

		if (e->sfactor || e->dfactor)
			glPopAttrib();
		e=e->next;
	}
	if (tx2d)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);
}

static internal_elements *addNext(internal_elements *e)
{
	internal_elements *n=(internal_elements *)calloc(1,sizeof(internal_elements));
	compiling=e->next=n;
	n->iselement=123;
	n->texture=e->texture;
	n->mode=e->mode;
	n->sfactor=e->sfactor;
	n->dfactor=e->dfactor;
	return n;
}

void elementsBindTexture( GLenum target, GLuint texture )
{
	internal_elements *e=compiling;
	if (e)
	{
		if (e->nvertices && e->texture!=texture)
			e=addNext(e);
		e->texture=texture;
	}
	else
		glBindTexture(target,texture);
}

void elementsBegin( GLenum mode )
{
	internal_elements *e=compiling;
	if (e)
	{
		if (e->mode && e->mode!=mode)
			e=addNext(e);
		e->mode=mode;
	}
	else
		glBegin(mode);
}

void elementsEnd( void )
{
	internal_elements *e=compiling;
	if (!e)
		glEnd();
}

void elementsBlendFunc( GLenum sfactor, GLenum dfactor )
{
	internal_elements *e=compiling;
	if (e)
	{
		if (e->nvertices && (e->sfactor!=sfactor || e->dfactor!=dfactor))
			e=addNext(e);
		e->sfactor=sfactor;
		e->dfactor=dfactor;
	}
	else
		glBlendFunc(sfactor,dfactor);
}

void elementsTexCoord2fv( const GLfloat *v )
{
	internal_elements *e=compiling;
	if (e)
	{
		if (v)
		{
			e->uv[(e->nuvs*2)+0]=v[0];
			e->uv[(e->nuvs*2)+1]=v[1];
			if (e->nuvs < MAX_ELEMENTS)
				e->nuvs++;
		}
	}
	else
		glTexCoord2fv(v);
}

void elementsTexCoord2f( GLfloat s, GLfloat t )
{
	internal_elements *e=compiling;
	if (e)
	{
		e->uv[(e->nuvs*2)+0]=s;
		e->uv[(e->nuvs*2)+1]=t;
		if (e->nuvs < MAX_ELEMENTS)
			e->nuvs++;
	}
	else
		glTexCoord2f(s,t);
	
}

void elementsVertex3fv( const GLfloat *v )
{
	internal_elements *e=compiling;
	if (e )
	{
		if (v)
		{
			e->vertex[(e->nvertices*3)+0]=v[0];
			e->vertex[(e->nvertices*3)+1]=v[1];
			e->vertex[(e->nvertices*3)+2]=v[2];
			e->nvertices++;
			if (e->nvertices >= MAX_ELEMENTS)
				e=addNext(e);
		}
	}
	else
		glVertex3fv(v);
}

void elementsVertex3f( GLfloat x, GLfloat y, GLfloat z )
{
	internal_elements *e=compiling;
	if (e )
	{
		e->vertex[(e->nvertices*3)+0]=x;
		e->vertex[(e->nvertices*3)+1]=y;
		e->vertex[(e->nvertices*3)+2]=z;
		e->nvertices++;
		if (e->nvertices >= MAX_ELEMENTS)
			e=addNext(e);
	}
	else
		glVertex3f(x,y,z);
}

void elementsVertex2f( GLfloat x, GLfloat y )
{
	internal_elements *e=compiling;
	if (e )
	{
		e->vertex[(e->nvertices*3)+0]=x;
		e->vertex[(e->nvertices*3)+1]=y;
		e->vertex[(e->nvertices*3)+2]=0.0f;
		e->nvertices++;
		if (e->nvertices >= MAX_ELEMENTS)
			e=addNext(e);
	}
	else
		glVertex2f(x,y);
}

#if 0
void elementsColor4fv( const GLfloat *v )
{
	internal_elements *e=compiling;
	if (e)
	{
		if (v)
		{
			e->color[(e->ncolors*4)+0]=v[0];
			e->color[(e->ncolors*4)+1]=v[1];
			e->color[(e->ncolors*4)+2]=v[2];
			e->color[(e->ncolors*4)+3]=v[3];
			if (e->ncolors < MAX_ELEMENTS)
				e->ncolors++;
		}
	}
	else
		glColor4fv(v);
}

void elementsColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
	internal_elements *e=compiling;
	if (e)
	{
		e->color[(e->ncolors*4)+0]=red;
		e->color[(e->ncolors*4)+1]=green;
		e->color[(e->ncolors*4)+2]=blue;
		e->color[(e->ncolors*4)+3]=alpha;
		if (e->ncolors < MAX_ELEMENTS)
			e->ncolors++;
	}
	else
		glColor4f(red,green,blue,alpha);
}
#endif

#endif
