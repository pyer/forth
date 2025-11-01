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
#include "const.h"
#include "types.h"
#include "macro.h"
#include "thread.h"
#include "compiler.h"
#include "interpret.h"
#include "terminal.h"
#include "history.h"

#define DECWIDTH (sizeof (p4cell) * 5 / 2 + 1)
#define HEXWIDTH (sizeof (p4cell) * 2)

/* ----------------------------------------------------------------------- */
static void pf_prCell (p4cell n)
{
    pf_outf ("\n%*ld [0x%0*lX] ",
      (int) DECWIDTH, (p4cell)n,
      (int) HEXWIDTH, (unsigned long)n);
}

/* ----------------------------------------------------------------------- */
/** .S ( -- )
 *     print the stack content in vertical nice format.
 *     tries to show cell-stack and float-stack side-by-side,
 *
 *     Depending on configuration,
 *    there are two parameter stacks: for integers and for
 *    floating point operations. If both stacks are empty, =>'.S'
 *    will display the message <code>&lt;stacks empty&gt;</code>.
 *
 *    If only the floating point stack is empty, =>'.S' displays
 *    the integer stack items  in one column, one item per line,
 *    both in hex and in decimal like this (the first item is topmost):
 12345 HEX 67890 .S
        424080 [00067890]
         12345 [00003039] ok
 *
 *      If both stacks ar not empty, => .S displays both stacks, in two
 *    columns, one item per line
 HEX 123456.78E90 ok
 DECIMAL 123456.78E90 .S
           291 [00000123]          1.234568E+95
    1164414608 [45678E90] ok
 *     Confusing example? Remember that floating point input only works
 *     when the => BASE number is =>'DECIMAL'. The first number looks like
 *     a floating point but it is a goodhex double integer too - the number
 *     base is =>'HEX'. Thus it is accepted as a hex number. Second try
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
void pf_dot_name (const p4char *nfa)
{
    if (nfa && (P4_NAMEFLAGS(nfa) & 0x80)) {
        pf_type ((const char *)NAMEPTR(nfa), NAMELEN(nfa));
        FX (pf_space);
    }
}

void pf_dot_number(p4ucell u)
{
    char sign=' ';
    char *hold = DP + MIN_HOLD;
    *hold-- = '\0';
    *hold-- = ' ';

    if (u < 0) {
        sign = '-';
        u = -u;
    }

    do {
      udiv_t res;
      res.quot = u / BASE;
      res.rem  = u % BASE;
      u = res.quot;
      *hold-- = pf_number2digit(res.rem);
    } while (u);

    if (sign == '-' )
      *hold = sign;
    else
      hold++;
    pf_outs(hold);
}

void pf_decompile_rest (p4char* nfa, p4xt *ip)
{
    pf_dot_name (nfa);
    while (**ip != P4CODE(pf_semicolon_execution)) {
        p4char *name = cfa_to_name(*ip);
        pf_dot_name(name);
        p4_Semant *s = ((p4_Semant *)((char *)(*ip) - OFFSET_OF (p4_Semant, exec[0])));
        //pf_outf ("\nip = %p *ip = %p  s = %p  ", ip, *ip, s);
        if (s->magic == P4_SEMANT_MAGIC) {
          ip++;
          //pf_outf ("\nSEMANT_MAGIC %ld skips = %d ", s->magic, s->skips);
          switch (s->skips) {
            case P4_SKIPS_OFFSET:
              break;
            case P4_SKIPS_CELL:
              p4cell n = *(p4cell *)ip;
              pf_dot_number(n);
              ip++;
              break;
            case P4_SKIPS_STRING:
              p4char *str = (p4char *)ip;
              pf_outs(".\" ");
              pf_type ((const char *)NAMEPTR(str), NAMELEN(str));
              pf_outs("\" ");
              ip = (p4xt *)(str + pf_aligned(*str));
              break;
            case P4_SKIPS_TO_TOKEN:
              break;
#if defined PF_WITH_FLOATING
            case P4_SKIPS_FLOAT:
              TYPEOF_FCELL *f = (TYPEOF_FCELL *)ip;
              pf_outf ("%.*f ", (int) PRECISION, *f);
              f++;
              ip = (p4xt *)f;
              break;
#endif
            case P4_SKIPS_NOTHING:
            default:
              ip++;
              break;
          }
        } else {
          ip++;
        }
        if (**ip == P4CODE(pf_does_execution)) {
          pf_outs("DOES> ");
          ip++;
        }
    }
}

/************************************************************************/
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
    p4char* nfa = pf_tick_nfa();
    p4xt    xt  = LINK_TO_CFA(name_to_link(nfa));
    p4xt* rest = (p4xt*) P4_TO_BODY(xt);

    FX (pf_cr);

    if (*xt == P4CODE(pf_colon_RT)) {
        pf_outs(": ");
        pf_decompile_rest(nfa, rest);
        pf_outs("; ");
    } else if (*xt == P4CODE(pf_create_RT)) {
        pf_outs("CREATE ");
        pf_dot_name (nfa);
    } else if (*xt == P4CODE(pf_builds_RT)) {
        pf_outs("<BUILDS ");
        pf_dot_name (nfa);
    } else if (*xt == P4CODE(pf_does_RT)) {
        pf_outs("<BUILDS ");
        pf_dot_name (nfa);
        pf_outs("DOES> ??? ");
        pf_outf("DOES> %p ", xt);
        //pf_decompile_rest(NULL, rest);
        pf_outs(";");
    } else if (*xt == P4CODE(pf_constant_RT)) {
        pf_outf("%d CONSTANT ", (p4cell)*rest);
        pf_dot_name (nfa);
    } else if (*xt == P4CODE(pf_value_RT)) {
        pf_outs("VALUE ");
        pf_dot_name (nfa);
    } else if (*xt == P4CODE(pf_variable_RT)) {
        pf_outs("VARIABLE ");
        pf_dot_name (nfa);
#if defined PF_WITH_FLOATING
    } else if (*xt == P4CODE(p4_f_constant_RT)) {
        double fval = *(double *)(rest);
        pf_outf ("%.*f FCONSTANT ", (int) PRECISION, fval);
        pf_dot_name (nfa);
    } else if (*xt == P4CODE(p4_f_variable_RT)) {
        pf_outs("FVARIABLE ");
        pf_dot_name (nfa);
#endif
    } else {
        pf_dot_name (nfa);
        pf_outs("is a primitive. ");
    }

    if (P4_NAMEFLAGS(nfa) & P4xIMMEDIATE)
            pf_outs ("IMMEDIATE ");
//    if (P4_NAMEFLAGS(nfa) & P4xISxRUNTIME)
//            pf_outs ("RUNTIME ");
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
    p4char *nfa = LATEST;        /* NFA of most recently CREATEd header */
    FX (pf_cr);
    while (nfa) {
        pf_dot_name(nfa);
        nfa = *name_to_link (nfa);
        if (pf_more_Q())
            break;
    }
}

/* ----------------------------------------------------------------------- */
/** HISTORY ( -- )
 */
FCode (pf_history)
{
    register HIST_ENTRY **the_list = history_list();
    register int i = 0;

    FX (pf_more);
    FX (pf_cr);
    while (the_list[i])
    {
        printf("%d: ",i);
        pf_outs( the_list[i]->line);
        i++;
        if (pf_more_Q())
            break;
    }
}

/* ----------------------------------------------------------------------- */
P4_LISTWORDS (tools) =
{
    P4_FXco (".S",           pf_dot_s),
    P4_FXco (".STATUS",      pf_dot_status),
    P4_FXco ("SEE",          pf_see),
    P4_FXco ("DUMP",         pf_dump),
    P4_FXco ("WORDS",        pf_words),
    P4_FXco ("HISTORY",      pf_history),
};
P4_COUNTWORDS (tools, "Tools words");

