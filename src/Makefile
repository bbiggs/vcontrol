
SDL_FLAGS = `sdl2-config --cflags --libs`
LIBS = `sdl2-config --libs` -lpng -lasound -lpthread -lz -lm
SRCS = pnginput.c channel.c minput.c ainput.c

vcontrol: vcontrol.c ${SRCS}
	gcc -g -Wall -std=c99 -o $@ -I. -I../include $^ ${SDL_FLAGS} ${LIBS}

