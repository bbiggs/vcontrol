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
#include <sys/time.h>
#include <alsa/asoundlib.h>
#include "minput.h"

#define MAX_TARGETS 32

struct minput_s
{
    snd_rawmidi_t *midi_in;
    int c_active;
    int c_id;
    int *targets[ 32 ];
};

minput_t *minput_new( const char *portname )
{
    minput_t *minput = malloc( sizeof( minput_t ) );
    int mode = SND_RAWMIDI_NONBLOCK;
    int status;

    status = snd_rawmidi_open( &minput->midi_in, NULL, portname, mode );
    if( status < 0 ) {
        fprintf( stderr, "minput: cannot open midi device: %s\n",
                 snd_strerror( status ) );
        free( minput );
        return 0;
    }

    minput->c_active = 0;
    minput->c_id = -1;
    for( int i = 0; i < MAX_TARGETS; i++ ) {
        minput->targets[ i ] = NULL;
    }
    return minput;
}

void minput_delete( minput_t *minput )
{
    snd_rawmidi_close( minput->midi_in );
    free( minput );
}

void minput_set_control( minput_t *minput, int controller, int *target )
{
    minput->targets[ controller ] = target;
}

void minput_check( minput_t *minput )
{
    int status;
    char buffer[ 1 ];
    for(;;) {
        status = snd_rawmidi_read( minput->midi_in, buffer, 1 );
        if( status == -EAGAIN ) return;
        if( status >= 0 ) {
            if( minput->c_id >= 0 ) {
                int id = minput->c_id;
                if( id >= 0 && id < MAX_TARGETS ) {
                    if( minput->targets[ id ] ) {
                        // fprintf( stderr, "MIDI: id %d, value %d\n", id, buffer[ 0 ] );
                        *minput->targets[ id ] = buffer[ 0 ];
                    }
                }
                minput->c_active = 0;
                minput->c_id = -1;
            } else if( minput->c_active ) {
                minput->c_id = buffer[ 0 ];
            } else if( buffer[ 0 ] == 0xb0 ) {
                minput->c_active = 1;
            }
        }
    }
}

