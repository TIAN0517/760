#include <cstdio>

extern "C" FILE* __cdecl __acrt_iob_func(unsigned index);

extern "C" FILE* __cdecl __iob_func()
{
    return __acrt_iob_func(0);
}
