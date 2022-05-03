/* DCL_error.c */

#include "DCL.h"

void DCL_runErrorHandler(DCL_CSS *p)
{
    if (ERR_NONE != p->err)
    {
        DCL_BREAK_POINT;
        while(1);
    }
}

/* end of file */
