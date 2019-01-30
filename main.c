#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "fontstash.h"

int loadFonts(struct sth_stash* stash, int *droidRegular, int *droidItalic, int *droidBold, int *droidJapanese, int *dejavu) {
    // Load the first truetype font from memory (just because we can).
	FILE *fp = fopen("DroidSerif-Regular.ttf", "rb");
	if (!fp)  return 0;
	fseek(fp, 0, SEEK_END);
	int datasize = (int) ftell(fp);
	fseek(fp, 0, SEEK_SET);
	unsigned char *data = (unsigned char*) malloc(datasize);
	if (!data) return 0;
	fread(data, 1, datasize, fp);
	fclose(fp);
	fp = 0;
	
	if (!(*droidRegular = sth_add_font_from_memory(stash, data))) return 0;
	// Load the remaining truetype fonts directly.
	if (!(*droidItalic = sth_add_font(stash,"DroidSerif-Italic.ttf"))) return 0;
	if (!(*droidBold = sth_add_font(stash,"DroidSerif-Bold.ttf"))) return 0;
	if (!(*droidJapanese = sth_add_font(stash,"DroidSansJapanese.ttf"))) return 0;
    
    /*GLuint texture;
    glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA8, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, 0);
    
    if (!(*dejavu = sth_add_bitmap_font(stash, 17, -4, 2))) return 0;
	sth_add_glyph_for_char(stash, *dejavu, texture, "T", 18, 14, 363, 331, 10, 11, 0,  3, 10);
	sth_add_glyph_for_char(stash, *dejavu, texture, "h", 18, 14, 225, 382,  8, 11, 1,  3, 10);
	sth_add_glyph_for_char(stash, *dejavu, texture, "i", 18, 14, 478, 377,  2, 11, 1,  3,  4);
	sth_add_glyph_for_char(stash, *dejavu, texture, "s", 18, 14, 199, 455,  7,  8, 1,  6,  9);
	sth_add_glyph_for_char(stash, *dejavu, texture, " ", 18, 14,  66, 185,  1,  1, 0, 14,  5);
	sth_add_glyph_for_char(stash, *dejavu, texture, "a", 18, 14,  18, 459,  8,  8, 1,  6, 10);
	sth_add_glyph_for_char(stash, *dejavu, texture, "b", 18, 14, 198, 383,  8, 11, 1,  3, 10);
	sth_add_glyph_for_char(stash, *dejavu, texture, "t", 18, 14, 436, 377,  5, 11, 1,  3,  6);
	sth_add_glyph_for_char(stash, *dejavu, texture, "m", 18, 14, 494, 429, 12,  8, 2,  6, 16);
	sth_add_glyph_for_char(stash, *dejavu, texture, "p", 18, 14, 436, 353,  8, 11, 1,  6, 10);
	sth_add_glyph_for_char(stash, *dejavu, texture, "f", 18, 14, 442, 377,  5, 11, 1,  3,  7);
	sth_add_glyph_for_char(stash, *dejavu, texture, "o", 18, 14, 483, 438,  8,  8, 1,  6, 10);
	sth_add_glyph_for_char(stash, *dejavu, texture, "n", 18, 14,   0, 459,  8,  8, 1,  6, 10);
	sth_add_glyph_for_char(stash, *dejavu, texture, ".", 18, 14, 285, 476,  2,  3, 1, 11,  6);*/
    return 1;
}

int main(int argc, char **argv) {
    unsigned w = 1280, h = 720;
    int error = glfwInit();
    if(error != GLFW_TRUE) {
        printf("Failed to initialize GLFW, error %d\n", error);
        return -1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Open a window and create its OpenGL context
    GLFWwindow* window = glfwCreateWindow(w, h, "app", 0, 0);
   // *((GLFWwindow**) context) = window;
    if(!window){
        fprintf( stderr, "Failed to open GLFW window\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // Initialize GLEW
    glewExperimental = GLEW_OK; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        printf("Failed to initialize GLEW\n");
        glfwTerminate();
        return -1;
    }

    struct sth_stash* stash = sth_create(512, 512);
	if(!stash) fprintf(stderr, "Could not create stash.\n");
    int droidRegular, droidItalic, droidBold, droidJapanese, dejavu;
	loadFonts(stash, &droidRegular, &droidItalic, &droidBold, &droidJapanese, &dejavu);
    sth_set_screen_size(stash, w, h);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
   
    do {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        float sx = 50, sy = 150, lh;
		float dx = sx, dy = sy, scale = 3.0f;
        
        sth_begin_draw(stash);
        sth_color(stash, 1.0f, 1.0f, 1.0f, 1.0f);
		sth_draw_text(stash, droidRegular, 24.0f * scale, dx, dy, "The quick ", &dx);
		sth_draw_text(stash, droidItalic, 48.0f * scale, dx, dy, "brown ", &dx);
		sth_draw_text(stash, droidRegular, 24.0f * scale, dx, dy, "fox ", &dx);
		sth_vmetrics(stash, droidItalic, 24 * scale, NULL, NULL, &lh);
        sth_end_draw(stash);


		dx = sx;
		dy += lh*1.2f;
        sth_begin_draw(stash);
        sth_color(stash, 1.0f, 0.5f, 0.5f, 1.0f);
		sth_draw_text(stash, droidItalic, 24.0f * scale, dx, dy, "jumps over ", &dx);
		sth_draw_text(stash, droidBold, 24.0f * scale, dx, dy, "the lazy ", &dx);
		sth_draw_text(stash, droidRegular, 24.0f * scale, dx, dy, "dog.", &dx);
        sth_end_draw(stash);

		dx = sx;
		dy += lh*1.2f;
        sth_begin_draw(stash);
        sth_color(stash, 0.5f, 1.0f, 0.5f, 1.0f);
		sth_draw_text(stash, droidRegular, 12.0f * scale, dx, dy, "Now is the time for all good men to come to the aid of the party.", &dx);
		sth_vmetrics(stash, droidItalic, 12 * scale, NULL, NULL, &lh);
        sth_end_draw(stash);
        
		dx = sx;
		dy += lh*1.2f*2;
        sth_begin_draw(stash);
        sth_color(stash, 1.0f, 1.0f, 0.4f, 1.0f);
		sth_draw_text(stash, droidItalic, 18.0f * scale, dx, dy, "Ég get etið gler án þess að meiða mig.", &dx);
		sth_vmetrics(stash, droidItalic, 18 * scale, NULL, NULL, &lh);
		dx = sx;
		dy += lh*1.2f;
        
		sth_draw_text(stash, droidJapanese, 18.0f * scale, dx, dy, "私はガラスを食べられます。それは私を傷つけません。", &dx);
		dx = sx;
		dy += lh*1.2f*2;
        
		//sth_draw_text(stash, dejavu, 18.0f * scale, dx, dy, "This is a bitmap font.", &dx);

		sth_end_draw(stash);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
    glfwTerminate();

    return 0;
}
