#ifndef _SFH_H_
#define _SFH_H_

#include <stdint.h>

/* An reimplementation of Super Fast Hash by Paul Hsieh */
uint32_t sfhash(const unsigned char *d, int len);

#endif /* _SFH_H_ */
