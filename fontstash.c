//
// Copyright (c) 2014 Sergio Moura sergio@moura.us
// Copyright (c) 2011-2013 Andreas Krinke andreas.krinke@gmx.de
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> /* @rlyeh: floorf() */

#include "fontstash.h"

#ifdef __APPLE__
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

// Shaders
    #define GLSL(version, shader)  "#version " #version "\n" #shader

    const char vertexShader[] = GLSL(330 core,
        // Input vertex data, different for all executions of this shader.
        layout(location = 0) in vec3 vertexPosition_modelspace;
        layout(location = 1) in vec2 vertexUV;

        // Output data ; will be interpolated for each fragment.
        out vec2 UV;
        out vec4 vColor;

        // Values that stay constant for the whole mesh.
        uniform mat4 MVP;
        uniform vec4 color = vec4(1);

        void main() {
            gl_Position = vec4(vertexPosition_modelspace, 1);
            UV = vertexUV;
            vColor = color;
        }
    );

    const char fragmentShader[] = GLSL(330 core,
        // Interpolated values from the vertex shaders
        in vec2 UV;
        in vec4 vColor;

        // Ouput data
        out vec4 color;

        // Values that stay constant for the whole mesh.
        uniform sampler2D myTextureSampler;

        void main() {
            color = vColor * texture(myTextureSampler, UV).rrrr;
        }
    );



static int idx = 1;

static unsigned int hashint(unsigned int a) {
	a += ~(a<<15);
	a ^=  (a>>10);
	a +=  (a<<3);
	a ^=  (a>>6);
	a += ~(a<<11);
	a ^=  (a>>16);
	return a;
}



// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

static const unsigned char utf8d[] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
	8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
	0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
	0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
	0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
	1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
	1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
	1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
};

static unsigned int decutf8(unsigned int* state, unsigned int* codep, unsigned int byte) {
	unsigned int type = utf8d[byte];
	*codep = (*state != UTF8_ACCEPT) ?
		(byte & 0x3fu) | (*codep << 6) :
		(0xff >> type) & (byte);
	*state = utf8d[256 + *state*16 + type];
	return *state;
}

sth_stash* sth_create(int cachew, int cacheh) {
	// Allocate memory for the font stash.
    sth_stash* stash = (sth_stash*) malloc(sizeof(sth_stash));
    if(!stash) goto error;
	memset(stash, 0, sizeof(sth_stash));

	// Allocate memory for the first texture
	struct sth_texture* texture = (struct sth_texture*) malloc(sizeof(struct sth_texture));
	if(!texture) goto error;
	memset(texture, 0, sizeof(struct sth_texture));

    // Setup the initial color
    stash->color[0] = 1.0f;
    stash->color[1] = 1.0f;
    stash->color[2] = 1.0f;
    stash->color[3] = 1.0f;

    // Create the Shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    GLint Result = GL_FALSE;
    int infoLogLen;

    const char *vPtr = vertexShader;
    const char *fPtr = fragmentShader;

    glShaderSource(VertexShaderID, 1, &vPtr, NULL);
    glCompileShader(VertexShaderID);
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLen);
    if (infoLogLen > 0) {
        char buf[1024];
        glGetShaderInfoLog(VertexShaderID, infoLogLen, NULL, buf);
        printf("Vertex shader compilation result (%u)\n%s\n", Result, buf);
        printf("%s\n", vPtr);
    }

    glShaderSource(FragmentShaderID, 1, &fPtr, NULL);
    glCompileShader(FragmentShaderID);
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLen);
    if (infoLogLen > 0) {
        char buf[1024];
        glGetShaderInfoLog(FragmentShaderID, infoLogLen, NULL, buf);
        printf("Fragment shader compilation result (%u)\n%s\n", Result, buf);
        printf("%s\n", fPtr);
    }

    stash->programID = glCreateProgram();
    glAttachShader(stash->programID, VertexShaderID);
    glAttachShader(stash->programID, FragmentShaderID);
    glLinkProgram(stash->programID);
    glGetShaderiv(stash->programID, GL_LINK_STATUS, &Result);
    glGetShaderiv(stash->programID, GL_INFO_LOG_LENGTH, &infoLogLen);
    if (infoLogLen > 0) {
        char buf[1024];
        glGetShaderInfoLog(stash->programID, infoLogLen, NULL, buf);
        printf("Program link compilation result (%u)\n%s\n", Result, buf);
    }

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    // Setup a few values
    stash->textureID = glGetUniformLocation(stash->programID, "myTextureSampler");
    stash->colorID = glGetUniformLocation(stash->programID, "color");

    // OpenGL buffers
    glGenVertexArrays(1, &stash->vao);
    glGenBuffers(1, &stash->vbo);
    glGenBuffers(1, &stash->ebo);

	// Create first texture for the cache.
	stash->tw = cachew;
	stash->th = cacheh;
	stash->itw = 1.0f / cachew;
	stash->ith = 1.0f / cacheh;
	//stash->empty_data = empty_data;
	stash->tt_textures = texture;
	glGenTextures(1, &texture->id);
	if(!texture->id) goto error;
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glTexImage2D(GL_TEXTURE_2D, 0, STH_GL_TEXTYPE, cachew, cacheh, 0, STH_GL_TEXTYPE, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return stash;

error:
	if(stash) free(stash);
	if(texture) free(texture);
	return NULL;
}

void sth_set_screen_size(sth_stash* stash, float width, float height) {
    stash->screen2gl_x = 2.0f / width;
    stash->screen2gl_y = 2.0f / height;
}

void sth_color(sth_stash* stash, GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    stash->color[0] = r;
    stash->color[1] = g;
    stash->color[2] = b;
    stash->color[3] = a;
}

int sth_add_font_from_memory(sth_stash* stash, unsigned char* buffer) {
	int ret, i, ascent, descent, fh, lineGap;

    sth_font* fnt = (sth_font*) malloc(sizeof(sth_font));
	if(!fnt) {
        ret = STH_ENOMEM;
        goto error;
    }
	memset(fnt, 0, sizeof(sth_font));

	// Init hash lookup.
	for(i = 0; i < HASH_LUT_SIZE; ++i) fnt->lut[i] = -1;

	fnt->data = buffer;

	// Init stb_truetype
	if(!stbtt_InitFont(&fnt->font, fnt->data, 0)) {
        ret = STH_ETTFINIT;
        goto error;
    }

	// Store normalized line height. The real line height is got
	// by multiplying the lineh by font size.
	stbtt_GetFontVMetrics(&fnt->font, &ascent, &descent, &lineGap);
	fh = ascent - descent;
	fnt->ascender = (float) ascent / (float) fh;
	fnt->descender = (float) descent / (float) fh;
	fnt->lineh = (float) (fh + lineGap) / (float) fh;

	fnt->idx = idx;
	fnt->type = TTFONT_MEM;
	fnt->next = stash->fonts;
	stash->fonts = fnt;

	return idx++;

error:
	if(fnt) {
		if (fnt->glyphs) free(fnt->glyphs);
		free(fnt);
	}
	return ret;
}

int sth_add_font(sth_stash* stash, const char* path) {
	FILE* fp = 0;
	int ret, datasize;
	unsigned char* data = NULL;
	int idx;

	// Read in the font data.
	fp = fopen(path, "rb");
	if(!fp) {
        ret = STH_EFILEIO;
        goto error;
    }
	fseek(fp,0,SEEK_END);
	datasize = (int)ftell(fp);
	fseek(fp,0,SEEK_SET);
	data = (unsigned char*)malloc(datasize);
	if(!data) {
        ret = STH_ENOMEM;
        goto error;
    }
	fread(data, 1, datasize, fp);
	fclose(fp);
	fp = 0;

	idx = sth_add_font_from_memory(stash, data);
	// Modify type of the loaded font.
	if(idx) stash->fonts->type = TTFONT_FILE;
	else free(data);

	return idx;

error:
	if(data) free(data);
	if(fp) fclose(fp);
	return ret;
}

int sth_add_bitmap_font(sth_stash* stash, int ascent, int descent, int line_gap) {
	int ret, i, fh;
    sth_font* fnt = NULL;

	fnt = (sth_font*) malloc(sizeof(sth_font));
	if (!fnt) {
        ret = STH_ENOMEM;
        goto error;
    }
	memset(fnt, 0, sizeof(sth_font));

	// Init hash lookup.
	for (i = 0; i < HASH_LUT_SIZE; ++i) fnt->lut[i] = -1;

	// Store normalized line height. The real line height is got
	// by multiplying the lineh by font size.
	fh = ascent - descent;
	fnt->ascender = (float) ascent / (float) fh;
	fnt->descender = (float) descent / (float) fh;
	fnt->lineh = (float) (fh + line_gap) / (float) fh;

	fnt->idx = idx;
	fnt->type = BMFONT;
	fnt->next = stash->fonts;
	stash->fonts = fnt;

	return idx++;

error:
	if(fnt) free(fnt);
	return ret;
}

int sth_add_glyph_for_codepoint(sth_stash* stash, int idx, GLuint id, unsigned codepoint, short size, short base,
                                int x, int y, int w, int h, float xoffset, float yoffset, float xadvance) {
    struct sth_texture* texture = NULL;
	struct sth_font* fnt = NULL;
    sth_glyph* glyph = NULL;

	if(!stash) return STH_EINVAL;
	texture = stash->bm_textures;
	while (texture != NULL && texture->id != id) texture = texture->next;
	if(!texture) {
		// Create new texture
		texture = (struct sth_texture*) malloc(sizeof(struct sth_texture));
		if (!texture) return STH_ENOMEM;
		memset(texture, 0, sizeof(struct sth_texture));
		texture->id = id;
		texture->next = stash->bm_textures;
		stash->bm_textures = texture;
	}

	fnt = stash->fonts;
	while(fnt && fnt->idx != idx) fnt = fnt->next;
	if(!fnt) return STH_EINVAL;
	if(fnt->type != BMFONT) return STH_EINVAL;
	// Alloc space for new glyph.
	fnt->nglyphs++;
	fnt->glyphs = (sth_glyph *) realloc(fnt->glyphs, fnt->nglyphs * sizeof(sth_glyph)); /* @rlyeh: explicit cast needed in C++ */
	if(!fnt->glyphs) return STH_ENOMEM;

	// Init glyph.
	glyph = &fnt->glyphs[fnt->nglyphs-1];
	memset(glyph, 0, sizeof(sth_glyph));
	glyph->codepoint = codepoint;
	glyph->size = size;
	glyph->texture = texture;
	glyph->x0 = x;
	glyph->y0 = y;
	glyph->x1 = glyph->x0+w;
	glyph->y1 = glyph->y0+h;
	glyph->xoff = xoffset;
	glyph->yoff = yoffset - base;
	glyph->xadv = xadvance;

	// Find code point and size.
	h = hashint(codepoint) & (HASH_LUT_SIZE-1);
	// Insert char to hash lookup.
	glyph->next = fnt->lut[h];
	fnt->lut[h] = fnt->nglyphs-1;

    return STH_ESUCCESS;
}

int sth_add_glyph_for_char(sth_stash* stash, int idx, GLuint id, const char* s, short size, short base,
                           int x, int y, int w, int h, float xoffset, float yoffset, float xadvance) {
    unsigned codepoint, state = 0;

    for(; *s; ++s)
		if(!decutf8(&state, &codepoint, *(unsigned char*)s))
            break;
	if (state != UTF8_ACCEPT) return STH_EINVAL;

    return sth_add_glyph_for_codepoint(stash, idx, id, codepoint, size, base, x, y, w, h, xoffset, yoffset, xadvance);
}

static sth_glyph* get_glyph(sth_stash* stash, sth_font* fnt, unsigned int codepoint, short isize) {
	// Find code point and size.
	unsigned h = hashint(codepoint) & (HASH_LUT_SIZE - 1);
	int i = fnt->lut[h];
	while(i != -1) {
		if(fnt->glyphs[i].codepoint == codepoint && (fnt->type == BMFONT || fnt->glyphs[i].size == isize))
			return &fnt->glyphs[i];
		i = fnt->glyphs[i].next;
	}
	// Could not find glyph.

	// For bitmap fonts: ignore this glyph.
	if (fnt->type == BMFONT) return 0;

	// For truetype fonts: create this glyph.
	float scale = stbtt_ScaleForPixelHeight(&fnt->font, isize / 10.0f);
	int g = stbtt_FindGlyphIndex(&fnt->font, codepoint);
	if(!g) return 0; /* @rlyeh: glyph not found, ie, arab chars */
    int advance, lsb, x0, y0, x1, y1, gw, gh;
	stbtt_GetGlyphHMetrics(&fnt->font, g, &advance, &lsb);
	stbtt_GetGlyphBitmapBox(&fnt->font, g, scale,scale, &x0, &y0, &x1, &y1);
	gw = x1 - x0;
	gh = y1 - y0;

	// Check if glyph is larger than maximum texture size
	if (gw >= stash->tw || gh >= stash->th) return 0;

	// Find texture and row where the glyph can be fit.
    sth_row* br = NULL;
	int rh = (gh + 7) & ~7;
	struct sth_texture* texture = stash->tt_textures;
	while(!br) {
		for (i = 0; i < texture->nrows; ++i)
			if (texture->rows[i].h == rh && texture->rows[i].x + gw + 1 <= stash->tw)
				br = &texture->rows[i];

		// If no row is found, there are 3 possibilities:
		//   - add new row
		//   - try next texture
		//   - create new texture
		if (!br) {
			short py = 0;
			// Check that there is enough space.
			if(texture->nrows) {
				py = texture->rows[texture->nrows - 1].y + texture->rows[texture->nrows - 1].h + 1;
				if(py + rh > stash->th) {
					if(texture->next) texture = texture->next;
					else {
						// Create new texture
						texture->next = (struct sth_texture*) malloc(sizeof(struct sth_texture));
						texture = texture->next;
						if (texture == NULL) goto error;
						memset(texture, 0 ,sizeof(struct sth_texture));
						glGenTextures(1, &texture->id);
						if (!texture->id) goto error;
						glBindTexture(GL_TEXTURE_2D, texture->id);
						glTexImage2D(GL_TEXTURE_2D, 0, STH_GL_TEXTYPE, stash->tw, stash->th, 0, STH_GL_TEXTYPE, GL_UNSIGNED_BYTE, 0);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					}
					continue;
				}
			}
			// Init and add row
			br = &texture->rows[texture->nrows];
			br->x = 0;
			br->y = py;
			br->h = rh;
			texture->nrows++;
		}
	}

	// Alloc space for new glyph.
	fnt->nglyphs++;
	fnt->glyphs = (sth_glyph *) realloc(fnt->glyphs, fnt->nglyphs * sizeof(sth_glyph)); /* @rlyeh: explicit cast needed in C++ */
	if (!fnt->glyphs) return 0;

	// Init glyph.
    sth_glyph* glyph = fnt->glyphs + (fnt->nglyphs - 1);
	memset(glyph, 0, sizeof(sth_glyph));
	glyph->codepoint = codepoint;
	glyph->size = isize;
	glyph->texture = texture;
	glyph->x0 = br->x;
	glyph->y0 = br->y;
	glyph->x1 = glyph->x0 + gw;
	glyph->y1 = glyph->y0 + gh;
	glyph->xadv = scale * advance;
	glyph->xoff = (float) x0;
	glyph->yoff = (float) y0;
	glyph->next = 0;

	// Advance row location.
	br->x += gw + 1;

	// Insert char to hash lookup.
	glyph->next = fnt->lut[h];
	fnt->lut[h] = fnt->nglyphs - 1;

	// Rasterize
    unsigned char* bmp = (unsigned char*) malloc(gw * gh);
	if(bmp) {
		stbtt_MakeGlyphBitmap(&fnt->font, bmp, gw, gh, gw, scale,scale, g);
		// Update texture
		glBindTexture(GL_TEXTURE_2D, texture->id);
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glTexSubImage2D(GL_TEXTURE_2D, 0, glyph->x0, glyph->y0, gw, gh, STH_GL_TEXTYPE, GL_UNSIGNED_BYTE, bmp);
		free(bmp);
	}

	return glyph;

error:
	if(texture) free(texture);
	return 0;
}

static void get_quad(sth_stash* stash, sth_font* fnt, sth_glyph* glyph, short isize, float* x, float* y, sth_quad* q) {
	float scale = 1.0f;

	if (fnt->type == BMFONT) scale = isize / (glyph->size * 10.0f);

	float rx = *x + scale * glyph->xoff;
	float ry = *y - scale * glyph->yoff;

	q->x0 = rx * stash->screen2gl_x - 1;
	q->y0 = ry * stash->screen2gl_y + 1;
	q->x1 = (rx + scale * (glyph->x1 - glyph->x0)) * stash->screen2gl_x - 1;
	q->y1 = (ry - scale * (glyph->y1 - glyph->y0)) * stash->screen2gl_y + 1;

	q->s0 = (glyph->x0) * stash->itw;
	q->t0 = (glyph->y0) * stash->ith;
	q->s1 = (glyph->x1) * stash->itw;
	q->t1 = (glyph->y1) * stash->ith;

	*x += scale * glyph->xadv;
}

static float* setv(float* v, float x, float y, float s, float t) {
	v[0] = x;
	v[1] = y;
	v[2] = s;
	v[3] = t;
	return v + 4;
}

static void flush_draw(sth_stash* stash) {
	struct sth_texture* texture = stash->tt_textures;
	short tt = 1;
	while (texture) {
		if (texture->nverts > 0) {
            unsigned indiceCount = texture->nverts * 3 / 2;
            if(stash->indiceCount < indiceCount) {
                stash->indiceCount = indiceCount;
                stash->elementIndices = (GLushort*) realloc(stash->elementIndices, sizeof(GLushort) * indiceCount);
                unsigned fOffset = 0;
                for(GLushort *pIndex = stash->elementIndices, *lastIndex = pIndex + indiceCount; pIndex < lastIndex;) {
                    *pIndex++ = fOffset + 0;
                    *pIndex++ = fOffset + 1;
                    *pIndex++ = fOffset + 3;

                    *pIndex++ = fOffset + 1;
                    *pIndex++ = fOffset + 2;
                    *pIndex++ = fOffset + 3;
                    fOffset += 4;
                }
            }
            // VAO
            glBindVertexArray(stash->vao);
            // Load vertexes into OpenGL
            glBindBuffer(GL_ARRAY_BUFFER, stash->vbo);
            glBufferData(GL_ARRAY_BUFFER, texture->nverts * 4 * sizeof(float), texture->verts, GL_STATIC_DRAW);

            // Load element buffer into OpenGL
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stash->ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indiceCount, stash->elementIndices, GL_STATIC_DRAW);

            // Attributes
            // 1st attribute buffer: vertices
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, VERT_STRIDE, (void*) 0);

            // 2nd attribute buffer: uv textures
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, VERT_STRIDE, (void*) (sizeof(float) * 2));
            glBindVertexArray(0);

            // Start OpenGL Stuff.
            glUseProgram(stash->programID);

            // Setup color
            glUniform4fv(stash->colorID, 1, stash->color);

            // Texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture->id);
            glUniform1i(stash->textureID, 0);  // Set our "myTextureSampler" sampler to user Texture Unit 0

            // Draw
            glBindVertexArray(stash->vao);
            glDrawElements(GL_TRIANGLES, indiceCount, GL_UNSIGNED_SHORT, 0);
            //glBindVertexArray(0);
            glBindTexture(GL_TEXTURE_2D, 0);
           
            texture->nverts = 0;
		}
		texture = texture->next;
		if (!texture && tt) {
			texture = stash->bm_textures;
			tt = 0;
		}
	}
}

void sth_begin_draw(sth_stash* stash) {
	if (!stash) return;
	if (stash->drawing) flush_draw(stash);
	stash->drawing = 1;
}

void sth_end_draw(sth_stash* stash) {
	if (!stash) return;
	if (!stash->drawing) return;
	flush_draw(stash);
	stash->drawing = 0;
}

void sth_draw_text(sth_stash* stash, int idx, float size, float x, float y, const char* s, float* dx) {
    if(!stash) return;
	unsigned codepoint;
	unsigned state = 0;
	short isize = (short) (size * 10.0f);
    sth_font* fnt  = stash->fonts;
    y = -y;
    
	while(fnt && fnt->idx != idx) fnt = fnt->next;
	if(!fnt) return;
	if(fnt->type != BMFONT && !fnt->data) return;
	for(; *s; ++s) {
		if(decutf8(&state, &codepoint, *(unsigned char*)s)) continue;
        sth_glyph* glyph = get_glyph(stash, fnt, codepoint, isize);
		if(!glyph) continue;
		struct sth_texture* texture = glyph->texture;
        sth_quad q;
		if(texture->nverts + 4 >= VERT_COUNT) flush_draw(stash);
		get_quad(stash, fnt, glyph, isize, &x, &y, &q);
		float *v = &texture->verts[texture->nverts * 4];

		v = setv(v, q.x0, q.y0, q.s0, q.t0);
		v = setv(v, q.x1, q.y0, q.s1, q.t0);
		v = setv(v, q.x1, q.y1, q.s1, q.t1);
		v = setv(v, q.x0, q.y1, q.s0, q.t1);

		texture->nverts += 4;
	}
	if(dx) *dx = x;
}

void sth_dim_text(sth_stash* stash, int idx, float size, const char* s, float* minx, float* miny, float* maxx, float* maxy) {
	if(!stash) return;
    unsigned int codepoint;
	unsigned int state = 0;
	
	short isize = (short) (size * 10.0f);
	float x = 0, y = 0;

	*minx = *maxx = *miny = *maxy = 0;	/* @rlyeh: reset vars before failing */

    sth_font* fnt = stash->fonts;
	while(fnt && fnt->idx != idx) fnt = fnt->next;
	if(!fnt) return;
	if(fnt->type != BMFONT && !fnt->data) return;

	for(; *s; ++s) {
		if (decutf8(&state, &codepoint, *(unsigned char*) s)) continue;
        sth_glyph* glyph = get_glyph(stash, fnt, codepoint, isize);
		if (!glyph) continue;
        sth_quad q;
		get_quad(stash, fnt, glyph, isize, &x, &y, &q);
		if (q.x0 < *minx) *minx = q.x0;
		if (q.x1 > *maxx) *maxx = q.x1;
		if (q.y1 < *miny) *miny = q.y1;
		if (q.y0 > *maxy) *maxy = q.y0;
	}
	if (floorf(x) > *maxx) *maxx = floorf(x);
}

void sth_vmetrics(sth_stash* stash, int idx, float size, float* ascender, float* descender, float* lineh) {
	if (!stash) return;
    sth_font* fnt = stash->fonts;
	while(fnt && fnt->idx != idx) fnt = fnt->next;
	if(!fnt) return;
	if(fnt->type != BMFONT && !fnt->data) return;
	if(ascender) *ascender = fnt->ascender*size;
	if(descender) *descender = fnt->descender*size;
	if(lineh) *lineh = fnt->lineh*size;
}

void sth_delete(sth_stash* stash) {
	if (!stash) return;

    glDeleteProgram(stash->programID);
    glDeleteVertexArrays(1, &stash->vao);
    glDeleteBuffers(1, &stash->vbo);
    glDeleteBuffers(1, &stash->ebo);

	struct sth_texture* tex = stash->tt_textures, *curtex = NULL;
	while(tex) {
		curtex = tex;
		tex = tex->next;
		if(curtex->id) glDeleteTextures(1, &curtex->id);
		free(curtex);
	}

	tex = stash->bm_textures;
	while(tex) {
		curtex = tex;
		tex = tex->next;
		if(curtex->id) glDeleteTextures(1, &curtex->id);
		free(curtex);
	}

    sth_font* fnt = stash->fonts, *curfnt;
	while(fnt) {
		curfnt = fnt;
		fnt = fnt->next;
		if(curfnt->glyphs) free(curfnt->glyphs);
		if(curfnt->type == TTFONT_FILE && curfnt->data) free(curfnt->data);
		free(curfnt);
	}
	free(stash->elementIndices);
	free(stash);
}
