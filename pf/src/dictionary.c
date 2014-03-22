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

/*
 * If we want to traverse a WORDLIST in it's entirety, we must follow
 * all threads simultaneously. The following definition eases this by
 * locating the thread with the hook pointing to the highest memory
 * location, assuming that this thread contains the latest definition
 * entered in the given WORDLIST. For usage refer to the definition of
 * WORDS.
 *
 * When following a wordlist using topmost, a copy of the word list
 * must be made. Everytime the topmost item was processed it must be
 * replaced by its successor in the linked list.
 */

/* find the thread with the latest word in the given word list */
_export p4char **
p4_topmost (p4_Wordl *w)

{
    int n = THREADS;
    p4char **p, **s = w->thread;

    for (p = s++; --n; s++)
        if (*s > *p)
            p = s;
    return p;
}

/** LATEST ( -- nfa )
 * return the NFA of the latest definition in the
 * => CURRENT vocabulary
 */
FCode (pf_latest)			
{
    *--SP = (p4cell) *p4_topmost (CURRENT);
}

/* ------------------------------ 
 */
void pf_dot_name (const p4_namebuf_t *nfa)
{
    if (! nfa || ! (P4_NAMEFLAGS(nfa) & 0x80)) {
        pf_outs ("<?""?""?> ");  /* avoid C preprocessor trigraph */
    } else {
        pf_type ((const char *)nfa + 1, *nfa);
        FX (pf_space);
    }
}

/* Search order extension words ============================================ */

/** ALSO ( -- )
 * a => DUP on the search => ORDER - each named vocabulary
 * replaces the topmost => ORDER vocabulary. Using => ALSO
 * will make it fixed to the search-order. (but it is 
 * not nailed in trap-conditions as if using => DEFAULT-ORDER )
 order:   vocn ... voc2 voc1 -- vocn ... voc2 voc1 voc1
 */
FCode (pf_also)
{
  int i;

  if (CONTEXT[PFE_set.wordlists - 1])
      p4_throw (P4_ON_SEARCH_OVER);
  for (i = PFE_set.wordlists; --i > 0;)
      CONTEXT[i] = CONTEXT[i - 1];
}

/** GET-ORDER ( -- vocn ... voc1 n )
 * get the current search order onto the stack, see => SET-ORDER
 */
FCode (pf_get_order)
{
    Wordl **p;
    p4cell n = 0;

    for (p = &CONTEXT[PFE_set.wordlists]; --p >= CONTEXT;)
        if (*p) { 
            *--SP = (p4cell)(*p);
            n++;
    }
    *--SP = (p4cell)(n);
}

/** ORDER ( -- )
 * show the current search-order, followed by 
 * the => CURRENT => DEFINITIONS vocabulary 
 * and the => ONLY base vocabulary
 */
FCode (pf_order)
{
    int i;

    FX (pf_get_order);
    for (i = *SP++; --i >= 0;)
    {
        Wordl *w = (Wordl *) *SP++;
        pf_dot_name (w->nfa);
    }
    pf_dot_name (CURRENT->nfa);
    pf_outs ("DEFINITIONS           ");
    pf_dot_name (ONLY->nfa);
}

/* --------------------------------
 * word list and forget 
 */

/** 
 * create a word list in the dictionary 
 */
_export p4_Wordl *
p4_make_wordlist (p4char* nfa)
{
    p4_Wordl *w = (Wordl *) DP; /* allocate word list in HERE */
    P4_INC (DP, Wordl);
    
    memset( w->thread, 0, sizeof(w->thread) );   /* initialize all threads to empty */
    w->nfa = nfa;               /* set name for the wordlist (if any) */
    w->flag = WORDL_FLAG;       /* init flags from global flags */
    w->prev = VOC_LINK;         /* chain word list in VOC-LINK */
    VOC_LINK = w;
    w->id = w->prev ? (w->prev->id << 1) : 1;
    if (w->flag & WORDL_CURRENT)
        w->also = CURRENT;      /* if WORDL_CURRENT, search also this */
    else
        w->also = 0;
    return w;
}

p4_Wordl * p4_find_wordlist (const p4_char_t* nm, int nmlen)
{
    auto p4_char_t upper[UPPERMAX];
    UPPERCOPY(upper, nm, nmlen)

    /* a special, since FORTH has no vocabulary_RT anymore */
    if (nmlen == 5 && !memcmp (nm, "FORTH", 5))
        return PFE.forth_wl;

    ___ p4_Wordl* wl;
    for (wl = VOC_LINK; wl ; wl = wl->prev)
    {
        p4_namebuf_t* nfa = wl->nfa;
        if (! nfa) continue;
        if (NAMELEN(nfa) != nmlen) continue;
        if (!memcmp (NAMEPTR(nfa), nm, nmlen) || 
	    !memcmp (NAMEPTR(nfa), upper, nmlen))
            return wl;
    } ____;
    return 0;
}

p4_Wordl * p4_find_wordlist_str (const char* nm)
{
    return p4_find_wordlist((p4_char_t*) nm, strlen(nm));
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

/** ONLY ( -- )
 * the only-vocabulary is special. Calling it will erase
 * the search => ORDER of vocabularies and only allows
 * to name some very basic vocabularies. Even => ALSO
 * is not available.
 example:
   ONLY FORTH ALSO EXTENSIONS ALSO DEFINITIONS
 */
FCode (p4_only_RT)
{
    /* NO BODY_ADDR */
    memset(CONTEXT, 0, PFE_set.wordlists*sizeof(p4_Wordl*));
    CONTEXT[0] = CURRENT = ONLY;
}

/** FORTH ( -- )
 : FORTH FORTH-WORDLIST CONTEXT ! ;
 */
FCode (p4_forth_RT)
{
    /* NO BODY_ADDR */
    CONTEXT[0] = PFE.forth_wl;
}

static p4_char_t p4_lit_ONLY[] = "ONLY";
static p4_char_t p4_lit_FORTH[] = "FORTH";
static p4_char_t p4_lit_xANSx[] = "[ANS]";
static p4_char_t p4_lit_LOADED[] = "LOADED";

_export void
p4_preload_only (void)
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
    PFE.atexit_wl = p4_make_wordlist (LAST); 
    PFE.atexit_wl->flag |= WORDL_NOHASH; /* see environment_dump in core.c */
}

/* ---------------------------------------------------------------------- *
 * initial dictionary setup                                             
 */

static void p4_load_into (const p4char* vocname, int vocname_len)
{
    if (! vocname) return;

    ___ Wordl* voc = p4_find_wordlist (vocname, vocname_len);
//    if (! voc) goto search_failed;
    if (voc)
       return;
    ___ register int i;
    for (i=PFE_set.wordlists; --i > 0; )
    {
	if (CONTEXT[i] == voc) 
	{
	    //P4_info2 ("search also '%.*s' : already there", vocname_len, vocname);
	    return;
	}
    }; ____;
    FX (pf_also);    /* the top-of-order (CONTEXT) isn't changed */
    CONTEXT [1] = voc; /* instead we place it under-the-top */
    //P4_info2 ("search also '%.*s' : done", vocname_len, vocname);
    return; ____;
// search_failed:
    // P4_warn3 ("search also failed: no '%.*s' vocabulary (%u)", vocname_len, vocname, (unsigned) vocname_len);
} 

/* ------------------------------------------------------------------- */

static FCode (p4_load_into)
{
    register char* vocname = (void*) *SP++;

    PFE.word.len = strlen ((char*) PFE.word.ptr);
    /* assume: PFE.word.ptr points to the static_string we like to have */
    TO_IN=0;
    *DP=0; /* PARSE-WORD-NOHERE */
    ___ register void* p = p4_find_wordlist (PFE.word.ptr, PFE.word.len);
    if (p) 
    {   
//	P4_debug1 (13, "load into old '%s'", PFE.word.ptr);
	CURRENT = p;
    }else{
	Wordl* current = 0;
	if (vocname) {
	    current = p4_find_wordlist_str (vocname);
	//  if (! current) 
	//	P4_warn1 ("could not find also-voc %s",  vocname);
	}
	if (! current) current = CURRENT;
	//P4_info2 ("load into new '%.*s'", (int) PFE.word.len, PFE.word.ptr);
	p4_header_comma (PFE.word.ptr, PFE.word.len, current);
	//P4_info1 ("did comma '%p'", LAST);
	FX_RUNTIME1 (pf_vocabulary);
        P4_NAMEFLAGS(p4_LAST) |= P4xIMMEDIATE;
	//P4_info1 ("done runtime '%p'", LAST);
	CURRENT = p4_make_wordlist (LAST);
	//P4_info1 ("load into current '%p'", CURRENT);
    }; ____;
    
    if (vocname) 
    {
	if (! CURRENT->also)
	    CURRENT->also = p4_find_wordlist_str (vocname);

	/* FIXME: it does nest for INTO and ALSO ? */
	p4_load_into (PFE.word.ptr, PFE.word.len); /* search-also */
    }
} 

/* ------------------------------------------------------------------- */
/**
 * (DICTVAR) forth-thread variable runtime, => VARIABLE like
 */
FCode_RT (pf_dictvar_RT)
{
    *--SP = (p4cell) ((char *) p4TH + (WP_PFA)[0]);
}

/**
 * (DICTGET) forth-thread constget runtime, => VALUE like
 */
FCode_RT (pf_dictget_RT)
{
    *--SP = *(p4cell *) ((char *) p4TH + (WP_PFA)[0]);
}

/* ------------------------------------------------------------------- */
/* >BODY is known to work on both DOES-style and VAR-style words
 * and it will even return the thread-local address of remote-style words
 * (DOES-style words are <BUILDS CREATE and DEFER in ans-forth-mode)
 */
p4cell * pf_to_body (p4xt xt)
{
    if (! xt) return P4_TO_BODY (xt);

/*
    if (p4_LogMask & P4_LOG_DEBUG)
        if ((p4char*)xt < PFE.dict || PFE.dictlimit < (p4char*)xt)
            p4_abortq ("xt in '>BODY' out of range (not in my dict space)");
*/
    if (P4_XT_VALUE(xt) == FX_GET_RT (pf_dictvar) || 
	P4_XT_VALUE(xt) == FX_GET_RT (pf_dictget)) 
        return ((p4cell*)( (char*)p4TH + P4_TO_BODY(xt)[0] ));
    else if (P4_XT_VALUE(xt) == FX_GET_RT (pf_builds) ||
             P4_XT_VALUE(xt) == FX_GET_RT (pf_does) || 
             P4_XT_VALUE(xt) == FX_GET_RT (pf_defer))
        return P4_TO_DOES_BODY(xt); 
    else /* it's not particularly right to let primitives return a body... */
        /* but otherwise we would have to if-check all known var-RTs ... */
        return P4_TO_BODY(xt);
}

/** >BODY ( some-xt* -- some-body* ) [ANS]
 * adjust the execution-token (ie. the CFA) to point
 * to the parameter field (ie. the PFA) of a word.
 * this is not a constant operation - most words have their
 * parameters at "1 CELLS +" but CREATE/DOES-words have the
 * parameters at "2 CELLS +" and ROM/USER words go indirect
 * with a rom'ed offset i.e. "CELL + @ UP +"
 */
FCode (pf_to_body)
{
    *SP = (p4cell) pf_to_body ((p4xt) *SP);
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

p4_namebuf_t** p4_name_to_link (const p4_namebuf_t* p)
{
#  ifdef PFE_WITH_ZNAME
    return (p4_namechar_t **) pf_aligned ((p4cell) (strchr((const char*) NAMEPTR(p), '\0')+1) );
# else
    return (p4_namechar_t **) pf_aligned ((p4cell) (NAMEPTR(p) + NAMELEN(p)) );
# endif
}

p4char * pf_to_name (p4xt xt)
{
    /* cfa to lfa */
    p4_Semant *s = p4_to_semant (xt);
    p4_namebuf_t ** cfa = (s ? p4_name_to_link (s->name) : (p4_namebuf_t**)( xt - 1 )); 
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

#  ifdef PFE_WITH_ZNAME
    /* Scan for flag byte. Note: this is not reliable! */
      for (;;)
      {
          /* traditional: search for CHAR of name-area with a hi-bit set
           * and assume that it is the flags/count field for the NAME */
          if (P4_NAMEFLAGS(p) & 0x80)
              return p;
          if (! isprintable (*p))
              return NULL;
          p--;
      }
#  else
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
#  endif
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
    p4xt xt = P4_LINK_FROM (p4_name_to_link (p));
    return xt;
}


/* ------------------------------------------------------------------------ */

/** ((DEFER)) ( -- )
 * runtime of => DEFER words
 */
FCode_RT (pf_defer_RT)
{
    register p4xt xt;
    xt = * (p4xt*) P4_TO_DOES_BODY(P4_BODY_FROM((WP_PFA))); /* check IS-field */
    if (xt) { PFE.execute (xt); return; }
//    else { P4_warn1 ("null execution in DEFER %p", PFE.wp); }
}

/** DEFER ( 'word' -- )
 * create a new word with ((DEFER))-semantics
 simulate:
   : DEFER  CREATE 0, DOES> ( the ((DEFER)) runtime ) 
      @ ?DUP IF EXECUTE THEN ;
   : DEFER  DEFER-RT HEADER 0 , ;
 *
 * declare as <c>"DEFER deferword"</c>  <br>
 * and set as <c>"['] executionword IS deferword"</c>
 * (in pfe, you can also use <c>TO deferword</c> to set the execution)
 */
FCode (pf_defer)
{
//    FX_RUNTIME_HEADER;
    p4_header_in(p4_CURRENT); P4_NAMEFLAGS(p4_LAST) |= P4xISxRUNTIME;
    FX_RUNTIME1 (pf_defer);
    FX_XCOMMA (0); /* <-- leave it blank (may become chain-link later) */
    FX_XCOMMA (0); /* <-- put XT here in fig-mode */
}
P4RUNTIME1(pf_defer, pf_defer_RT);
    
/* ------------------------------------------------------------------- */
void p4_load_words (const p4Words* ws, p4_Wordl* wid, int unused);

static FCode (p4_load_words)
{
    void* p = (void*) *SP++;
    if (p) p4_load_words (p, CURRENT, 0);
}

void p4_load_words (const p4Words* ws, p4_Wordl* wid, int unused)
{
    Wordl* save_current = CURRENT;
    int k = ws->n;
    const p4Word* w = ws->w;
    char dictname[NAME_SIZE_MAX+1]; char* dn;

    if (!wid) wid = CURRENT;
    
    if (ws->name) 
    {  
        //P4_info1 ("load '%s'", (ws->name));
        strncpy (dictname, ws->name, NAME_SIZE_MAX);
        dictname[NAME_SIZE_MAX] = '\0';
        if ((dn= strchr (dictname, ' '))
            ||  (dn= strchr (dictname, '(')))
            *dn = '\0';
    }else{
        sprintf (dictname, "%p", DP);
    }
    ___ extern FCode (pf_vocabulary);
    
    for ( ; --k >= 0; w++)
    {
        if (! w) continue;
	/* the C-name is really type-byte + count-byte away */
	___ char type = *w->name;

	PFE.word.ptr = ((p4_char_t*)(w->name+2));
	PFE.word.len = -1;
        *--SP = (p4cell)(w->ptr);
	
	switch (type)
	{
	case p4_LOAD:
	    FX (p4_load_words); /* RECURSION !! */
	    continue;
	case p4_INTO:
	    FX (p4_load_into);
	    continue;
	case p4_EXPT:
	    FX (pf_exception_string);
	    continue;
	case p4_SXCO:
#         ifndef HOST_WIN32
	    ___ p4_Semant* semant = (p4_Semant*)(void*)(*SP++);
#          else  /* on WIN32, the ptr is a function that returns a SemantP */
	    ___ p4_Semant* semant = ((p4_Semant*(*)()) (void*)(*SP++)) ();
#         endif

	    p4_header_in(p4_CURRENT);
	    FX_COMMA ( semant->comp );
	    if (! (semant ->name))
		semant ->name = (p4_namebuf_t*)( PFE.word.ptr-1 ); 
	    /* discard const */
	    /* BEWARE: the arg' name must come from a wordset entry to
	       be both static and have a byte in front that could be 
	       a maxlen
	    */
	    break; ____;
	case p4_RTCO:
	    ___ p4_Runtime2* runtime  = ((p4_Runtime2 *) (*SP++));
	    p4_header_in(p4_CURRENT);
	    FX_COMMA ( runtime->comp );
	    break; ____;
	case p4_IXCO:         /* these are real primitives which do */
	case p4_FXCO:         /* not reference an info-block but just */
	    p4_header_in(p4_CURRENT);        /* the p4code directly */
	    FX_COMMA ( *SP ); 
	    P4_INC(SP,p4cell);
	    break;
	case p4_XXCO:
	    p4_header_in(PFE.atexit_wl);
	    FX_COMMA ( *SP ); 
	    ((p4code)(*SP++)) ();
	    break;
	case p4_IVOC:
	case p4_OVOC:
	    FX (pf_vocabulary);
	    P4_INC(SP,p4cell);
	    break;
	case p4_DVAR:
//          FX_RUNTIME_HEADER;
            p4_header_in(p4_CURRENT); P4_NAMEFLAGS(p4_LAST) |= P4xISxRUNTIME;
	    FX_RUNTIME1_RT (pf_dictvar);
	    FX_COMMA (*SP++);
	    break;
	case p4_DCON:
//          FX_RUNTIME_HEADER;
            p4_header_in(p4_CURRENT); P4_NAMEFLAGS(p4_LAST) |= P4xISxRUNTIME;
	    FX_RUNTIME1_RT (pf_dictget);
	    FX_COMMA (*SP++);
	    break;
	case p4_OVAL:
	case p4_IVAL:
	    FX (pf_value);
	    break;
	case p4_OVAR:
	case p4_IVAR:
	    FX (pf_variable);
	    break;
	case p4_OCON:
	case p4_ICON:
	    FX (pf_constant);
	    break;
	default:
	    pf_outf("\nERROR: unknown typecode for loadlist entry "
		      "0x%x -> \"%s\"", 
		      type, PFE.word.ptr);
	} /*switch*/
	
	/* implicit IMMEDIATE still around: */
	if ('A' <= type && type <= 'Z')
            P4_NAMEFLAGS(p4_LAST) |= P4xIMMEDIATE;
	____;
    } /* for w in ws->w */

    CURRENT = save_current; /* should save_current moved to the caller? */
    ____;
}

/* ------------------------------------------------------------------- */

#ifdef _export
extern p4xcode* p4_compile_comma (p4xcode* at, p4xt);
extern p4xcode* p4_compile_xcode (p4xcode* at, p4xcode);
extern p4xcode* p4_compile_xcode_CODE (p4xcode* at, p4xcode);
extern p4xcode* p4_compile_xcode_BODY (p4xcode* at, p4xcode, p4cell*);
#endif

# ifdef PFE_CALL_THREADING
typedef struct { const char* name; const p4xcode xcode; } const loader_t;
static loader_t * loader (p4char c)
{
    static loader_t trampoline = { "trampoline", 0 };
    static loader_t primitive = { "primitive", 0 };
    static loader_t compiling = { "compiling-prim", 0 };
    static loader_t creating =  { "creating-prim", 0 };
    static loader_t createdW = { "created-word", 0 };
    static loader_t vocabulary = { "vocabulary", PFX (pf_vocabulary_RT) };
    static loader_t dictvar = { "system-variable", PFX(pf_dictvar_RT) };
    static loader_t dictget = { "system-constant", PFX(pf_dictget_RT) };
    static loader_t valuevar = { "valuevar", PFX(pf_value_RT) };
    static loader_t variable = { "variable", PFX(pf_variable_RT) };
    static loader_t constant = { "constant", PFX(pf_constant_RT) };
    static loader_t obsoleted = { "obsolete-word", PFX(p4_obsoleted_RT) };
    static loader_t unknown = { "unknown-typecode", 0 };

    switch (c)
    {
    case 0:	  return & trampoline;
    case p4_FXCO:
    case p4_IXCO: 
    case p4_XXCO: return & primitive;
    case p4_SXCO: return & compiling;
    case p4_RTCO: return & creating;
    case p4_ITEM: return & createdW;
    case p4_IVOC: 
    case p4_OVOC: return & vocabulary;
    case p4_DVAR: return & dictvar;
    case p4_DCON: return & dictget;
    case p4_OVAR: 
    case p4_IVAR: return & variable;
    case p4_OVAL: 
    case p4_IVAL: return & valuevar;
    case p4_OCON: 
    case p4_ICON: return & constant;
    case p4_iOLD: 
    case p4_xOLD: return & obsoleted ;
    default:	  return & unknown;
    }
}

p4xcode* p4_compile_xcode(p4xcode* at, p4code code)
{
    FX_COMPILE1_CALL (at, code);
    return at;
}

p4xcode* p4_compile_xcode_BODY(p4xcode* at, p4code code, p4cell* body)
{
    FX_ARG_BODY_ADDR (at, body);
    FX_COMPILE1_CALL (at, code);
    FX_PUT_BODY_ADDR (at, body);
    return at;
}

p4xcode* p4_compile_xcode_CODE(p4xcode* at, p4code code)
{
    FX_ARG_CODE_ADDR (at);
    FX_COMPILE1_CALL (at, code);
    FX_PUT_CODE_ADDR (at);
    return at;
}

/* the const here will hint where possibly sth. would write to code-mem */
const p4xcode* p4_to_code(p4xt xt)
{
    static p4xcode vocabulary = PFX(pf_vocabulary_RT);
    static p4xcode dictvar =    PFX(pf_dictvar_RT);
    static p4xcode dictget =    PFX(pf_dictget_RT);
    static p4xcode value =      PFX(pf_value_RT);
    static p4xcode variable =   PFX(pf_variable_RT);
    static p4xcode constant =   PFX(pf_constant_RT);
    static p4xcode obsoleted =  PFX(p4_obsoleted_RT);

    switch (*xt->type->def)
    {
    case 0:        /* the "" string indicates a trampoline */
    case p4_NEST:  /* a trampoline compiled by : ... ; */
    case p4_DTOR:
    case p4_FXCO:
    case p4_IXCO:
    case p4_XXCO:	return & xt->word->ptr;
    case p4_SXCO:	return & ((p4_Semant*)xt->word->ptr)->comp;
    case p4_RTCO:	return & ((p4_Runtime2*)xt->word->ptr)->comp;
    case p4_ITEM:	return & xt->call->exec[0];
    case p4_IVOC:
    case p4_OVOC:	return & vocabulary;
    case p4_DVAR:	return & dictvar ;
    case p4_DCON:	return & dictget ;
    case p4_OVAR:
    case p4_IVAR:	return & variable ;
    case p4_OVAL:
    case p4_IVAL:	return & value ;
    case p4_OCON:
    case p4_ICON:	return & constant ;
    case p4_iOLD:
    case p4_xOLD:	return & obsoleted;
    default:
	pf_outf("ERROR: <!unknown execution code!(%c:%s)>", 
                  *xt->type->def, loader(*xt->type->def)->name);
	/* not yet supported */
	return 0;
    }
    /* unreachable */
}



/* simplest form of compilation */
p4xcode* p4_compile_comma(p4xcode* at, p4xt xt)
{
    switch (*xt->type->def)
    {
    case 0: /* the "" string indicates a trampoline */
    case p4_FXCO:
    case p4_IXCO:
    case p4_XXCO:
	return p4_compile_xcode (at,xt->word->ptr);
    case p4_SXCO:
	return p4_compile_xcode (at,((p4_Semant*)xt->word->ptr)->comp);
    case p4_RTCO:
	return p4_compile_xcode (at,((p4_Runtime2*)xt->word->ptr)->comp);
    case p4_ITEM:
	if (! xt->call->flag & P4_ONLY_CODE1) 
	    return p4_compile_xcode_BODY (at,xt->call->exec[0],P4_TO_BODY(xt));
	else
	    return p4_compile_xcode (at, xt->call->exec[0]);
    case p4_DTOR: /* a destroyer-trampoline */
	return p4_compile_xcode_BODY (at, xt->word->ptr, P4_TO_BODY(xt));
    case p4_NEST: /* a CODE trampoline */
	return p4_compile_xcode (at, (p4xcode)(xt+1));
    case p4_IVOC:
    case p4_OVOC:	
    case p4_DVAR:      
    case p4_DCON:               /* all these are not primitives */
    case p4_OVAR:               /* their runtimes will fetch the */
    case p4_IVAR:               /* body-ptr being compiled here */
    case p4_OVAL:
    case p4_IVAL:  
    case p4_OCON:
    case p4_ICON:
	/* P4_fail5 ("<!word type=%c:%s xt=%p code=%p body=%p!>",
	 *           *xt->type->def, loader(*xt->type->def)->name, xt, 
	 *           *p4_to_code(xt), P4_TO_BODY(xt));
	 */
	return p4_compile_xcode_BODY (at, *p4_to_code(xt), P4_TO_BODY(xt));
    default:
	pf_outf("ERROR: <!unknown compile code!(%c:%s)>", 
                  *xt->type->def, loader(*xt->type->def)->name);
	/* not yet supported */
	return at;
    }
    /* unreachable */
}

# if defined PFE_SBR_CALL_THREADING
/* ... and here are the SBR snippets needed to call an XT from C ... */

# if defined PFE_SBR_CALL_ARG_THREADING
/* defeat the compiler which wishes to optimize arg away for being unused */
#  if defined PFE_HOST_ARCH_I386
/*  the i386 architecture is so heavily register-starved that it does
 *  quite always setup a local frame which however breaks the ret-jmp
 *  asm-code presented. So what, we make another subroutine for which
 *  hopefully the compiler will not try to build an extra locals frame */
#   define __call(X,Y) { \
     register void* _v P4_SBR_TAKE_BODY; \
     _v = (X);        asm volatile ("push %0":: "r" (_v)); \
     _v = P4_TO_BODY (Y); asm volatile ("ret":: "r" (_v)); }
#   define _call(X,Y) p4_sbr_call_arg((X),(Y),(Y)) 
    void p4_sbr_call_arg(void* code, void* xt1, void*xt2) { __call(code,xt1); }
#  elif defined PFE_HOST_ARCH_M68K
#   define _call(X,Y) { \
     register p4xcode _x asm ("%a0") = (X); \
     register void* _y asm ("%a1") = P4_TO_BODY (Y); \
     asm volatile ("jsr %0@":: "r" (_x), "r" (_y)); }
#  elif defined PFE_HOST_ARCH_POWERPC
#   define _call(X,Y) { \
     register p4xcode _x asm ("0") = (X); \
     register p4xcode _y P4_SBR_TAKE_BODY = P4_TO_BODY (Y); \
     asm volatile ("mtlr %0" :: "r" (_x)); \
     asm volatile ("blrl" :: "r" (_y)); }

#  else
#   error need to define asm p4_sbr_call for this architecture
#  endif /* PFE_HOST_* */
# endif /* PFE_SBR_CALL_ARG_THREADING */

#if PFE_SBR_CALL_DEBUGGING+0
static void _enter() { p4_outf(" {<%p:%p>", p4RP,p4RP[-2]); }
static void _leave() { p4_outf("<%p:%p>}", p4RP,p4RP[-2]); }
void p4_sbr_call (p4xt xt) {
    _enter();
    _p4_sbr_call(xt);
    _leave();
}
#define p4_sbr_call _p4_sbr_call
#else
#define _enter()
#define _leave()
#endif

_export void p4_sbr_call (p4xt xt)
{
# if defined PFE_HOST_ARCH_I386 /* && ! SBR_CALL_ARG */

    /* the modern RISC architectures do not quite like it when some memory
     * area is modified and executed right away. It emerges to be a variant
     * of the problems about self-modifiying code. Even a jump via register
     * did not help it. So far, only the i386 processors can stand the
     * following simple code that is based on the compile_comma code
     */

    /* sbr-stub, xt-code, xt-data, sbr-exit */
    p4xcode list[6] /* = { 0,0,0,0,0,0 } */;
    void* p = p4_compile_comma(list, xt);
    PFE_SBR_COMPILE_EXIT(p);    _enter();
    ((p4code) (list))();        _leave();
    return;

    /* note however, that quite some i386-type processors do not honour
     * such conditions lightly - they might not just only flush the
     * the instruction pipeline, they might even flush the L1 cache when
     * there are different L1 caches for data and code. You don't want that.
     */
# elif defined PFE_SBR_CALL_ARG_THREADING
    /* and here, we have to recreate the variants of our compile_xcode
     * routines, in this case however, we just want to have the register effect
     * immediatly that the compiled code would have, and jump to the target
     * xcode - it is almost easy with sbr-arg threading since we just need
     * to setup the arg-register correctly, and then call the actual routine.
     */

    switch (*xt->type->def)
    {
    case 0: /* the "" string indicates a trampoline */
    case p4_FXCO:
    case p4_IXCO:
    case p4_XXCO:
        xt->word->ptr (); return; 
        /* p4_compile_xcode (at,xt->word->ptr); */
    case p4_SXCO:
        ((p4_Semant*)xt->word->ptr)->comp (); return;
	/* p4_compile_xcode (at,((p4_Semant*)xt->word->ptr)->comp); */
    case p4_RTCO:
        ((p4_Runtime2*)xt->word->ptr)->comp (); return;
	/* p4_compile_xcode (at,((p4_Runtime2*)xt->word->ptr)->comp);*/
    case p4_ITEM:
        _call (xt->call->exec[0], xt); return;
	/* if (! xt->call->flag & P4_ONLY_CODE1) 
         *  return p4_compile_xcode_BODY (at,xt->call->exec[0],P4_TO_BODY(xt));
         * else
         *  return p4_compile_xcode (at, xt->call->exec[0]);
         */
    case p4_DTOR: /* a destroyer-trampoline */
        _call (xt->word->ptr, xt); return;
	/* p4_compile_xcode_BODY (at, xt->word->ptr, P4_TO_BODY(xt)); */
    case p4_NEST: /* a CODE trampoline */
        ((p4xcode)(xt+1)) (); return;
	/* p4_compile_xcode (at, (p4xcode)(xt+1)); */
    case p4_IVOC:
    case p4_OVOC:	
    case p4_DVAR:      
    case p4_DCON:               /* all these are not primitives */
    case p4_OVAR:               /* their runtimes will fetch the */
    case p4_IVAR:               /* body-ptr being compiled here */
    case p4_OVAL:
    case p4_IVAL:  
    case p4_OCON:
    case p4_ICON:
	/* P4_note5 ("<!word type=%c:%s xt=%p code=%p body=%p!>",
	 *           *xt->type->def, loader(*xt->type->def)->name, xt, 
	 *           *p4_to_code(xt), P4_TO_BODY(xt));
	 */
        
        _call (*p4_to_code(xt), xt); return; 
	/* p4_compile_xcode_BODY (at, *p4_to_code(xt), P4_TO_BODY(xt)); */
    default:
	pf_outf("\nERROR: <!unknown execute code!(%c:%s)>", 
                  *xt->type->def, loader(*xt->type->def)->name);
	/* not yet supported */
	return;
    }
    /* unreachable */
# else /* other HOST_* && ! SBR_CALL_ARG */
    /* for the case of sbr-call no-arg, we will need to do it quite
     * differently. Here we have to lie about the return-code that is
     * on the stack and which will be used to return later on. Instead
     * of setting an arg-register, we set a memory cell and take the
     * address of it to be pushed on the return-stack as the return
     * address for the items with an call-body, the other items can
     * just be called as is. Then we JUMP into the routine instead of
     * CALL to the routine, which will execute that routine and let
     * it return via the ret-code that follows the data area. However,
     * this is not tested - I'm not sure if it works on superpipelined
     * RISC machines although I guess it should. For the work at the
     * Tek labs, the no-arg sbr-threaded mode is not used anyway. Feel
     * free to add it. Otherwise this area will be left incomplete
     * as its support is only academic - for commercial grade developments
     * all the cpu docs are at hand, so it should be always possible to
     * define the bits to use sbr-call-arg threading.
     */
    pf_outf("ERROR: <!sbr-call no-arg is not supported on this platform,"
              " can not handle execute code!(%c:%s)>", 
              *xt->type->def, loader(*xt->type->def)->name);
    return;
# endif
}

/* _SBR_CALL_THREADING */
# endif
/* _CALL_THREADING */
#endif
/* -------------------------------------------------------------- */
FCode_RT (pf_destroyer_RT)
{   
    /* this code is a trampoline for ITC code not using an FFA flag.
     * we just expect the a prior p4_call in p4_forget has setup an
     * appropriate BODY pointer - either it goes through a p4WP or
     * indirectly through p4IP. All we have to do is, to fetch the
     * actual ccode from PFA[1] and branch down into the target code.
     */
    ((p4code*)(WP_PFA))[1]();
}

/** ((FORGET)) 
 * remove words from dictionary, free dictionary space, this is the
 * runtime helper of => (FORGET)
 */
FCode (pf_forget_dp)
{
    register p4_Wordl *wl;
    register p4char* new_dp = PFE.forget_dp;

    /* unchain words in all threads of all word lists: */
    for (wl = VOC_LINK; wl; wl = wl->prev)
    {
        p4_namebuf_t **p = wl->thread;
        int i;
       
//	if (0) if (wl->nfa) 
//	    P4_debug2(4, "\"%.*s\"", NAMELEN(wl->nfa), NAMEPTR(wl->nfa));

        for (i = THREADS; --i >= 0; p++)
        {  /* unchain words in thread: */
            while (*p >= new_dp) 
            {
                // if (PFE_IS_DESTROYER(*p))
                if( P4_NAMEFLAGS(*p) & P4xIMMEDIATE
                    && P4_XT_VALUE(P4_LINK_FROM(pf_name_to_link(*p))) == FX_GET_RT(pf_destroyer))
                {
                    //P4_info2 (" destroy: \"%.*s\"", NAMELEN(*p), NAMEPTR(*p));
                    pf_call (pf_name_from (*p));
                    new_dp = PFE.forget_dp; /* forget_dp is volatile */
                    /* and may have changed through recursive forget */
                }
                *p = *p4_name_to_link (*p);
            }
        }
    }

    /* unchain word lists: */
    while (VOC_LINK && VOC_LINK >= (p4_Wordl *) new_dp) 
    {   
        {   /* delete from search-order */   
            int i;
            for (i=0; i < PFE_set.wordlists; i++) 
            {
                if (CONTEXT[i] == VOC_LINK) 
                {
                    CONTEXT[i] = NULL;
                }
            
                if (PFE.dforder[i] == VOC_LINK) 
                {
                    PFE.dforder[i] = NULL;
                }
            }
        }
        
        VOC_LINK = VOC_LINK->prev;
    }
    /* compact search-order */
    { register int i, j;
      for (i=0, j=0; i < PFE_set.wordlists; i++)
      {
        if (CONTEXT[i]) CONTEXT[j++] = CONTEXT[i];
      }
      while (j < PFE_set.wordlists) CONTEXT[j++] = NULL;

      for (i=0, j=0; i < PFE_set.wordlists; i++)
      {
        if (PFE.dforder[i]) PFE.dforder[j++] = PFE.dforder[i];
      }
      while (j < PFE_set.wordlists) PFE.dforder[j++] = NULL;
    }
    /* free dictionary space: */
    DP = (p4char *) new_dp; 
    LAST = NULL;
    PFE.forget_dp = 0;

    if (CURRENT >= (p4_Wordl *) new_dp) 
    {
        if (CONTEXT[0]) CURRENT = PFE.forth_wl; /* initial CURRENT */
    }
}

/** (FORGET)
 * forget anything above address
 */
void pf_forget (p4_byte_t* above)
{
    if ((p4char*) above < FENCE)
        p4_throw (P4_ON_INVALID_FORGET);

    if (PFE.forget_dp) /* some pf_forget_dp already started */
    {
        /* P4_info1 ("recursive forget %p", above); */
        if (PFE.forget_dp > above) 
        {
            PFE.forget_dp = above; /* update pf_forget_dp argument */
        }
    }else{ 
        /* P4_info1 ("forget start %p", above); */
        PFE.forget_dp = above; /* put new argument for pf_forget_dp */
        FX (pf_forget_dp);     /* forget execution start */
    }
}

/** FORGET ( "word" -- )
 simulate:
   : FORGET  [COMPILE] '  >NAME (FORGET) ; IMMEDIATE
 */
FCode (pf_forget)
{
    if (LAST) FX (pf_reveal);
    pf_forget (P4_NAMESTART (pf_tick_nfa ()));
}

/*
static void pf_create_marker (const p4_char_t* name, p4cell len, p4_Wordl* wordlist)
{
    int i;
    p4char* forget_address = PFE.dp;

    p4_header_comma (name, len, wordlist);
    FX_RUNTIME1 (p4_marker);

    FX_PCOMMA (forget_address); // PFE.dp restore
    FX_PCOMMA (p4_FENCE);
    FX_PCOMMA (p4_LAST);
    FX_PCOMMA (p4_ONLY);
    FX_PCOMMA (p4_CURRENT);
*/
    /* we do not need to memorize null-ptrs in the search-order,
     * and we use a nullptr to flag the end of the saved ptrlist
     */
/*
    for (i=0; i < PFE_set.wordlists ; i++)
        if (p4_CONTEXT[i]) 
            FX_PCOMMA (p4_CONTEXT[i]); 
    FX_UCOMMA (0);

    for (i=0; i < PFE_set.wordlists ; i++)
        if (p4_DFORDER[i]) 
            FX_PCOMMA (p4_DFORDER[i]); 
    FX_UCOMMA (0);
}
*/

/** "(MARKER)" ( str-ptr str-len -- ) [HIDDEN]
 * create a named marker that you can use to => FORGET ,
 * running the created word will reset the dict/order variables
 * to the state at the creation of this name.
 : (MARKER) (CREATE) HERE , 
         GET-ORDER DUP , 0 DO ?DUP IF , THEN LOOP 0 , 
         ...
   DOES> DUP @ (FORGET) 
         ...
 ; 
 */
/*
FCode (pf_paren_marker)
{
    register p4cell len = *SP++; 
    register p4char* name = (p4char*) *SP++;
    p4_create_marker (name, len, CURRENT);
}
*/
/** "((MARKER))" ( -- ) [HIDDEN]
 * runtime compiled by => MARKER
 */ 
FCode_RT (pf_marker_RT)
{
    int i;
    void** RT = (void*) WP_PFA;
    void* forget_address;
    /* assert (sizeof(void*) == sizeof(p4cell)) */

    forget_address =       (*RT++); 
    p4_FENCE =   (p4char*) (*RT++);
    p4_LAST =    (p4char*) (*RT++);
    p4_ONLY =     (Wordl*) (*RT++);
    p4_CURRENT =  (Wordl*) (*RT++);
    for (i=0; i < PFE_set.wordlists ; i++)
    {
        if (! *RT) 
            p4_CONTEXT[i] = 0; /* no RT++ !! */
        else
            p4_CONTEXT[i] = (Wordl*) (*RT++);
    }
    while (*RT) RT++;
    RT++; /* skip null */
    for (i=0; i < PFE_set.wordlists ; i++)
    {
        if (! *RT)
            p4_DFORDER[i] = 0; /* no RT++ !! */
        else
            p4_DFORDER[i] = (Wordl*) (*RT++);
    }
    pf_forget (forget_address); /* will set the PFE.dp */
    /* MARKER RT wants (FORGET) to prune VOC_LINK and run DESTROYERs */
}

/** MARKER ( 'name' -- ) [ANS]
 * create a named marker that you can use to => FORGET ,
 * running the created word will reset the dict/order variables
 * to the state at the creation of this name.
 : MARKER PARSE-WORD (MARKER) ;
 * see also => ANEW which is not defined in ans-forth but which uses
 * the => MARKER functionality in the way it should have been defined.
 : MARKER PARSE-WORD (MARKER) ;
 */
/*
    FX_2ROOM;
    pf _skip _spaces();
    pf _parse _word(' ');
    *DP = 0;
    SP[1] = (p4ucell) PFE.word.ptr;
    SP[0] = (p4ucell) PFE.word.len;
*/
FCode (pf_marker)
{
//    FX (p4_parse_word);
//    FX (pf_paren_marker);
}
P4RUNTIME1(pf_marker, pf_marker_RT);

/* -------------------------------------------------------------- */
/* -------------------------------------------------------------- */
/** ((VOCABULARY)) ( -- ) [HIDDEN]
 * runtime of a => VOCABULARY
 */ 
FCode_RT (pf_vocabulary_RT)
{
    CONTEXT[0] = (Wordl *) WP_PFA;
}

/** VOCABULARY ( "name" -- ) [FTH]
 * create a vocabulary of that name. If the named vocabulary
 * is called later, it will run => ((VOCABULARY)) , thereby
 * putting it into the current search order.
 * Special pfe-extensions are accessible via 
 * => CASE-SENSITIVE-VOC and => SEARCH-ALSO-VOC
 simulate:
   : VOCABULARY  CREATE ALLOT-WORDLIST
        DOES> ( the ((VOCABULARY)) runtime )
          CONTEXT ! 
   ; IMMEDIATE
 */
FCode (pf_vocabulary)
{
    p4_header_in(p4_CURRENT);
    FX_RUNTIME1(pf_vocabulary);
    p4_make_wordlist (LAST);
}
P4RUNTIME1(pf_vocabulary, pf_vocabulary_RT);

/* -------------------------------------------------------------- */
P4_LISTWORDS (dictionary) =
{
//    P4_INTO ("FORTH", 0),
    P4_DVaL ("CONTEXT",      context),
    P4_DVaR ("CURRENT",      current),
    P4_FXco (">BODY",        pf_to_body),
    P4_FXco (">NAME",        pf_to_name),
    P4_FXco ("LATEST",       pf_latest),
    P4_FXco ("ALSO",         pf_also),
    P4_FXco ("ORDER",        pf_order),
    P4_FXco ("FORGET",	     pf_forget),
    P4_RTco ("MARKER",       pf_marker),
    P4_RTco ("VOCABULARY",   pf_vocabulary),
};
P4_COUNTWORDS (dictionary, "Dictionary words");
