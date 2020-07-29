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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <SDL2/SDL.h>
#include "pnginput.h"
#include "channel.h"

struct channel_s
{
    SDL_Renderer *renderer;
    const char *filename;
    int screen_width;
    int screen_height;
    int fullscreen;

    SDL_Texture *texture;
    int t_width;
    int t_height;

    int x_offset;
    int y_offset;
    int a_offset;

    int x_control;
    int y_control;
    int a_control;

    time_t last_mtime;

    SDL_Rect src_rect;

    int dst_skiprender;
    int dst_alpha;
    SDL_Rect dst_rect;

    int lst_skiprender;
    int lst_alpha;
    SDL_Rect lst_rect;
};

channel_t *channel_new( SDL_Renderer *renderer, const char *filename,
                        int screen_width, int screen_height, int fullscreen )
{
    channel_t *channel = malloc( sizeof( channel_t ) );
    channel->renderer = renderer;
    channel->filename = filename;
    channel->screen_width = screen_width;
    channel->screen_height = screen_height;
    channel->fullscreen = fullscreen;

    channel->texture = NULL;
    channel->t_width = 0;
    channel->t_height = 0;

    channel->x_offset = 0;
    channel->y_offset = 0;
    channel->a_offset = 0xff;

    channel->x_control = 0;
    channel->y_control = 0;
    channel->a_control = 0;

    channel->last_mtime = 0;

    channel->src_rect.x = 0;
    channel->src_rect.y = 0;
    channel->src_rect.w = 0;
    channel->src_rect.h = 0;

    channel->dst_skiprender = 0;
    channel->dst_alpha = 0;
    channel->dst_rect.x = 0;
    channel->dst_rect.y = 0;
    channel->dst_rect.w = 0;
    channel->dst_rect.h = 0;

    channel->lst_skiprender = 0;
    channel->lst_alpha = 0;
    channel->lst_rect.x = 0;
    channel->lst_rect.y = 0;
    channel->lst_rect.w = 0;
    channel->lst_rect.h = 0;

    return channel;
}

void channel_delete( channel_t *channel )
{
    if( channel->texture ) {
        SDL_DestroyTexture( channel->texture );
    }
    free( channel );
}

int *channel_get_x_offset( channel_t *channel )
{
    return &channel->x_offset;
}

int *channel_get_y_offset( channel_t *channel )
{
    return &channel->y_offset;
}

int *channel_get_a_offset( channel_t *channel )
{
    return &channel->a_offset;
}

int *channel_get_a_control( channel_t *channel )
{
    return &channel->a_control;
}

int *channel_get_x_control( channel_t *channel )
{
    return &channel->x_control;
}

int *channel_get_y_control( channel_t *channel )
{
    return &channel->y_control;
}

static void channel_loadpng( channel_t *channel )
{
    if( channel->texture ) {
        SDL_DestroyTexture( channel->texture );
        channel->texture = NULL;
    }

    pnginput_t *png = pnginput_new( channel->filename );
    unsigned int width = pnginput_get_width( png );
    unsigned int height = pnginput_get_height( png );
    unsigned char *data = malloc( width * height * 4 );
    int has_alpha = pnginput_has_alpha( png );
    int depth, stride;

    fprintf( stderr, "channel: loaded %s: alpha: %d, w %d, h %d\n",
             channel->filename, has_alpha, width, height );

    if( !has_alpha ) {
        depth = 24;
        stride = 3 * width;
    } else {
        depth = 32;
        stride = 4 * width;
    }

    for( int i = 0; i < height; i++ ) {
        memcpy( data + (i * stride), pnginput_get_scanline( png, i ), stride );
    }

    Uint32 red   = 0x000000ff;
    Uint32 green = 0x0000ff00;
    Uint32 blue  = 0x00ff0000;
    Uint32 alpha = has_alpha? 0xff000000 : 0;
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom( (void *) data,
        width, height, depth, stride, red, green, blue, alpha );
    if( !surface ) {
        fprintf( stderr, "channel: failed to create surface: %s\n", SDL_GetError() );
    } else {
        channel->texture = SDL_CreateTextureFromSurface( channel->renderer, surface );
        if( channel->fullscreen ) {
            SDL_SetTextureBlendMode( channel->texture, SDL_BLENDMODE_BLEND );
        }
        channel->t_width = width;
        channel->t_height = height;
        channel->src_rect.x = 0;
        channel->src_rect.y = 0;
        channel->src_rect.w = width;
        channel->src_rect.h = height;
        SDL_FreeSurface( surface );
    }
    pnginput_delete( png );
}


void channel_checkfile( channel_t *channel )
{
    struct stat s;

    if( stat( channel->filename, &s ) == 0 ) {
        if( s.st_mtime != channel->last_mtime ) {
            channel->last_mtime = s.st_mtime;
            channel_loadpng( channel );
        }
    }
}

static int calc_offset( int size, int max, int controller )
{
    if( !controller ) return (0 - size);
    float fc = (float) controller;
    float fm = (float) max;
    float fs = (float) size;
    float val = ((fc / 127.0) * (fm + fs)) - fs;
    return ((int) val);
}

static int calc_control( int max, int controller )
{
    if( !controller ) return 0;
    float fm = (float) max;
    float fc = (float) controller;
    float val = (fc / 32768.0) * (fm * 2.0); // scale audio gain?
    return ((int) val);
}

static int channel_skiprender( channel_t *channel )
{
    if( (channel->dst_rect.x + channel->dst_rect.w) < 0 ) return 1;
    if( (channel->dst_rect.y + channel->dst_rect.h) < 0 ) return 1;
    if( channel->dst_rect.x >= channel->screen_width ) return 1;
    if( channel->dst_rect.y >= channel->screen_height ) return 1;

    if( channel->fullscreen ) {
        if( channel->dst_alpha == 0 ) return 1;
    }
    return 0;
}

int channel_prepare( channel_t *channel )
{
    if( !channel->texture ) return 0;

    int x_offset = calc_offset( channel->t_width, channel->screen_width,
                                channel->x_offset );
    int y_offset = calc_offset( channel->t_height, channel->screen_height,
                                127 - channel->y_offset );
    int a_offset = calc_offset( 0, 0xff, channel->a_offset );

    int x_control = calc_control( channel->screen_width, channel->x_control );
    int y_control = calc_control( channel->screen_height, channel->y_control );
    int a_control = calc_control( 0xff, channel->a_control );

    if( channel->fullscreen ) {
        channel->dst_rect.x = 0;
        channel->dst_rect.y = 0;
        channel->dst_rect.w = channel->t_width;
        channel->dst_rect.h = channel->t_height;
        channel->dst_alpha = a_offset + a_control;
    } else {
        channel->dst_rect.x = x_offset + x_control;
        channel->dst_rect.y = y_offset + y_control;
        channel->dst_rect.w = channel->t_width;
        channel->dst_rect.h = channel->t_height;
    }

    channel->dst_skiprender = channel_skiprender( channel );

    if( channel->dst_skiprender && channel->lst_skiprender ) {
        return 0;
    }

    if( (channel->dst_rect.x == channel->lst_rect.x) &&
        (channel->dst_rect.y == channel->lst_rect.y) &&
        (channel->dst_rect.w == channel->lst_rect.w) &&
        (channel->dst_rect.h == channel->lst_rect.h) &&
        (channel->dst_alpha == channel->lst_alpha) ) {
        return 0;
    }

    return 1;
}

void channel_render( channel_t *channel )
{
    if( !channel->texture ) return;

    if( !channel->dst_skiprender ) {
        if( channel->fullscreen ) {
            SDL_SetTextureAlphaMod( channel->texture, channel->dst_alpha );
        }
        SDL_RenderCopy( channel->renderer, channel->texture,
                        &channel->src_rect, &channel->dst_rect );
    }

    channel->lst_skiprender = channel->dst_skiprender;
    channel->lst_alpha = channel->dst_alpha;
    channel->lst_rect.x = channel->dst_rect.x;
    channel->lst_rect.y = channel->dst_rect.y;
    channel->lst_rect.w = channel->dst_rect.w;
    channel->lst_rect.h = channel->dst_rect.h;
}

