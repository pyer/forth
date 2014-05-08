/** 
 * -- The Optional Programming-Tools Word Set
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.15 $
 *     (modified $Date: 2008-09-11 01:27:20 $)
 *
 *  @description
 *      The ANS Forth defines some "Programming Tools", words to
 *      inspect the stack (=>'.S'), memory (=>'DUMP'), 
 *      compiled code (=>'SEE') and what words
 *      are defined (=>'WORDS').
 *
 *      There are also word that provide some precompiler support 
 *      and explicit acces to the =>'CS-STACK'.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>

#include "config.h"
#include "types.h"
#include "macro.h"
#include "listwords.h"
#include "thread.h"
#include "interpret.h"
#include "terminal.h"

#define DECWIDTH (sizeof (p4cell) * 5 / 2 + 1)
#define HEXWIDTH (sizeof (p4cell) * 2)

/* ----------------------------------------------------------------------- */
static void pf_prCell (p4cell n)
{
    pf_outf ("\n%*ld [%0*lX] ",
      (int) DECWIDTH, (p4cell)n,
      (int) HEXWIDTH, (unsigned long)n);
}

/* ----------------------------------------------------------------------- */
/** .S ( -- )
 *     print the stack content in vertical nice format.
 *     tries to show cell-stack and float-stack side-by-side,
 *
 *	 Depending on configuration,
 *	there are two parameter stacks: for integers and for
 *	floating point operations. If both stacks are empty, =>'.S'
 *	will display the message <code>&lt;stacks empty&gt;</code>.
 *
 *	If only the floating point stack is empty, =>'.S' displays
 *	the integer stack items  in one column, one item per line,
 *	both in hex and in decimal like this (the first item is topmost):
 12345 HEX 67890 .S
    	424080 [00067890]
         12345 [00003039] ok
 *
 *      If both stacks ar not empty, => .S displays both stacks, in two
 *	columns, one item per line
 HEX 123456.78E90 ok
 DECIMAL 123456.78E90 .S
    	   291 [00000123]          1.234568E+95
    1164414608 [45678E90] ok
 * 	Confusing example? Remember that floating point input only works
 * 	when the => BASE number is =>'DECIMAL'. The first number looks like
 * 	a floating point but it is a goodhex double integer too - the number
 * 	base is =>'HEX'. Thus it is accepted as a hex number. Second try 
 *      with a decimal base will input the floating point number.
 *
 *      If only the integer stack is empty, => .S shows two columns, but
 *      he first columns is called <tt>&lt;stack empty&gt;</tt>, and the
 *      second column is the floating point stack, topmost item first.
 */
FCode (pf_dot_s)
{
    int i;
    
    int dd = S0 - SP;
#if defined PF_WITH_FLOATING
    int fd = F0 - FP;
    if (fd == 0)
#endif
    {
        if (dd == 0)
        {   /* !fd !dd */
            /* both stacks empty */
            pf_outf ("\n%*s",
              (int)(DECWIDTH + HEXWIDTH + 4), "<stacks empty> ");
        }else{ /* !fd dd */
            /* only data stack not empty */
            for (i = 0; i < dd; i++)
            {
                FX (pf_cr);
                pf_prCell (SP[i]);
            }
        }
    }
#if defined PF_WITH_FLOATING
    else if (dd == 0) /* fd !dd */
    {
        /* only floating point stack not empty */
        pf_outf ("\n%*s%15.*G ", (int)(DECWIDTH + HEXWIDTH + 4), "<stack empty> ", (int)PRECISION, FP[0]);
        for (i = 1; i < fd; i++) {
            pf_outf ("\n%*.*G ", (int)(DECWIDTH + HEXWIDTH + 4) + 15, (int)PRECISION, FP[i]);
        }
    } else { /* fd dd */
        int bd = dd < fd ? dd : fd;
        for (i = 0; i < bd; i++) {
	    FX (pf_cr);
	    pf_prCell (SP[i]);
            pf_outf ("%15.*G ", (int)PRECISION, FP[i]);
        }
	for (; i < dd; i++) {
	    FX (pf_cr);
	    pf_prCell (SP[i]);
        }
	for (; i < fd; i++) {
            pf_outf ("\n%*.*G ", (int)(DECWIDTH + HEXWIDTH + 4) + 15, (int)PRECISION, FP[i]);
        }
    }
#endif
}

/** .STATUS ( -- ) [FTH]
 * display internal variables and memory status
 */
FCode (pf_dot_status)
{
    pf_outf ("\nDictionary space:    %7ld Bytes, in use: %7ld Bytes",(p4cell) (dictlimit - dict),(p4cell) (DP - dict));
    pf_outf ("\nStack space:         %7ld cells",  (p4cell) (S0 - PFE.stack));  /* sizeof (p4cell) */
    pf_outf ("\nReturn stack space:  %7ld cells, (not the C call stack)",(p4cell) (R0 - PFE.rstack));  /* sizeof (p4xt**) */
#if defined PF_WITH_FLOATING
    pf_outf ("\nFloating stack space:%7ld floats", (p4cell) (F0 - PFE.fstack)); /* sizeof (double) */
    pf_outf ("\nPRECISION:           %3d", (int) PRECISION);
#endif
    pf_outf ("\nmaximum number of open files:     %u",  P4_MAX_FILES);
    FX (pf_cr);
}

/************************************************************************/
/** HELP ( -- )
 *
 */
void pf_help(const char *name, int len)
{
    char buffer[256];
    int found = 0;
    FILE *fh = fopen( PF_HELP_FILE, "r" );
    if( fh != NULL ) {
        while( fgets( buffer, 255, fh ) != NULL ) {
            // if( found == 0 && strncmp(buffer,(char *)upper,len) == 0 ) {
            if( found == 0 && strncmp(buffer,(char *)name,len) == 0 ) {
                found = 1;
            }
            if( found ) {
                int buflen = strlen(buffer);
                if( buflen < 2 )
                    break;
                pf_type(buffer,buflen);
            }
        }
        fclose( fh );
    } else {
        printf( "File %s not found\n", PF_HELP_FILE );
    }
}

FCode (pf_help)
{
    const char *fn = cap_word (' ');
    char len = *fn++;
    FX (pf_cr);
    pf_help( fn, len );
}

/************************************************************************/

/** DUMP ( addr len -- )
 * show a hex-dump of the given area, if it's more than a screenful
 * it will ask using => ?CR
 *
 * You can easily cause a segmentation fault of something like that
 * by accessing memory that does not belong to the pfe-process.
 */
FCode (pf_dump)
{
    p4ucell i, j;
    p4ucell n = (p4ucell)*(SP++);
    p4char *p = (p4char*)(*(SP++));
    
    FX (pf_more);
    FX (pf_cr);
    pf_outf ("%*s ", (int)HEXWIDTH, "");
    for (j = 0; j < 16; j++)
        pf_outf ("%02X ", (unsigned)((p4ucell)(p + j) & 0x0F));
    for (j = 0; j < 16; j++)
        pf_outf ("%X", (unsigned)((p4ucell)(p + j) & 0x0F));
    for (i = 0; i < n; i += 16, p += 16)
    {
        if (pf_more_Q())
            break;
        pf_outf ("%0*lX ", (int)HEXWIDTH, (unsigned long)p);
        for (j = 0; j < 16; j++)
            pf_outf ("%02X ", p [j]);
        for (j = 0; j < 16; j++)
            pf_outf ("%c", (isspace (p [j]) ? ' ' : 
			    ! isascii (p [j]) ? '_' :
			    isprint (p [j]) ? p [j] : '.'));
    }
    FX (pf_space);
}

/* ----------------------------------------------------------------------- */
/** WORDS ( -- )
 * lists the words defined in that vocabulary.
 */
FCode (pf_words)
{
    p4_namebuf_t *nfa = LATEST;		/* NFA of most recently CREATEd header */
//# define WILD_TAB 26 /* traditional would be 20 (26*4=80), now 26*3=78 */
# define WILD_TAB 20 /* traditional would be 20 (26*4=80), now 26*3=78 */
    FX (pf_more);
    FX (pf_cr);
    while (nfa)
    {
	char c = pf_category (*name_to_cfa(nfa));
        pf_outc(c); pf_outc(' ');
        pf_dot_name(nfa);
        pf_emits (WILD_TAB - get_outs() % WILD_TAB, ' ');
        if (get_outs()+WILD_TAB > get_cols()) {
            if (pf_more_Q())
                break;
        }
        nfa = *name_to_link (nfa);
    }
    FX (pf_cr);
}

/* ----------------------------------------------------------------------- */
P4_LISTWORDS (tools) =
{
//    P4_INTO ("FORTH", 0),
    P4_FXco (".S",           pf_dot_s),
    P4_FXco (".STATUS",      pf_dot_status),
    P4_FXco ("HELP",         pf_help),
    P4_FXco ("MAN",          pf_help),
    P4_FXco ("DUMP",         pf_dump),
    P4_FXco ("WORDS",        pf_words),
};
P4_COUNTWORDS (tools, "Tools words");

