/** 
 * FACILITY --- The Optional Facility Word Set
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.3 $
 *     (modified $Date: 2008-04-20 04:46:29 $)
 *
 *  @description
 */

//#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "config.h"
#include "types.h"
#include "macro.h"
#include "listwords.h"
#include "thread.h"

/* -------------------------------------------------------------- */
/** MS ( milliseconds# -- ) [ANS]
 * wait at least the specified milliseconds
 * (suspend the forth tasklet)
 */
FCode (pf_ms)
{
    unsigned int ms = *SP++;
    usleep (ms * 1000);
}

/** RANDOM ( -- x )
 *
 */
FCode (pf_random)
{
    *--SP = (p4cell)random();
}

/** SEED ( x -- )
 *
 */
FCode (pf_seed)
{
    srandom((unsigned int) *SP++);
}

/** NOW ( -- sec# min# hrs# day# month# year# ) [ANS]
 * return the broken down current time
 */
FCode (pf_now)
{
    time_t t;
    struct tm *tm;
    
    time (&t);
    tm = localtime (&t);
    SP -= 6;
    SP[5] = tm->tm_sec;
    SP[4] = tm->tm_min;
    SP[3] = tm->tm_hour;
    SP[2] = tm->tm_mday;
    SP[1] = tm->tm_mon + 1;
    SP[0] = tm->tm_year + 1900;
}

P4_LISTWORDS (facility) =
{
    P4_FXco ("MS",     pf_ms),
    P4_FXco ("RANDOM", pf_random),
    P4_FXco ("SEED",   pf_seed),
    P4_FXco ("NOW",    pf_now),
};
P4_COUNTWORDS (facility, "Facility");
