//
// Copyright (c) 2014 Sergio Moura sergio@moura.us
// Copyright (c) 2011 Andreas Krinke andreas.krinke@gmx.de
// Copyright (c) 2009 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef FONTSTASH_H
#define FONTSTASH_H

#define STH_OPENGL3

#ifdef __APPLE__
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

#define STH_ESUCCESS 0
// error opening file
#define STH_EFILEIO -1
// error initializing truetype font
#define STH_ETTFINIT -2
// invalid argument
#define STH_EINVAL -3
// not enough memory
#define STH_ENOMEM -4

#ifdef __cplusplus
extern "C" {
#endif
    
/* @rlyeh: removed STB_TRUETYPE_IMPLENTATION. We link it externally */
#include "stb_truetype.h"

#define HASH_LUT_SIZE 256
#define MAX_ROWS 128
#define VERT_COUNT (6*128)
#define VERT_STRIDE (sizeof(float)*4)

#define TTFONT_FILE 1
#define TTFONT_MEM  2
#define BMFONT      3

#define STH_GL_TEXTYPE   GL_RED

typedef struct sth_quad {
	float x0,y0,s0,t0;
	float x1,y1,s1,t1;
} sth_quad;

typedef struct sth_row {
	short x,y,h;
} sth_row;

typedef struct sth_glyph {
	unsigned int codepoint;
	short size;
	struct sth_texture* texture;
	int x0,y0,x1,y1;
	float xadv,xoff,yoff;
	int next;
} sth_glyph;

typedef struct sth_font {
	int idx;
	int type;
	stbtt_fontinfo font;
	unsigned char* data;
    sth_glyph* glyphs;
	int lut[HASH_LUT_SIZE];
	int nglyphs;
	float ascender;
	float descender;
	float lineh;
	struct sth_font* next;
} sth_font;

struct sth_texture {
	GLuint id;
	// TODO: replace rows with pointer
    sth_row rows[MAX_ROWS];
	int nrows;
	float verts[4 * VERT_COUNT];
	int nverts;
	struct sth_texture* next;
};

typedef struct sth_stash {
	int tw;
    int th;
	float itw;
    float ith;
    float screen2gl_x;
    float screen2gl_y;
	struct sth_texture* tt_textures;
	struct sth_texture* bm_textures;
    sth_font* fonts;
	int drawing;
    GLuint programID;
    GLuint textureID;
    GLuint vao;
    GLuint vbo, ebo;
    GLfloat color[4];
    GLuint colorID;
    GLushort *elementIndices;
    unsigned indiceCount;
} sth_stash;

sth_stash* sth_create(int cachew, int cacheh);
void sth_set_screen_size(sth_stash* stash, float width, float height);
int sth_add_font(sth_stash* stash, const char* path);
int sth_add_font_from_memory(sth_stash* stash, unsigned char* buffer);

int sth_add_bitmap_font(sth_stash* stash, int ascent, int descent, int line_gap);
int sth_add_glyph_for_codepoint(sth_stash* stash, int idx, GLuint id, unsigned int codepoint, short int size, short int base, int x, int y, int w, int h, float xoffset, float yoffset, float xadvance);
int sth_add_glyph_for_char(sth_stash* stash, int idx, GLuint id, const char* s, short int size, short int base, int x, int y, int w, int h, float xoffset, float yoffset, float xadvance);

void sth_begin_draw(sth_stash* stash);
void sth_end_draw(sth_stash* stash);

void sth_draw_text(sth_stash* stash, int idx, float size, float x, float y, const char* s, float* dx);
void sth_dim_text(sth_stash* stash, int idx, float size, const char* s, float* minx, float* miny, float* maxx, float* maxy);
void sth_vmetrics(sth_stash* stash, int idx, float size, float* ascender, float* descender, float* lineh);
void sth_delete(sth_stash* stash);
void sth_color(sth_stash* stash, GLfloat r, GLfloat g, GLfloat b, GLfloat a);

#ifdef __cplusplus
}
#endif

#endif // FONTSTASH_H
