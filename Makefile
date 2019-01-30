CC=gcc
CFLAGS="-Wall"
LIBS=-lGL -lGLEW -lc -lm -lglfw
stable:clean
	$(CC) $(CFLAGS) $(LIBS) -o fontstashtest main.c stb_truetype.c fontstash.c
clean:
	rm -vfr *~ fontstashtest
