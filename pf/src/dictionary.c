/** 
 *  Implements dictionary and wordlists.
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.9 $
 *     (modified $Date: 2008-05-03 14:20:20 $)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stddef.h> /*offsetof*/
#include <setjmp.h>

#include "config.h"
#include "types.h"
#include "const.h"
#include "macro.h"
#include "listwords.h"
#include "session.h"

#include "compiler.h"
#include "dictionary.h"
#include "exception.h"
#include "interpret.h"
#include "terminal.h"

/* -------------------------------------------------------------- */

#define ___ {
#define ____ }

/* -------------------------------------------------------------- */
FCode (pf_exception_string);
FCode_RT (pf_defer_RT);
/* -------------------------------------------------------------- */

/*
 * A vocabulary is organized as a mixture between hash-table and
 * linked list. (This is a practice you can observe in several
 * systems.) It works like this: Given a name, first a hash-code is
 * generated. This hash-code selects one of several linked lists
 * called threads. The hooks to these threads are stored in a table.
 *
 * The body of a WORDLIST is essentially such a table of pointers to
 * threads, while in FIG-Forth it was just a pointer to the one and
 * only linked list a VOCABULARY consists of in FIG-Forth.
 */
_export int
p4_wl_hash (const p4_char_t *s, int l)
/* s string, l length of string, returns hash-code for that name */
{
    register p4_char_t c = *s;

    while(--l > 0)
    { c += *s++; c ^= l; }
    return c & (THREADS - 1);
}

/* ------------------------------ 
 */
/* --------------------------------
 * word list and forget 
 */

/** 
 * create a word list in the dictionary 
 */
p4_Wordl * p4_make_wordlist (p4char* nfa)
{
    p4_Wordl *w = (Wordl *) DP; /* allocate word list in HERE */
    P4_INC (DP, Wordl);
    w->link = nfa;
    LATEST = nfa;
    return w;
}

/* ---------------------------
 * create a header 
 */

/* writes counted string into dictionary, returns address */
p4_charbuf_t* p4_string_comma (const p4_char_t* s, int len)
{
    p4char *p = DP;
    
    if (len >= (1 << CHAR_BIT))
        p4_throw (P4_ON_ARG_TYPE);
    *DP++ = len;                /* store count byte */
    while (--len >= 0)          /* store string */
        *DP++ = *s++;
    FX (pf_align);
    return p;
}


/* ------------------------------------------------------------------- */
p4_Semant * p4_to_semant (p4xt xt)
{
   /* I don't like this either. :-) */
# define TO_SEMANT(XT,ELEMENT) \
    ((p4_Semant *)((char *)XT - OFFSET_OF (p4_Semant, ELEMENT)))
    p4_Semant *s;

    s = TO_SEMANT (xt, exec[0]);
    if (s->magic == P4_SEMANT_MAGIC)
        return s;
    s = TO_SEMANT (xt, exec[1]);
    if (s->magic == P4_SEMANT_MAGIC)
        return s;
    return NULL;
# undef TO_SEMANT
}

p4char** pf_name_to_link (const p4char* p)
{
    return (p4char **) pf_aligned ((p4cell) (NAMEPTR(p) + NAMELEN(p)) );
}

p4char * pf_to_name (p4xt xt)
{
    /* cfa to lfa */
    p4_Semant *s = p4_to_semant (xt);
    p4_namebuf_t ** cfa = (s ? pf_name_to_link (s->name) : (p4_namebuf_t**)( xt - 1 )); 
    /* cfa to lfa
     * scan backward for count byte preceeding name of definition
     * returns pointer to count byte of name field or NULL
     */
    p4_char_t * p = (p4_char_t *) cfa;
    unsigned n;

#   define NAME_ALIGN_WIDTH sizeof(p4cell) /* one or two byte */
  /* Skip possible alignment padding: (and ZNAME end-of-string) */
    for (n = 0; *--p == '\0'; n++)
         if (n > NAME_ALIGN_WIDTH)
            return NULL;

  /* Scan for count byte. Note: not reliable even that limits are used. */
    for (n = 0; n < (NAME_SIZE_MAX + NAME_ALIGN_WIDTH); n++, p--)
    {
        /* traditional: search for CHAR of name-area with a hi-bit set
         * and assume that it is the flags/count field for the NAME */
        if ((P4_NAMEFLAGS(p) & 0x80) && ((unsigned)NAMELEN(p) == n))
            return p;
        if (! isprint(*p))
            return NULL;
    }
    return NULL;
}

/** >NAME ( cfa -- nfa )
 * converts a pointer to the code-field (CFA) to point
 * then to the corresponding name-field (NFA)
 implementation-specific simulation:
   : >NAME  >LINK L>NAME ;
 */
FCode (pf_to_name)
{
    *SP = (p4cell) pf_to_name ((p4xt) *SP);
}

/* ------------------------------------------------------------------- */
/* name> ( nfa* -- xt* ) 
 * it has one special trick in that it can see a SYNONYM
 * runtime and dereference it immediately. Thus only the 
 * target is being compiled/executed. If you need to know
 * the actual SYNONYM DEFER then you must use the sequence
 * N>LINK LINK> to get to the execution token of a word.
 */
p4xt pf_name_from (const p4_namebuf_t *p)
{
    p4xt xt = LINK_TO_CFA (pf_name_to_link (p));
    return xt;
}


/* ------------------------------------------------------------------------ */
/**
 * make a new dictionary entry in the word list identified by wid 
 *                   ( TODO: delete the externs in other code portions)
 * This function is really ifdef'd a lot because every implementation
 * needs to be (a) fast because it is used heavily when loading a forth
 * script and (b) robust to bad names like non-ascii characters and (c)
 * each variant has restrictions on header field alignments.
 * 
 */
p4_namebuf_t * pf_create_header (const p4char *name, int len)
{
    p4_namebuf_t * ptr = LATEST;
    /* move exception handling to the end of this word - esp. nametoolong */
    if (len == 0)
        p4_throw (P4_ON_ZERO_NAME);
    
#   define CHAR_SIZE_MAX      ((1 << CHAR_BIT)-1)
    if (len > NAME_SIZE_MAX || len > CHAR_SIZE_MAX)
    {
	    pf_outf("\nERROR: name too long '%.*s'", len, name);
	    p4_throw (P4_ON_NAME_TOO_LONG);
    }

    if (redefined_msg && search_thread (name, len, CURRENT->link, CURRENT ))
        pf_outf ("\n\"%.*s\" is redefined ", len, name);

    /* and now, do the p4_string_comma ... */
# if defined PFE_WITH_FFA
    /* for the FFA style we have to insert a flag byte before the 
     * string that might be HERE via a WORD call. However that makes
     * the string to move UP usually - so we have to compute the overall 
     * size of the namefield first and its gaps, then move it */ 
    DP += 2; DP += len; FX (pf_align); 
    memmove (DP-len, name, len); /* i.e. #define NFA2LFA(p) (p+1+*p) */
    ptr = DP-len -1;      /* point to count-byte before the name */
    *ptr = len;           /* set the count-byte */
    ptr[-1] = '\x80';     /* set the flag-byte before the count-byte */
# else
    /* traditional way - avoid copying if using WORD. Just look for the
     * only if() in this code which will skip over the memcpy() call if
     * WORD $HEADER, was called. At the same time we do not look for any
     * overlaps - when memcpy runs lower-to-upper address then this is
     * okay with strings shortened at HERE - but there *are* rare cases 
     * that this could fail. That's the responsibility of the user code
     * to avoid this by copying into a scratch pad first. Easy I'd say.
     */
    ptr = DP++;
    if (name != DP) memcpy(DP, name, len);
    *ptr = len;
    *ptr |= '\x80'; 
    DP += len; FX (pf_align); 
# endif

    //FX_PCOMMA (wid->link); /* create the link field... */
    FX_PCOMMA (ptr); /* create the link field... */
    //wid->thread[hc] = LAST;
//    wid->link = LAST;
    return ptr;
}

/* -------------------------------------------------------------- */
p4char* pf_header_in (p4_Wordl* wid)
{
//printf("PFE.word.len=%d\n",PFE.word.len);
    /* quick path for wordset-loader: */
    if (PFE.word.len == -1) {
      PFE.word.len = strlen ((char*) PFE.word.ptr);
      TO_IN = 0;
    } else {
      pf_skip_spaces();
      pf_parse_word(' ');
    }
    *DP = 0; /* PARSE-WORD-NOHERE */
    return p4_header_comma (PFE.word.ptr, PFE.word.len, wid);
}

/* ------------------------------------------------------------------------ */

#if 0
void oldp4_preload_only (void)
{
    auto p4_Wordl only;                   /* scratch ONLY word list */
    
    DP = (p4char *) & PFE.dict[1];
  
    /* Load the ONLY word list to the scratch ONLY: */
    memset (&only, 0, sizeof only);
    /* # only.flag |= WORDL_NOHASH; */
    p4_header_comma (p4_lit_ONLY, 4, &only ); FX_RUNTIME1_RT(p4_only);
    ONLY = p4_make_wordlist (LAST);
    /* # ONLY->flag |= WORDL_NOHASH; */
    memcpy( ONLY->thread, only.thread, sizeof(ONLY->thread) ); 
    /* i.e. copy threads from scratch ONLY to real ONLY*/
    CURRENT = ONLY;

    /* FORTH -> [ANS] -> ONLY */
    p4_header_comma (p4_lit_FORTH, 5, ONLY); FX_RUNTIME1_RT(p4_forth);
    PFE.forth_wl = p4_make_wordlist (LAST); 
    p4_header_comma (p4_lit_xANSx, 5, ONLY); FX_RUNTIME1(pf_vocabulary);
    P4_NAMEFLAGS(p4_LAST) |= P4xIMMEDIATE;
    PFE.forth_wl->also = p4_make_wordlist (LAST);
    PFE.forth_wl->also->also = ONLY;

    /* destroyers :: LOADED */
    p4_header_comma (p4_lit_LOADED, 6, ONLY); FX_RUNTIME1(pf_vocabulary);
}
#endif

void p4_preload_only (void)
{
    auto p4_Wordl only;                   /* scratch ONLY word list */
    memset (&only, 0, sizeof only);

    DP = (p4char *) & PFE.dict[1];
    p4_header_comma ("FORTH", 5, &only);
    CURRENT = p4_make_wordlist (LATEST); 
    P4_NAMEFLAGS(LATEST) |= P4xIMMEDIATE;

//    LATEST = pf_create_header ("FORTH", 5);
//    CURRENT = p4_make_wordlist (LATEST); 
//    P4_NAMEFLAGS(LATEST) |= P4xIMMEDIATE;

//    CURRENT = (Wordl *) DP; /* allocate word list in HERE */
//#define P4_INC(P,T)	((*(T **)&(P))++)
//    P4_INC (DP, CURRENT);
//    ((*(CURRENT **)&(DP))++);
//    CURRENT = (Wordl *) *DP;
//    CURRENT->link = LATEST;
}

/* ------------------------------------------------------------------- */
