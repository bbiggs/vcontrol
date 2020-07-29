#include <stdio.h>
#include <SDL2/SDL.h>
#include "channel.h"
#include "minput.h"
#include "ainput.h"

int main( int argc, char **argv )
{
    int width = 720;
    int height = 480;

    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        fprintf( stderr, "SDL_Init failed.\n" );
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow( "vcontrol",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height, SDL_WINDOW_FULLSCREEN );

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED );

    SDL_ShowCursor( SDL_DISABLE );

    // midi
    minput_t *minput = minput_new( "hw:2,0,0" );
    // audio
    ainput_t *ainput = ainput_new( "hw:3,0,0" );

    // Sprite channels
    channel_t *ch0 = channel_new( renderer, "ch0.png", width, height, 0 );
    minput_set_control( minput, 0, channel_get_y_offset( ch0 ) );
    minput_set_control( minput, 16, channel_get_x_offset( ch0 ) );

    channel_t *ch1 = channel_new( renderer, "ch1.png", width, height, 0 );
    minput_set_control( minput, 1, channel_get_y_offset( ch1 ) );
    minput_set_control( minput, 17, channel_get_x_offset( ch1 ) );

    channel_t *ch2 = channel_new( renderer, "ch2.png", width, height, 0 );
    minput_set_control( minput, 2, channel_get_y_offset( ch2 ) );
    minput_set_control( minput, 18, channel_get_x_offset( ch2 ) );

    channel_t *ch3 = channel_new( renderer, "ch3.png", width, height, 0 );
    minput_set_control( minput, 3, channel_get_y_offset( ch3 ) );
    minput_set_control( minput, 19, channel_get_x_offset( ch3 ) );

    channel_t *ch4 = channel_new( renderer, "ch4.png", width, height, 0 );
    minput_set_control( minput, 4, channel_get_y_offset( ch4 ) );
    minput_set_control( minput, 20, channel_get_x_offset( ch4 ) );

    // Background channels
    channel_t *ch5 = channel_new( renderer, "ch5.png", width, height, 1 );
    minput_set_control( minput, 5, channel_get_a_offset( ch5 ) );

    channel_t *ch6 = channel_new( renderer, "ch6.png", width, height, 1 );
    minput_set_control( minput, 6, channel_get_a_offset( ch6 ) );

    channel_t *ch7 = channel_new( renderer, "ch7.png", width, height, 1 );
    minput_set_control( minput, 7, channel_get_a_offset( ch7 ) );

    // Audio moves sprite 1
    ainput_set_control( ainput, channel_get_y_control( ch1 ) );
    ainput_start( ainput );

    SDL_Event event;
    int quit = 0;
    int frames = 0;

    while( !quit ) {
        while( SDL_PollEvent( &event ) ) {
            if( event.type == SDL_QUIT ) quit = 1;
        }

        // check for new files, but not too often
        frames = (frames + 1) % 80;
        if( frames == 0 ) {
            channel_checkfile( ch0 );
        } else if( frames == 10 ) {
            channel_checkfile( ch1 );
        } else if( frames == 20 ) {
            channel_checkfile( ch2 );
        } else if( frames == 30 ) {
            channel_checkfile( ch3 );
        } else if( frames == 40 ) {
            channel_checkfile( ch4 );
        } else if( frames == 50 ) {
            channel_checkfile( ch5 );
        } else if( frames == 60 ) {
            channel_checkfile( ch6 );
        } else if( frames == 70 ) {
            channel_checkfile( ch7 );
        }

        // update midi input
        minput_check( minput );

        int r = 0;
        r += channel_prepare( ch0 );
        r += channel_prepare( ch1 );
        r += channel_prepare( ch2 );
        r += channel_prepare( ch3 );
        r += channel_prepare( ch4 );
        r += channel_prepare( ch5 );
        r += channel_prepare( ch6 );
        r += channel_prepare( ch7 );

        if( r > 0 ) {
            SDL_RenderClear( renderer );

            // Background channels
            channel_render( ch5 );
            channel_render( ch6 );
            channel_render( ch7 );

            // Sprite channels
            channel_render( ch0 );
            channel_render( ch1 );
            channel_render( ch2 );
            channel_render( ch3 );
            channel_render( ch4 );

            SDL_RenderPresent( renderer );
        }

        // yield
        SDL_Delay( 30 );
    }

    ainput_delete( ainput );
    minput_delete( minput );
    channel_delete( ch0 );
    channel_delete( ch1 );
    channel_delete( ch2 );
    channel_delete( ch3 );
    channel_delete( ch4 );
    channel_delete( ch5 );
    channel_delete( ch6 );
    channel_delete( ch7 );
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();
    return 0;
}

