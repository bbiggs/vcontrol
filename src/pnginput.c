/**
 * Copyright (C) 2002, 2003 Billy Biggs <vektor@dumbterm.net>.
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
#include <errno.h>
#include <string.h>
#include <png.h>
#include "pnginput.h"

struct pnginput_s
{
    FILE *f;
    png_structp png_ptr;
    png_infop info_ptr;
    int has_alpha;
    int channels;
    uint8_t *inscanline;
};

pnginput_t *pnginput_new( const char *filename )
{
    pnginput_t *pnginput = malloc( sizeof( pnginput_t ) );
    png_uint_32 width;
    //, height;
    int colour_type, channels;
    // int bit_depth;
    // int rowbytes;
    double gamma;

    if( !pnginput ) return 0;

    pnginput->png_ptr = 0;
    pnginput->info_ptr = 0;

    pnginput->f = fopen( filename, "rb" );
    if( !pnginput->f ) {
        fprintf( stderr, "pnginput: Cannot open %s: %s\n",
                 filename, strerror( errno ) );
        pnginput_delete( pnginput );
        return 0;
    }

    pnginput->png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, 0, 0, 0 );
    if( !pnginput->png_ptr ) {
        fprintf( stderr, "pnginput: Cannot open PNG write struct.\n" );
        pnginput_delete( pnginput );
        return 0;
    }

    pnginput->info_ptr = png_create_info_struct( pnginput->png_ptr );
    if( !pnginput->info_ptr ) {
        png_destroy_read_struct( &(pnginput->png_ptr), 0, 0 );
        pnginput->png_ptr = 0;
        fprintf( stderr, "pnginput: Cannot open PNG info struct.\n" );
        pnginput_delete( pnginput );
        return 0;
    }

    png_init_io( pnginput->png_ptr, pnginput->f );

    /* So paletted pngs work... Need to detect about alpha still though.. */
    png_set_expand( pnginput->png_ptr );

    png_read_png( pnginput->png_ptr, pnginput->info_ptr,
                  PNG_TRANSFORM_IDENTITY, 0 );

    width = png_get_image_width( pnginput->png_ptr, pnginput->info_ptr );
    // height = png_get_image_height( pnginput->png_ptr, pnginput->info_ptr );
    // bit_depth = png_get_bit_depth( pnginput->png_ptr, pnginput->info_ptr );
    colour_type = png_get_color_type( pnginput->png_ptr, pnginput->info_ptr );
    channels = png_get_channels( pnginput->png_ptr, pnginput->info_ptr );
    // rowbytes = png_get_rowbytes( pnginput->png_ptr, pnginput->info_ptr );

    png_get_gAMA( pnginput->png_ptr, pnginput->info_ptr, &gamma );

    if( colour_type == PNG_COLOR_TYPE_GRAY_ALPHA ||
        colour_type == PNG_COLOR_TYPE_RGB_ALPHA ) {
        pnginput->has_alpha = 1;
    } else {
        pnginput->has_alpha = 0;
    }

    pnginput->channels = channels;
    if( channels == 1 ) {
        pnginput->inscanline = malloc( width * 3 );
    }

    return pnginput;
}

void pnginput_delete( pnginput_t *pnginput )
{
    if( pnginput->png_ptr && pnginput->info_ptr ) {
        png_destroy_read_struct( &(pnginput->png_ptr), &(pnginput->info_ptr), 0 );
    }
    if( pnginput->f ) {
        fclose( pnginput->f );
    }
    free( pnginput );
}

uint8_t *pnginput_get_scanline( pnginput_t *pnginput, int num )
{
    uint8_t *in = png_get_rows( pnginput->png_ptr, pnginput->info_ptr )[ num ];
    if( pnginput->channels == 1 ) {
        int w = pnginput_get_width( pnginput );
        uint8_t *cur = pnginput->inscanline;

        while( w-- ) {
            *cur++ = *in;
            *cur++ = *in;
            *cur++ = *in;
            in++;
        }
        return pnginput->inscanline;
    } else {
        return in;
    }
}

unsigned int pnginput_get_width( pnginput_t *pnginput )
{
    return png_get_image_width( pnginput->png_ptr, pnginput->info_ptr );
}

unsigned int pnginput_get_height( pnginput_t *pnginput )
{
    return png_get_image_height( pnginput->png_ptr, pnginput->info_ptr );
}

int pnginput_has_alpha( pnginput_t *pnginput )
{
    return pnginput->has_alpha;
}

