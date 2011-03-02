#include<stdio.h>
#include<stdlib.h>
#include <GL/gl.h>

#ifdef USE_DRAWELEMENTS
#ifndef NO_LIGHT
#error NOT SUPPORTED LIGHTING WITH USE_DRAWELEMENTS
#endif
#define MAX_ELEMENTS (3*2*512)
GLboolean elementsIsList( GLuint list );
void elementsDeleteLists( GLuint list, GLsizei range );
GLuint elementsGenLists( GLsizei range );
void elementsNewList( GLuint list, GLenum mode );
void elementsEndList( void );
void elementsCallList( GLuint list );
void elementsBindTexture( GLenum target, GLuint texture );
void elementsBegin( GLenum mode );
void elementsEnd( void );
void elementsBlendFunc( GLenum sfactor, GLenum dfactor );
void elementsTexCoord2fv( const GLfloat *v );
void elementsTexCoord2f( GLfloat s, GLfloat t );
void elementsVertex3fv( const GLfloat *v );
void elementsVertex3f( GLfloat x, GLfloat y, GLfloat z );
void elementsVertex2f( GLfloat x, GLfloat y );
void elementsColor4fv( const GLfloat *v );
void elementsColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );
#else
#define elementsIsList(L) glIsList(L)
#define elementsDeleteLists(L,R) glDeleteLists(L,R)
#define elementsGenLists(R) glGenLists(R)
#define elementsNewList(L,M) glNewList(L,M)
#define elementsEndList() glEndList()
#define elementsCallList(L) glCallList(L)
#define elementsBindTexture(TG,TX) glBindTexture(TG,TX)
#define elementsBegin(M) glBegin(M)
#define elementsEnd() glEnd()
#define elementsBlendFunc(SF,DF) glBlendFunc(SF,DF)
#define elementsTexCoord2fv(V) glTexCoord2fv(V)
#define elementsVertex3fv(V) glVertex3fv(V)
#define elementsColor4fv(V) glColor4fv(V)
#define elementsTexCoord2f(S,T) glTexCoord2f(S,T)
#define elementsVertex3f(X,Y,Z) glVertex3f(X,Y,Z)
#define elementsColor4f(R,G,B,A) glColor4f(R,G,B,A)
#define elementsVertex2f(X,Y) glVertex2f(X,Y)
#endif
