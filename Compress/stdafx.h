#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <tchar.h>
#include <time.h>

#if defined(_WIN32) && !defined(tzset)
#define tzset _tzset
#endif

#ifndef _ZLIB_TYPES_DEFINED
#define _ZLIB_TYPES_DEFINED
typedef unsigned char Byte;
typedef unsigned int uInt;
typedef unsigned long uLong;
typedef Byte* Bytef;
typedef uLong uLongf;
typedef void* voidpf;
#endif
