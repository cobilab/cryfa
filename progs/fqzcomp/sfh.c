/*
 * An reimplementation of Super Fast Hash by Paul Hsieh
 * 
 * See: http://www.azillionmonkeys.com/qed/hash.html
 */

#include <stdio.h>
#include <stdint.h>

#include "sfh.h"

uint32_t sfhash(const unsigned char *d, int len) {
    uint32_t h = len, t;
    int i;

    /* Primary hashing */
    len -= 3;

    for (i = 0; i < len; i += 4, d += 4) {
	t  = (h += d[0] + (d[1]<<8)) ^ ((d[2]<<11) + (d[3]<<19));
	h = (t ^ (h << 16)) + ((t ^ (h << 16)) >> 11);
    }

    /* Remainder for when len is not a multiple of 4 */
    switch (len&3) {
    case 0:
	h += d[0] + (d[1]<<8);
	h ^= h<<16;
	h ^= ((signed char)d[2]) << 18;
	h += h >> 11;
	break;
    case 3:
	h += d[0] + (d[1]<<8);
	h ^= h << 11;
	h += h >> 17;
	break;
    case 2:
	h += (signed char)d[0];
	h ^= h << 10;
	h += h >> 1;
	break;
    }

    /* Final bit avalanching */
    h ^= h <<  3;
    h += h >>  5;
    h ^= h <<  4;
    h += h >> 17;
    h ^= h << 25;
    h += h >>  6;

    return h;
}
