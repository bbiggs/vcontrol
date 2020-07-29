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

#ifndef PNGINPUT_H_INCLUDED
#define PNGINPUT_H_INCLUDED

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Example usage:
 *
 * pnginput_t *pngin = pnginput_new( "myimage.png" );
 *
 * for( i = 0; i < pnginput_get_height( pngin ); i++ ) {
 *     uint8_t *scanline = pnginput_get_scanline( pngin, i );
 *
 *     for( x = 0; x < pnginput_get_width( pngin ); x++ ) {
 *         if( pnginput_has_alpha( pngin ) ) {
 *             r = scanline[ (x * 4) + 0 ];
 *             g = scanline[ (x * 4) + 1 ];
 *             b = scanline[ (x * 4) + 2 ];
 *             a = scanline[ (x * 4) + 3 ];
 *         } else {
 *             r = scanline[ (x * 3) + 0 ];
 *             g = scanline[ (x * 3) + 1 ];
 *             b = scanline[ (x * 3) + 2 ];
 *         }
 *     }
 * }
 *
 * pnginput_delete( pngin );
 */


typedef struct pnginput_s pnginput_t;

/**
 * Opens the filename as a png file.  Returns 0 on error.
 */
pnginput_t *pnginput_new( const char *filename );

/**
 * Closes the png file.
 */
void pnginput_delete( pnginput_t *pnginput );

/**
 * Returns the width of the png file.
 */
unsigned int pnginput_get_width( pnginput_t *pnginput );

/**
 * Returns the height of the png file.
 */
unsigned int pnginput_get_height( pnginput_t *pnginput );

/**
 * Returns a pointer to the given scanline from 0 to height-1.
 */
uint8_t *pnginput_get_scanline( pnginput_t *pnginput, int num );

/**
 * Returns true if this png file has an alpha channel.
 */
int pnginput_has_alpha( pnginput_t *pnginput );

#ifdef __cplusplus
};
#endif
#endif /* PNGINPUT_H_INCLUDED */
