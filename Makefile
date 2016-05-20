FILE   = main
SRCS   = $(wildcard *.c)
OBJS   = $(SRCS:%.c=%.o)
BINDIR = bin

CC      = g++
CFLAGS  = -Wall -Wextra -g
CFLAGS  += `sdl2-config --cflags` `pkg-config --cflags SDL2_image`
LDFLAGS = -lGL
LDFLAGS += `sdl2-config --libs` `pkg-config --libs SDL2_image`

all : $(FILE)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(FILE): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)
	mv $(FILE) $(BINDIR)/

clean:
	@rm -rfv *.o 2> /dev/null

proper: clean
	@rm -fv $(BINDIR)/$(FILE) *~ 2> /dev/null

run : $(FILE)
	cd $(BINDIR) && ./$(FILE)

.PHONY: clean, proper, run
