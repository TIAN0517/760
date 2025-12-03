#include "stdafx.h"
#include <stdio.h>

// VS2015+ ?‡è???OpenSSL ?„ç›¸å®¹æ€§ä¿®å¾?
// OpenSSL ä½¿ç”¨å·²æ??¨ç? __iob_func,?€è¦æ?ä¾›æ›¿ä»?¯¦ä½?

extern "C" {
    // __iob_func ??VS2015 ä¸­å·²ç§»é™¤,?‘å€‘é?è¦æ?ä¾›æ›¿ä»?¯¦ä½?
    // è¿”å??‡å? stdin/stdout/stderr ?„é™£??
    FILE* __cdecl __iob_func(void)
    {
        // å»ºç??¨æ????ä¸¦è???
        static FILE* iob_array[3];
        iob_array[0] = stdin;
        iob_array[1] = stdout;
        iob_array[2] = stderr;
        return iob_array[0];
    }
}
