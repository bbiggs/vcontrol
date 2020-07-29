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
#include <pthread.h>
#include "ainput.h"

struct ainput_s
{
    snd_pcm_t *audio_in;
    int *target;
    pthread_t thread_handle;
};

ainput_t *ainput_new( const char *portname )
{
    ainput_t *ainput = malloc( sizeof( ainput_t ) );
    int mode = SND_PCM_STREAM_CAPTURE;
    snd_pcm_hw_params_t *hw_params;
    int status;

    ainput->thread_handle = 0;

    status = snd_pcm_open( &ainput->audio_in, portname, mode, 0 );
    if( status < 0 ) {
        fprintf( stderr, "ainput: fail open: %s\n",
                 snd_strerror( status ) );
        free( ainput );
        return 0;
    }

    status = snd_pcm_hw_params_malloc( &hw_params );
    if( status < 0 ) {
        fprintf( stderr, "ainput: failed to alloc audio params: %s\n",
                 snd_strerror( status ) );
        free( ainput );
        return 0;
    }

    status = snd_pcm_hw_params_any( ainput->audio_in, hw_params );
    if( status < 0 ) {
        fprintf( stderr, "ainput: failed init hw params: %s\n", snd_strerror( status ) );
        free( ainput );
        return 0;
    }

    status = snd_pcm_hw_params_set_access( ainput->audio_in, hw_params,
        SND_PCM_ACCESS_RW_INTERLEAVED );
    if( status < 0 ) {
        fprintf( stderr, "ainput: cannot interleaved: %s\n", snd_strerror( status ) );
        free( ainput );
        return 0;
    }

    status = snd_pcm_hw_params_set_format( ainput->audio_in, hw_params,
        SND_PCM_FORMAT_S16 );
    if( status < 0 ) {
        fprintf( stderr, "ainput: cannot s16: %s\n", snd_strerror( status ) );
        free( ainput );
        return 0;
    }

    unsigned int sample_rate = 44100;
    status = snd_pcm_hw_params_set_rate_near( ainput->audio_in, hw_params,
        &sample_rate, 0 );
    if( status < 0 ) {
        fprintf( stderr, "ainput: failed to set sample rate: %s\n",
                 snd_strerror( status ) );
        free( ainput );
        return 0;
    }
    fprintf( stderr, "ainput: actual sample rate: %d\n", sample_rate );

    status = snd_pcm_hw_params_set_channels( ainput->audio_in, hw_params, 1 );
    if( status < 0 ) {
        fprintf( stderr, "ainput: cannot mono: %s\n", snd_strerror( status ) );
        free( ainput );
        return 0;
    }

    status = snd_pcm_hw_params( ainput->audio_in, hw_params );
    if( status < 0 ) {
        fprintf( stderr, "ainput: cannot audio params: %s\n", snd_strerror( status ) );
        free( ainput );
        return 0;
    }

    snd_pcm_hw_params_free( hw_params );

    status = snd_pcm_prepare( ainput->audio_in );
    if( status < 0 ) {
        fprintf( stderr, "ainput: failed to prepare: %s\n", snd_strerror( status ) );
        free( ainput );
        return 0;
    }

    ainput->target = NULL;
    return ainput;
}

void ainput_delete( ainput_t *ainput )
{
    if( ainput->thread_handle ) {
        fprintf( stderr, "ainput: cancel audio thread\n" );
        pthread_cancel( ainput->thread_handle );
        pthread_join( ainput->thread_handle, NULL );
        fprintf( stderr, "ainput: audio thread joined!\n" );
    }
    snd_pcm_close( ainput->audio_in );
    free( ainput );
}

void ainput_set_control( ainput_t *ainput, int *target )
{
    ainput->target = target;
}

// static int throttle = 0;

static void ainput_check( ainput_t *ainput )
{
    snd_pcm_sframes_t frames_read;
    short buffer[ 512 ];
    float state = 0.0f;
    for(;;) {
        pthread_testcancel();
        frames_read = snd_pcm_readi( ainput->audio_in, buffer, 512 );
        if( frames_read > 0 ) {
            // throttle = (throttle + 1) % 100;
            // if( throttle == 0 ) fprintf( stderr, "val %d\n", buffer[ 0 ] );
            // smooth a little?
            float cur = (float) abs( buffer [ 0 ] );
            state = (state * 0.6f) + (cur * 0.4f);
            if( ainput->target ) *ainput->target = ((int) state); //abs( buffer[ 0 ] );
        }
    }
}

void ainput_thread_main( ainput_t *ainput )
{
    fprintf( stderr, "ainput: thread main\n" );
    ainput_check( ainput );
}

static void *thread_thunk( void *ain )
{
    pthread_setcanceltype( PTHREAD_CANCEL_DEFERRED, 0 );
    ainput_thread_main( ain );
    return NULL;
}

void ainput_start( ainput_t *ainput )
{
    if( pthread_create( &ainput->thread_handle, NULL, thread_thunk, ainput ) != 0 ) {
        fprintf( stderr, "ainput: failed to create audio thread\n" );
    }
}


