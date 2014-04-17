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
 *      There are the following primitive words for input and output:
 *
 *      => KEY waits for a character typed on the keyboard and returns
 *      that character, but => KEY does not return non-character
 *      input events like function keys pressed - use => EKEY for 
 *      a more complete keyboard query.
 *      Furthermore, => KEY? returns true if a key is available for
 *      reading using => KEY (function key presses are not detected
 *      but discarded).
 *
 *      => EMIT will display the character at the current cursor
 *      position, control characters take effect depending on the
 *      system. => TYPE displays all the chars in the given string 
 *      buffer.
 *
 *      To get the current cursor position, use =>'AT-XY'.
 */

//#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "config.h"
#include "types.h"
#include "macro.h"
#include "listwords.h"
#include "session.h"
#include "terminal.h"

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

/** TIME&amp;DATE ( -- sec# min# hrs# day# month# year# ) [ANS]
 * return the broken down current time
 */
FCode (pf_time_and_date)
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
/*
    P4_INTO ("[ANS]", 0),
    P4_FXco ("AT-XY",		p4_at_x_y),
    P4_FXco ("KEY?",		p4_key_question),
    P4_FXco ("PAGE",		p4_dot_clrscr),
    P4_FXco ("EKEY",		p4_ekey),
    P4_FXco ("EKEY>CHAR",	p4_ekey_to_char),
    P4_FXco ("EKEY?",		p4_ekey_question),
    P4_FXco ("EMIT?",		p4_emit_question),
*/
    P4_FXco ("AT-XY",        pf_gotoxy),	// in terminal.c
    P4_FXco ("MS",           pf_ms),
    P4_FXco ("PAGE",         pf_clear_screen),	// in terminal.c
    P4_FXco ("RANDOM",       pf_random),
    P4_FXco ("SEED",         pf_seed),
    P4_FXco ("TIME&DATE",    pf_time_and_date),
};
P4_COUNTWORDS (facility, "Facility");
