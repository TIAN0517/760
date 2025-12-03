#include "stdafx.h"

#ifndef BASE
#define BASE 65521L
#endif

#ifndef NMAX
#define NMAX 5552
#endif

#ifndef DO1
#define DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#endif

#ifndef DO2
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#endif

#ifndef DO4
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#endif

#ifndef DO8
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#endif

#ifndef DO16
#define DO16(buf)   DO8(buf,0); DO8(buf,8);
#endif

uLong adler32(uLong adler, const BYTE *buf, uInt len)
{
	uLong s1 = adler & 0xffff;
	uLong s2 = (adler >> 16) & 0xffff;

	int k;

	if (!buf) return 0L;

	while (len > 0) {
		k = len < NMAX ? len : NMAX;
		len -= k;

		while (k >= 16) {
			DO16(buf);
			buf += 16;
			k -= 16;
		}

		if (k != 0) do {
			s1 += *buf++;
			s2 += s1;
		} while (--k);

		s1 %= BASE;
		s2 %= BASE;
	}

	return (s2 << 16) | s1;
}
