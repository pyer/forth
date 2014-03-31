/**
 * --  Forth internal interpreter
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
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

FCode (p4_debug_colon_RT);
FCode (p4_debug_colon);
FCode (p4_debug_does_RT);
FCode (p4_debug_does);
/* -------------------------------------------------------------- */
// display a message when a word is redefined
int redefined_msg = 0;
/* -------------------------------------------------------------- */
// input buffer
char* source;
int length = 0;

void print_source(void)
{
    pf_type(source,length);
}

/* -------------------------------------------------------------- */
/** SOURCE ( -- c-addr u )
 *  c-addr is the address of, and u is the number of characters in, the input buffer.
 */
FCode (pf_source)
{
    *--SP = (p4cell) source;
    *--SP = (p4cell) length;
}

/* -------------------------------------------------------------- */
/** LATEST ( -- nfa )
 * return the NFA of the latest created word
 */
FCode (pf_latest)			
{
    *--SP = (p4cell) LATEST;
}


/* **********************************************************************
 * inner and outer interpreter
 */

/**
 * longjmp via (Except->jmp) following inline
 * - purpose: stop the inner interpreter
 */
FCode_XE (pf_call_stop)
{
    p4_Except *buf = (p4_Except *) *PFE.ip;
    longjmp (buf->jmp, 1);
}

/**
 * Run a forth word from within C-code
 * - this is the inner interpreter
 */
void pf_call_loop (p4xt xt)
{
    p4_Except stop;

    static p4code call_stop = P4CODE (pf_call_stop);
    p4xcode list[3];
    list[0] = xt;
    list[1] = &call_stop;
    list[2] = (p4xcode) &stop;

    PFE.ip = list;
    PFE.wp = *PFE.ip;

    if (setjmp (stop.jmp))
    {
        return;
    }

    /* next_loop */
    for (;;)
    {
	/* ip and p4WP are same: register or not */
        PFE.wp = *PFE.ip++, (*PFE.wp) (); // next
    }
}

/**
 */
void pf_call (p4xt xt)
{
    p4xcode *saved_ip = PFE.ip;
    pf_call_loop (xt);
    PFE.ip = saved_ip;
}

/* -------------------------------------------------------------- */
/**
 * the NEXT call. Can be replaced by p4_debug_execute to
 * trace the inner forth interpreter.
 */
void pf_normal_execute (p4xt xt)
{
    pf_call(xt);
}

/* -------------------------------------------------------------- */
/** number to digit ( val -- c )
 * make digit
 */
char pf_number2digit(p4ucell n)
{
    if (n < 10)
        return n + '0';
    if (n < 10 + 'Z' - 'A' + 1)
        return n - 10 + 'A';
    else
        return n - (10 + 'Z' - 'A' + 1) + 'a';
}

/** digit to number ( c n* base -- ?ok )
 * Get value of digit c into *n, return flag: valid digit.
 */
int pf_digit2number (p4_char_t c, p4ucell *n, p4ucell base)
{
    if (c < '0')
        return P4_FALSE;
    if (c <= '9')
        c -= '0';
    else
    {
        if (c < 'A')
            return P4_FALSE;
        if (c <= 'Z')
            c -= 'A' - ('9' - '0' + 1);
        else
	{
            if (c < 'a')
                return P4_FALSE;
            c -= 'a' - ('9' - '0' + 1);
	}
    }
    if (c >= base)
        return P4_FALSE;
    *n = c;
    return P4_TRUE;
}

/* -------------------------------------------------------------- */
/** _lower_ ( str* str# -- )
 * _tolower_ applied to a stringbuffer
 : _lower_ 0 do dup c@ _tolower_ over c! 1+ loop drop ;
 */
void p4_lower (p4_char_t *p, int n)
{
    while (--n >= 0)
    {
        *p = (p4_char_t) tolower ((char) *p);
        p++;
    }
}

/** _upper_ ( str* str# -- )
 * _toupper_ applied to a stringbuffer
 : _upper_ 0 do dup c@ _toupper_ over c! 1+ loop drop ;
 */
void p4_upper (p4_char_t *p, int n)
{
    while (--n >= 0)
    {
        *p = (p4_char_t) toupper ((char) *p);
        p++;
    }
}

/* -------------------------------------------------------------- */

p4char* search_thread (const p4_char_t *nm, int l, p4char *t, const p4_Wordl* wl)
{
    auto p4_char_t upper[UPPERMAX];
        UPPERCOPY (upper, nm, l);
        /* this thread does contain some upper-case defs 
           AND lower-case input shall match those definitions */
        while (t)
        {
            if (! (P4_NAMEFLAGS(t) & P4xSMUDGED) && NAMELEN(t) == l)
            {
                if (!memcmp (nm, NAMEPTR(t), l))  break;
                if (!memcmp (upper, NAMEPTR(t), l)) break;
            }
            t = *pf_name_to_link (t);
        }
    return t;
}

/* search all word lists in the search order for name, return NFA 
 * (we use the id speedup here - the first WLs have each a unique bitmask
 *  in the wl->id. Especially the FORTH wordlist can be present multiple
 *  time - even in being just search via wl->also. With w->id each is just
 *  searched once - atleast for each of the WLs that have gotten an id-bit
 *  which on a 32bit system are 32 WLs - enough for many system setups.
 *  It might be possible to use the old code even here (that walked the
 *  ORDER to see if the next WL is present in an earlier slot) but in a
 *  system with so many vocs it is quite improbable to find duplicates
 *  other than the basic vocs like FORTH in there anyway - so we use this
 *  one that might search a WL twice theoretically. Tell me of occasions
 *  where that is really a problem - in my setups it happens that the ORDER
 *  overflows much before getting duplicates other than the basic wordlists.
 */
p4char * pf_find (const p4_char_t *nm, int l)
{
    return search_thread (nm, l, CURRENT->link, CURRENT );
}

/** FIND ( name-bstr* -- name-bstr* 0 | name-xt* -1|1 ) [ANS]
 * looks into the current search-order and tries to find
 * the name string as the name of a word. Returns its
 * execution-token or the original-bstring if not found,
 * along with a flag-like value that is zero if nothing
 * could be found. Otherwise it will be 1 (a positive value)
 * if the word had been immediate, -1 otherwise (a negative
 * value).
 */
FCode (pf_find)
{
    p4char *p = (p4char *) *SP;

    p = pf_find (p + 1, *p);
    if (p)
    {
        *SP = (p4cell) pf_name_from (p);
        *--SP = (P4_NAMEFLAGS(p) & P4xIMMEDIATE) ? P4_POSITIVE : P4_NEGATIVE;
    }
    else
        *--SP = 0;
}

/* -------------------------------------------------------------- */
/** HERE ( -- here* ) [ANS]
 * used with => WORD and many compiling words
 simulate:   : HERE DP @ ;
 */
FCode (pf_here)
{
    *--SP = (p4cell) DP;
}

/** _hold_ ( c -- )
 * insert into pictured numeric output string
 */
void pf_hold (char c)
{
    if (HLD <= DP)
        p4_throw (P4_ON_PICNUM_OVER);
    *--HLD = c;
}

/** HOLD ( char# -- ) [ANS]
 * the old-style forth-formatting system -- this
 * word adds a char to the picutred output string.
 */
FCode (pf_hold)
{
    pf_hold ((char) *SP++);
}

/** PAD ( -- pad* ) [ANS]
 * transient buffer region
 */
FCode (pf_pad)
{
    *--SP = (p4cell) PAD;
}

/* -------------------------------------------------------------- */

/** SIGN ( a# -- ) [ANS]
 * put the sign of the value into the hold-space, this is
 * the forth-style output formatting, see => HOLD
 */
FCode (pf_sign)
{
    if (*SP++ < 0)
        pf_hold ('-');
}

/** <# ( -- ) [ANS]
 * see also => HOLD for old-style forth-formatting words
 * and => PRINTF of the C-style formatting - this word
 * does initialize the pictured numeric output space.
 */
FCode (pf_less_sh)
{
    HLD = PAD;
}

/** # ( n -- n' ) [ANS]
 * see also => HOLD for old-style forth-formatting words
 * and => PRINTF of the C-style formatting - this word
 * divides the argument by => BASE and add it to the
 * picture space - it should be used inside of => <#
 * and => #>
 */
FCode (pf_sh)
{
    p4ucell u = *SP;
    udiv_t res;
    res.quot = u / BASE;
    res.rem  = u % BASE;
    *SP = res.quot;
    pf_hold (pf_number2digit(res.rem));
}

/** #> ( n -- hold-str-ptr hold-str-len ) [ANS]
 * see also => HOLD for old-style forth-formatting words
 * and => PRINTF of the C-style formatting - this word
 * drops the argument and returns the picture space
 * buffer
 */
FCode (pf_sh_greater)
{
    *SP = (p4cell) HLD;
    *--SP = (p4cell) (PAD - HLD);
}

/** #S ( n -- 0 ) [ANS]
 * see also => HOLD for old-style forth-formatting words
 * and => PRINTF of the C-style formatting - this word
 * does repeat the word => # for a number of times, until
 * the argument becomes zero. Hence the result is always
 * null - it should be used inside of => <# and => #>
 */
FCode (pf_sh_s)
{
    do {
        FX (pf_sh);
    } while (SP[0]);
}

/* -------------------------------------------------------------- */
void pf_skip_spaces(void)
{
//    const p4_char_t *q = PFE.tib;
    char *q=source;
    int   n = length;
    int   i = TO_IN;

    while ( i < n && isascii (q[i]) && isspace (q[i]) )
             i++;
    TO_IN = i;
}

void pf_parse_word( char delimiter )
{
//    const p4_char_t *q = PFE.tib;
    char *q=source;
    int   n = length;
    int   i = TO_IN;

    PFE.word.ptr = (p4_char_t*) q + i;
    PFE.word.len = 0;

//printf("n=%d >IN=%d [%s]\n",n,(int)TO_IN,TIB);
    /* BL && QUOTED -> before whitespace and after doublequote */
    while ( i < n && q[i]>=' ' && q[i]!=delimiter ) {
        i++;
    }
    PFE.word.len = i - TO_IN;
    /* put the ">IN" pointer just after the delimiter that was found */
    TO_IN = i+1;
}

/* -------------------------------------------------------------- */
/**
 * tick next word,  and
 * return count byte pointer of name field (to detect immediacy)
 */
p4char * pf_tick_nfa (void)
{
    register p4char *p;
    pf_skip_spaces();
    pf_parse_word(' ');
    *DP = 0; /* PARSE-WORD-NOHERE */
    p = pf_find (PFE.word.ptr, PFE.word.len);
    if (! p)
        p4_throw (P4_ON_UNDEFINED);
    return p;
}

/**
 * tick next word,  and return xt
 */
p4xt pf_tick_cfa (void)
{
    return pf_name_from (pf_tick_nfa ());
}

/** "'" ( 'name' -- name-xt* ) [ANS]
 * return the execution token of the following name. This word
 * is _not_ immediate and may not do what you expect in
 * compile-mode. See => ['] and => '> - note that in FIG-forth
 * the word of the same name had returned the PFA (not the CFA)
 * and was immediate/smart, so beware when porting forth-code
 * from FIG-forth to ANSI-forth.
 */
FCode (pf_tick)
{
    *--SP = (p4cell) pf_tick_cfa();
}

/* -------------------------------------------------------------- */

const p4_char_t * pf_to_number (const p4_char_t *p, p4ucell *n, p4cell *d, p4ucell base)
{
    p4cell value = 0;
    int sign = 0;
    if (*p == '-') { p++; n--; sign = 1; }

    for (; *n > 0; p++, --*n) {
        p4ucell c;
        if (!pf_digit2number(*p, &c, base))
            break;
        value = value * base + c;
    }
    if ( sign )
       *d = -value;
    else
       *d = value;
    return p;
}

/** _?number_ ( str* str# dcell* -- ?ok )
 * try to convert into number, see => ?NUMBER
 */
_export int
p4_number_question (const p4_char_t *p, p4ucell n, p4dcell *d)
{
    p4ucell base = 0;
    int sign = 0;
    if (*p == '-') { p++; n--; sign = 1; }

    if (base == 0)
        base = BASE;

    d->lo = d->hi = 0;
    DPL = -1;
    p = pf_to_number (p, &n, (p4cell *) d->lo, base);
    if (n == 0)
        goto happy;
happy:
    if (sign)
        d->lo=-d->lo;
//        p4_d_negate (d);

    return P4_TRUE;
}

/** >NUMBER ( a str-ptr str-len -- a' str-ptr' str-len) [ANS]
 * try to convert a string into a number, and place
 * that number at a respecting => BASE
 */
FCode (pf_to_number)
{
    SP[1] = (p4cell)
        pf_to_number (
                      (p4_char_t *) SP[1],
                      (p4ucell *) &SP[0],
                      (p4cell *) &SP[2],
                      BASE);
}

/* -------------------------------------------------------------- */
int pf_find_word(void)
{
    register p4char *nfa;
    register p4xt xt;

    /* WORD-string is at HERE and at PFE.word.ptr / PFE.word.len */
    nfa = pf_find (PFE.word.ptr, PFE.word.len);
    if (! nfa)
        return 0;

    xt = pf_name_from (nfa);
    if (! STATE || (P4_NAMEFLAGS(nfa) & P4xIMMEDIATE))
    {
	pf_call (xt);           /* execute it now */
	FX (pf_Q_stack);        /* check stack */
    }else{
	FX_XCOMMA (xt);  /* comma token */
    }
    return 1;
}

/* pf_convert_number(void)
 * convert a string to a number using BASE or prefix
 * Prefix are 0x for hexa, 0b for binary, 0o for octal
 *
 */
int pf_convert_number(void)
{
    /* WORD-string is at HERE and at PFE.word.ptr / PFE.word.len */
    const p4_char_t *p = PFE.word.ptr;
    p4ucell          n = PFE.word.len;

    p4ucell base = 0;
    int sign = 0;
    p4cell value = 0;

    if (*p == '-') {
        p++; n--; sign = 1;
    }

    if( n > 2 && *p == '0' )
    {
        switch(*(p+1))
        {
         case 'x':
         case 'X':
             if (BASE <= 10+'X'-'A') {
                 base = 16; p+=2; n-=2;
             }
             break;
         case 'o':
         case 'O':
             if (BASE <= 10+'O'-'A') {
                 base = 8; p+=2; n-=2;
             }
             break;
         case 'b':
         case 'B':
             if (BASE <= 10+'B'-'A') {
                 base = 2; p+=2; n-=2;
             }
             break;
        }
    }

    if (base == 0)
        base = BASE;

    for (; n > 0; p++, --n)
    {
        p4ucell c;
        if (!pf_digit2number(*p, &c, base))
            break;
        value = value * base + c;
    }

    if (n != 0)
        return 0;
    if (sign)
        value = -value;

    if (STATE)
    {
	FX_COMPILE (pf_literal);
        FX_SCOMMA(value);
    }else{
	*--SP = value;
    }
    return 1;
}

/* -------------------------------------------------------------- */
/** "." ( value# -- | value* -- [?] | value -- [??] ) [ANS]
 * print the numerical value to stdout - uses => BASE
 */
FCode (pf_dot)
{
    char sign=' ';
    FX (pf_less_sh);

    if (*SP < 0) {
        sign = '-';
        *SP = -(*SP);
    }

    do {
        FX (pf_sh);
    } while (*SP);

    if (sign == '-' )
        pf_hold ('-');

    FX (pf_sh_greater);
    FX (pf_type);
}

/** "PARSE,\""  ( "chars<">" -- ) [HIDDEN]
 *  Store a quote-delimited string in data space as a counted
 *  string.
 : ," [CHAR] " PARSE  STRING, ; IMMEDIATE
 *
 * implemented here as
 : PARSE," [CHAR] " PARSE, ; IMMEDIATE
 */
FCode (pf_parse_comma_quote)
{
    pf_parse_word('"');
    const p4_char_t *s = PFE.word.ptr;    
    int len   = PFE.word.len; 
    *DP++ = len;                /* store count byte */
    while (--len >= 0)          /* store string */
        *DP++ = *s++;
    FX (pf_align);
}

/** '((.\"))' ( -- ) [HIDDEN] skipstring
 * compiled by => ." string"
 */
FCode_XE (pf_dot_quote_execution)
{
    register p4_char_t *p = (p4_char_t *) PFE.ip;
    pf_type ((const char *)p + 1, *p);
    FX_SKIP_STRING;
}

/** '.\"' ( [string<">] -- ) [ANS]
 * print the string to stdout
 */
FCode (pf_dot_quote)
{
    if (STATE)
    {
        FX_COMPILE (pf_dot_quote);
	FX (pf_parse_comma_quote);
    }else{
        pf_skip_spaces();
        pf_parse_word('"');
        pf_type ((const char *)PFE.word.ptr, PFE.word.len);
    }
}
P4COMPILES (pf_dot_quote, pf_dot_quote_execution, P4_SKIPS_STRING, P4_DEFAULT_STYLE);

/** "("  ( 'comment<closeparen>' -- ) [ANS]
 * eat everything up to the next closing paren - treat it
 * as a comment.
 */
FCode (pf_paren)
{
        pf_parse_word(')');
}

/** "\\" ( [comment<eol>] -- ) [ANS]
 * eat everything up to the next end-of-line so that it is
 * getting ignored by the interpreter.
 */
FCode (pf_backslash)
{
        TO_IN = length;
}

/* -------------------------------------------------------------- */
void pf_dot_name (const p4_namebuf_t *nfa)
{
    if (! nfa || ! (P4_NAMEFLAGS(nfa) & 0x80)) {
        pf_outs ("<?""?""?> ");  /* avoid C preprocessor trigraph */
    } else {
        pf_type ((const char *)NAMEPTR(nfa), NAMELEN(nfa));
        FX (pf_space);
    }
}

/* -------------------------------------------------------------- */
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

/** ((DEFER)) ( -- )
 * runtime of => DEFER words
 */
FCode_RT (pf_defer_RT)
{
    register p4xt xt;
    xt = * (p4xt*) P4_TO_DOES_BODY(P4_BODY_FROM((WP_PFA))); /* check IS-field */
    if (xt) {
        PFE.execute (xt);
    }
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
    p4_header_in(CURRENT); P4_NAMEFLAGS(LATEST) |= P4xISxRUNTIME;
    FX_RUNTIME1 (pf_defer);
    FX_XCOMMA (0); /* <-- leave it blank (may become chain-link later) */
    FX_XCOMMA (0); /* <-- put XT here in fig-mode */
}
P4RUNTIME1(pf_defer, pf_defer_RT);

/* -------------------------------------------------------------- */
char pf_category (p4code p)
{
    if (p == P4CODE(pf_colon_RT) || p == P4CODE(p4_debug_colon_RT))
        return ':';
    if (p == P4CODE(pf_variable_RT) || p == P4CODE(pf_value_RT) || p == P4CODE(pf_builds_RT))
        return 'V';
    if (p == P4CODE(pf_constant_RT))
        return 'C';
//    if (p == P4CODE(p4_vocabulary_RT))
//        return 'W';
    if (p == P4CODE(pf_does_RT) || p == P4CODE(p4_debug_does_RT))
        return 'D';
//    if (p == P4CODE(p4_marker_RT))
//        return 'M';
    if (p == P4CODE(pf_defer_RT))
        return 'F'; 
    /* must be primitive */
    return 'p';
}

/* -------------------------------------------------------------- */
/* >BODY is known to work on both DOES-style and VAR-style words
 * and it will even return the thread-local address of remote-style words
 * (DOES-style words are <BUILDS CREATE and DEFER in ans-forth-mode)
 */
p4cell * cfa_to_body (p4xt xt)
{
    if (! xt) return P4_TO_BODY (xt);

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
/* -------------------------------------------------------------- */
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
    ___
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
	case p4_EXPT:
	    FX (pf_exception_string);
	    continue;
	case p4_SXCO:
	    ___ p4_Semant* semant = (p4_Semant*)(void*)(*SP++);
	    p4_header_in(CURRENT);
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
	    p4_header_in(CURRENT);
	    FX_COMMA ( runtime->comp );
	    break; ____;
	case p4_IXCO:         /* these are real primitives which do */
	case p4_FXCO:         /* not reference an info-block but just */
	    p4_header_in(CURRENT);        /* the p4code directly */
	    FX_COMMA ( *SP ); 
	    P4_INC(SP,p4cell);
	    break;
	case p4_DVAR:
//          FX_RUNTIME_HEADER;
            p4_header_in(CURRENT); P4_NAMEFLAGS(LATEST) |= P4xISxRUNTIME;
	    FX_RUNTIME1_RT (pf_dictvar);
	    FX_COMMA (*SP++);
	    break;
	case p4_DCON:
//          FX_RUNTIME_HEADER;
            p4_header_in(CURRENT); P4_NAMEFLAGS(LATEST) |= P4xISxRUNTIME;
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
            P4_NAMEFLAGS(LATEST) |= P4xIMMEDIATE;
	____;
    } /* for w in ws->w */

    CURRENT = save_current; /* should save_current moved to the caller? */
    LATEST = CURRENT->link;
    ____;
}


/* -------------------------------------------------------------- */
/**
 * make a new dictionary entry in the word list identified by wid 
 *                   ( TODO: delete the externs in other code portions)
 * This function is really ifdef'd a lot because every implementation
 * needs to be (a) fast because it is used heavily when loading a forth
 * script and (b) robust to bad names like non-ascii characters and (c)
 * each variant has restrictions on header field alignments.
 * 
 */
p4char* p4_header_comma (const p4char *name, int len, p4_Wordl *wid)
{
    /* move exception handling to the end of this word - esp. nametoolong */
    if (len == 0)
        p4_throw (P4_ON_ZERO_NAME);
    
#   define CHAR_SIZE_MAX      ((1 << CHAR_BIT)-1)
    if (len > NAME_SIZE_MAX || len > CHAR_SIZE_MAX)
    {
	    pf_outf("\nERROR: name too long '%.*s'", len, name);
	    p4_throw (P4_ON_NAME_TOO_LONG);
    }

    if (redefined_msg && search_thread (name, len, wid->link, wid ))
        pf_outf ("\n\"%.*s\" is redefined ", len, name);

    /* and now, do the p4_string_comma ... */
# if defined PFE_WITH_FFA
    /* for the FFA style we have to insert a flag byte before the 
     * string that might be HERE via a WORD call. However that makes
     * the string to move UP usually - so we have to compute the overall 
     * size of the namefield first and its gaps, then move it */ 
    DP += 2; DP += len; FX (pf_align); 
    memmove (DP-len, name, len); /* i.e. #define NFA2LFA(p) (p+1+*p) */
    LATEST = DP-len -1;      /* point to count-byte before the name */
    *LATEST = len;           /* set the count-byte */
    LATEST[-1] = '\x80';     /* set the flag-byte before the count-byte */
# else
    /* traditional way - avoid copying if using WORD. Just look for the
     * only if() in this code which will skip over the memcpy() call if
     * WORD $HEADER, was called. At the same time we do not look for any
     * overlaps - when memcpy runs lower-to-upper address then this is
     * okay with strings shortened at HERE - but there *are* rare cases 
     * that this could fail. That's the responsibility of the user code
     * to avoid this by copying into a scratch pad first. Easy I'd say.
     */
    LATEST = DP++;
    if (name != DP) memcpy(DP, name, len);
    *LATEST = len;
    *LATEST |= '\x80'; 
    DP += len; FX (pf_align); 
# endif

    FX_PCOMMA (wid->link); /* create the link field... */
    //wid->thread[hc] = LATEST;
    wid->link = LATEST;
    return LATEST;
}

/* -------------------------------------------------------------- */
p4char* p4_header_in (p4_Wordl* wid)
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

/* -------------------------------------------------------------- */
/** WORD ( delimiter-char# -- here* ) [ANS]
 * read the next => SOURCE section (thereby moving => >IN ) up
 * to the point reaching $delimiter-char - the text is placed
 * at => HERE - where you will find a counted string. You may
 * want to use => PARSE instead.
 */
char* pf_word ( char del )
{
    char *here = (char *)DP;
    char *p;
    int   n;
    pf_skip_spaces();
    pf_parse_word(del);
    p = (char *)PFE.word.ptr;
    *here++ = (char)PFE.word.len;
    for ( n = PFE.word.len; n>0; n-- )
        *here++ = *p++;
    *here = 0; // zero-terminated string
    return (char *)DP;
}

/* -------------------------------------------------------------- */
char* pf_string ( void )
{
    char *here = (char *)DP;
    char *str = here;
    int   n;
    char *p = (char *)PFE.word.ptr;
    *here++ = (char)PFE.word.len;
    for ( n = PFE.word.len; n>0; n-- )
        *here++ = *p++;
    *here++ = 0; // zero-terminated string
    PFE.dp = (p4_byte_t *)here;
    return str;
}

/** ( --- addr )
 */
void pf_convert_string(void)
{
    if (STATE)
    {
//        FX_COMPILE (pf_convert_string);
	FX (pf_parse_comma_quote);
    }else{
	*--SP = (p4cell)pf_string();
    }
}

/* -------------------------------------------------------------- */
/**
 * the => INTERPRET as called by the outer interpreter
    char *q=source;
    int   n = length;
    int   i = TO_IN;

    PFE.word.ptr = (p4_char_t*) q + i;
 */
void pf_interpret(char *buf, int len)
{
    redefined_msg = 1;
    source = buf;
    length = len;
//             PFE.tib = buf;
    TO_IN = 0;
    while( TO_IN < length ) {
	/* the parsed string is in PFE.word.ptr / PFE.word.len,
	 * and by setting the HERE-string to length null, THROW
	 * will not try to report it but instead it prints PFE.word.
	 */
        pf_skip_spaces();
        if ( *(source + TO_IN) == '"' ) {
            TO_IN++;
            pf_parse_word('"');
            pf_convert_string();
            continue;
        }
        pf_parse_word(' ');
	*DP = 0; /* PARSE-WORD-NOHERE */
	if (PFE.word.len>0) {
	    if (pf_find_word())
		   continue;
	    if (pf_convert_number())
                   continue;
            p4_throw (P4_ON_UNDEFINED);
	}
    }
}

/* -------------------------------------------------------------- */
void pf_include(const char *name, int len)
{
    char buffer[256];
//printf("include %s\n",name);
    FILE *fh = fopen( name, "r" );
    if( fh != NULL ) {
        //while( fgets( (char *)PFE.tib, TIB_SIZE, fh ) != NULL ) {
        while( fgets( buffer, 255, fh ) != NULL ) {
             pf_interpret(buffer, strlen(buffer));
        }
        fclose( fh );
    } else {
        printf( "File %s not found.\n", name );
    }
}

/** INCLUDE ( "filename" -- ??? ) [FTH]
 * load the specified file, see also => LOAD" filename"
 */
FCode (pf_include)
{
    const char *fn = pf_word (' ');
    char len = *fn++;
    pf_include( fn, len );
}

/** INCLUDED ( name-ptr name-len -- ) [ANS]
 * open the named file and then => INCLUDE-FILE
 * see also the interactive => INCLUDE
 */
FCode (pf_included)
{
    const char *fn = (const char *) SP[1];	/* c-addr, name */
    int  len = SP[0];		/* length of name */
    SP += 2;
    pf_include( fn, len );
}

/* -------------------------------------------------------------- */
/*

<# # #>  #S  >IN  '  (
BASE CONVERT >NUMBER
FIND
HERE  HOLD PAD  SIGN  
. .(

not implemented
>BODY
-TRAILING
DECIMAL HEX
DEFINITIONS  
FORGET  FORTH  
U.
#TIB TIB  WORD 

not here
U.
ABORT QUIT

Forth200x
This standard removes six words that were marked 'obsolescent' in the ANS Forth '94 document. These are:
6.2.0060	#TIB		6.2.1390	EXPECT		6.2.2240	SPAN
6.2.0970	CONVERT		6.2.2040	QUERY		6.2.2290	TIB

Words affected:
#TIB, CONVERT, EXPECT, QUERY, SPAN, TIB, WORD.
Reason:
Obsolescent words have been removed.
Impact:
WORD is no longer required to leave a space at the end of the returned string.
It is recommended that, should the obsolete words be included, they have the behaviour described in
Forth 94. The names should not be reused for other purposes.
Transition/Conversion:
The functions of TIB and #TIB have been superseded by SOURCE.
The function of CONVERT has been superseded by >NUMBER.
The functions of EXPECT and SPAN have been superseded by ACCEPT.
The function of QUERY may be performed with ACCEPT and EVALUATE.


*/

P4_LISTWORDS (interpret) =
{
//    P4_INTO ("FORTH", 0),
    P4_DVaR ("BASE",         base),
    P4_FXco ("SOURCE",       pf_source),
    P4_FXco ("LATEST",       pf_latest),
    P4_FXco ("SIGN",         pf_sign),
    P4_FXco ("<#",           pf_less_sh),
    P4_FXco ("#",            pf_sh),
    P4_FXco ("#>",           pf_sh_greater),
    P4_FXco ("#S",           pf_sh_s),
    P4_FXco ("'",            pf_tick),
    P4_FXco (">NUMBER",      pf_to_number),
    P4_FXco ("FIND",         pf_find),
    P4_FXco ("HERE",         pf_here),
    P4_FXco ("HOLD",         pf_hold),
    P4_FXco ("PAD",          pf_pad),

    P4_FXco (".",            pf_dot),
    P4_SXco (".\"",          pf_dot_quote),
    P4_IXco ("(",            pf_paren),
    P4_IXco ("\\",           pf_backslash),
    P4_FXco ("INCLUDE",	     pf_include),
    P4_FXco ("INCLUDED",     pf_included),
};
P4_COUNTWORDS (interpret, "Interpreter words");

