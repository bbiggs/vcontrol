/**
 * Copyright (C) 2020 Billy Biggs <vektor@dumbterm.net>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef CHANNEL_H_INCLUDED
#define CHANNEL_H_INCLUDED

#include <stdint.h>
#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct channel_s channel_t;

channel_t *channel_new( SDL_Renderer *renderer, const char *filename,
                        int screen_width, int screen_height, int fullscreen );
void channel_delete( channel_t *channel );
int *channel_get_x_offset( channel_t *channel );
int *channel_get_y_offset( channel_t *channel );
int *channel_get_a_offset( channel_t *channel );
int *channel_get_a_control( channel_t *channel );
int *channel_get_x_control( channel_t *channel );
int *channel_get_y_control( channel_t *channel );
void channel_checkfile( channel_t *channel );
int channel_prepare( channel_t *channel );
void channel_render( channel_t *channel );

#ifdef __cplusplus
};
#endif
#endif /* CHANNEL_H_INCLUDED */
