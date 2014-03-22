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
#include "session.h"
#include "interpret.h"
#include "dictionary.h"
#include "terminal.h"

#define DECWIDTH (sizeof (p4cell) * 5 / 2 + 1)
#define HEXWIDTH (sizeof (p4cell) * 2)

#ifndef USE_END_CODE
#if defined PFE_SBR_COMPILE_PROC && defined PFE_SBR_COMPILE_EXIT
#define USE_END_CODE 1
#else
#define USE_END_CODE 0
#endif
#endif

void p4_decompile (p4char* nfa, p4xt xt);
char p4_category (p4code p);
/* ----------------------------------------------------------------------- */
static void pf_prCell (p4cell n)
{
    pf_outf ("\n%*ld [%0*lX] ",
      (int) DECWIDTH, (p4celll)n,
      (int) HEXWIDTH, (p4ucelll)n);
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
    
    int dd = PFE.s0 - PFE.sp;
//printf("dd=%d\n",dd);
#  ifndef P4_NO_FP
    int fd = PFE.f0 - PFE.fp;
//printf("fd=%d\n",fd);
    
    if (fd == 0)
#  endif
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
                FCode (pf_cr);
                pf_prCell (PFE.sp[i]);
            }
        }
    }
# ifndef P4_NO_FP
    else if (dd == 0) /* fd !dd */
    {
        /* only floating point stack not empty */
        pf_outf ("\n%*s%15.7G ",
          (int)(DECWIDTH + HEXWIDTH + 4), "<stack empty> ", PFE.fp[0]);
        for (i = 1; i < fd; i++)
            pf_outf ("\n%*.7G ",
              (int)(DECWIDTH + HEXWIDTH + 4) + 15, PFE.fp[i]);
    }else{ /* fd dd */
        int bd = dd < fd ? dd : fd;
        for (i = 0; i < bd; i++)
        {
	    FCode (pf_cr);
	    pf_prCell (PFE.sp[i]);
	    pf_outf ("%15.7G ", PFE.fp[i]);
        }
	for (; i < dd; i++)
        {
	    FCode (pf_cr);
	    pf_prCell (PFE.sp[i]);
        }
	for (; i < fd; i++)
            pf_outf ("\n%*.7G ",
              (int)(DECWIDTH + HEXWIDTH + 4) + 15, PFE.fp[i]);
    }
# endif
}

/** .MEMORY ( -- ) [FTH]
 */
FCode (pf_dot_memory)
{
    pf_outf ("\nDictionary space:    %7ld Bytes, in use: %7ld Bytes",(p4celll) (PFE.dictlimit - PFE.dict),(p4celll) (PFE.dp - PFE.dict));
    pf_outf ("\nStack space:         %7ld cells",  (p4celll) (PFE.s0 - PFE.stack));  /* sizeof (p4cell) */
    pf_outf ("\nFloating stack space:%7ld floats", (p4celll) (PFE.f0 - PFE.fstack)); /* sizeof (double) */
    pf_outf ("\nReturn stack space:  %7ld cells, (not the C call stack)",(p4celll) (PFE.r0 - PFE.rstack));  /* sizeof (p4xt**) */
    pf_cr_();
}

/** .STATUS ( -- ) [FTH]
 * display internal variables
 : .STATUS .SEARCHPATHS .DICTVARS .REGSUSED ;
 */
FCode (pf_dot_status)
{
    pf_outf ("\nmaximum number of open files:     %u",  P4_MAX_FILES);
    pf_outf ("\nmaximum wordlists in search order %u", ORDER_LEN);

#  define flag(X) ((X) ? "ON " : "OFF")
#  ifdef P4_REGTH
    pf_outs ("           REGTH="P4_REGTH);
#  else
    pf_outs ("           (static regTH pointer)");
#  endif
#  ifdef P4_REGSP
    pf_outs ("           REGSP="P4_REGSP);
#  endif
    pf_outf ("\nRESET-ORDER-IS %s", flag (RESET_ORDER));
#  ifdef P4_REGIP
    pf_outs ("           REGIP="P4_REGIP);
#  elif defined PFE_SBR_CALL_THREADING
    pf_outs ("           REGIP (hardware)");
#  endif
    pf_outf ("\nREDEFINED-MSG  %s", flag (REDEFINED_MSG));
#  ifdef P4_REGRP
    pf_outs ("           REGRP="P4_REGRP);
#  elif defined PFE_SBR_CALL_THREADING
    pf_outs ("           REGRP (hardware)");
#  endif
#  if defined PFE_REGFP
    pf_outs ("           REGFP="P4_REGFP);
#  endif
#  undef flag
    pf_outf ("\nPRECISION     %3d", (int) PRECISION);
#  ifdef P4_REGW
    pf_outs ("            REGW="P4_REGW" (threading)");
#  elif defined PFE_SBR_CALL_ARG_THREADING
    pf_outs ("           (sbr-call-arg-threading)");
#  elif defined PFE_SBR_CALL_THREADING
    pf_outs ("           (sbr-call-threading)");
#  elif defined PFE_CALL_THREADING
    pf_outs ("           (call-threading)");
#  else
    pf_outs ("           (traditional-threading)");
#  endif
    pf_cr_();
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
    p4ucell n = (p4ucell)*(PFE.sp++);
    p4char *p = (p4char*)(*(PFE.sp++));
    
    FCode (pf_cr);
    FCode (pf_more);
    pf_outf ("%*s ", (int)HEXWIDTH, "");
    for (j = 0; j < 16; j++)
        pf_outf ("%02X ", (unsigned)((p4ucell)(p + j) & 0x0F));
    for (j = 0; j < 16; j++)
        pf_outf ("%X", (unsigned)((p4ucell)(p + j) & 0x0F));
    for (i = 0; i < n; i += 16, p += 16)
    {
        if (pf_more_Q())
            break;
        pf_outf ("%0*lX ", (int)HEXWIDTH, (p4ucelll)p);
        for (j = 0; j < 16; j++)
            pf_outf ("%02X ", p [j]);
        for (j = 0; j < 16; j++)
            pf_outf ("%c", (isspace (p [j]) ? ' ' : 
			    ! isascii (p [j]) ? '_' :
			    isprint (p [j]) ? p [j] : '.'));
    }
    FCode (pf_space);
}

/** SEE ( "word" -- )
 *  decompile word - tries to show it in re-compilable form.
 *
 *  => (SEE) tries to display the word as a reasonable indented
 *  source text. If you defined your own control structures or
 *  use extended control-flow patterns, the indentation may be
 *  suboptimal.
 simulate:
   : SEE  [COMPILE] ' (SEE) ; 
 */

FCode (pf_see)
{
    p4char *nfa = pf_tick_nfa();
    p4_decompile (nfa, pf_name_from(nfa));
}

/* ----------------------------------------------------------------------- */
/** WORDS ( -- )
 * uses CONTEXT and lists the words defined in that vocabulary.
 * usually the vocabulary to list is named directly in before.
 example:
    FORTH WORDS  or  LOADED WORDS
 */
FCode (pf_words)
{
    Wordl *wl = CONTEXT [0] ? CONTEXT [0] : ONLY;
    p4char **t;
    /* Wordl wcopy = *wl;          // clobbered while following it */
    Wordl wcopy; memcpy (&wcopy, wl, sizeof(wcopy));

# ifndef WILD_TAB
//# define WILD_TAB 26 /* traditional would be 20 (26*4=80), now 26*3=78 */
# define WILD_TAB 20 /* traditional would be 20 (26*4=80), now 26*3=78 */
# endif
    
    FCode (pf_cr);
    FCode (pf_more);
    for (t = p4_topmost (&wcopy); *t; t = p4_topmost (&wcopy))
    {
        p4char *w = *t;
        p4char **s = p4_name_to_link (w);
	char c = p4_category (*P4_TO_CODE(P4_LINK_FROM (s)));
//        w++;
//        int l = strlen((char *)w);
        int l = *w++;
/*
        if (get_outs()+WILD_TAB - get_outs()%WILD_TAB + 2 + l > get_cols() ||
            get_outs()+WILD_TAB - get_outs()%WILD_TAB + WILD_TAB*2/3 > get_cols())
        {
            if (pf_more_Q())
                break;
        }
*/
        //pf_outf ("%c %.*s ", c, l, w);
        pf_outc(c); pf_outc(' ');
        pf_type((char *)w,l);
        pf_tab (WILD_TAB);
        pf_cr_();
        *t = *s;
    }
}

/* ----------------------------------------------------------------------- */
P4_LISTWORDS (tools) =
{
//    P4_INTO ("FORTH", 0),
    P4_FXco (".S",           pf_dot_s),
    P4_FXco (".MEMORY",      pf_dot_memory),
    P4_FXco (".STATUS",      pf_dot_status),
    P4_FXco ("DUMP",         pf_dump),
    P4_FXco ("SEE",          pf_see),
    P4_FXco ("WORDS",        pf_words),
};
P4_COUNTWORDS (tools, "Tools words");

