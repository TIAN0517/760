#include "stdafx.h"



uLong crc32_for_byte(uLong r) {
  for(int j = 0; j < 8; ++j)
    r = (r & 1? 0: (uLong)0xEDB88320L) ^ r >> 1;
  return r ^ (uLong)0xFF000000L;
}

uLong crc32(uLong crc, const BYTE *buf, uInt len)
{
	uInt i;
	static uLong crc_tables[0x100];

	if(!*crc_tables)
		for(i = 0; i < 0x100; ++i)
			crc_tables[i] = crc32_for_byte(i);

	for(i = 0; i < len; ++i)
		crc = crc_tables[(BYTE)crc ^ ((BYTE*)buf)[i]] ^ crc >> 8;

	return crc;
}
