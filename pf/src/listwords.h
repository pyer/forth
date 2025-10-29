#ifndef __PF_LISTWORDS_H
#define __PF_LISTWORDS_H
/** 
 * -- types, codes and macros for symbol table load
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2003.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.5 $
 *     (modified $Date: 2008-09-11 01:27:20 $)
 *
 *  @description
 *         here are the macros that are used to build the various
 *         symbols tables, including wordsets.
 */

typedef struct                  /* describe a word for registration */
{                               /* in the dictionary */
    const char *name;           /* name of word */
    p4code ptr;                 /* code it executes or something */
}                               /* according to spc@pineal.math.fau.edu: */
p4Word;                         /* 'void *' doesn't work here on SGI */

typedef struct                  /* describes the set of words */
{                               /* a module loads into the dictionary */
    int n;                      /* how many words? */
    const p4Word *w;            /* pointer to vector of words */
    const char *name;           /* name of word set, or null */
}
p4Words;

/* options section */
#define THREADS_SHIFT 5
#define THREADS (1<<THREADS_SHIFT)

#ifndef P4_THREADS_SHIFT /* 2^n number of threads in a word list */
#define P4_THREADS_SHIFT 5
#endif

#ifndef P4_MAX_FILES	/* maximum number of open files */
#define P4_MAX_FILES	0x10
#endif

/* some constants needed here, they are dependent of options */
#define P4_THREADS (1<<P4_THREADS_SHIFT)


typedef p4char p4_namechar_t;      /* word list name char */
typedef p4char p4_namebuf_t;       /* start of counted string with namechars */
typedef p4char p4_charbuf_t;       /* start of counted string with i/o chars */
/*   */        /* char */          /* i/o char of Standard C/C++ (compiler) */

typedef p4cell  (*p4cell_p4code) (void); /* very useful sometimes */
typedef p4ucell (*p4ucell_p4code) (void); /* very useful sometimes */

/* ---------------------- Decomp support -------------------- */

typedef struct p4_Decomp p4_Decomp; /* informations for the decompiler */
typedef struct p4_Semant p4_Semant; /* pointer set for state smart words */

#define P4_CODE_SEE(func) p4xt* func (p4xt* ip, char* p, p4_Semant* s)
#define P4_CODE_RUN(func) p4xt* func (char* p, p4xt xt, p4char* nfa)

/* encodings for what information follows the compiled word inline */
#define  P4_SKIPS_NOTHING            ((P4_CODE_SEE((*)))(0))
#define  P4_SKIPS_OFFSET             ((P4_CODE_SEE((*)))(1))
#define  P4_SKIPS_CELL               ((P4_CODE_SEE((*)))(2))
#define  P4_SKIPS_DCELL              ((P4_CODE_SEE((*)))(3))
#define  P4_SKIPS_STRING             ((P4_CODE_SEE((*)))(5))
#define  P4_SKIPS_2STRINGS           ((P4_CODE_SEE((*)))(6))
#define  P4_SKIPS_TO_TOKEN           ((P4_CODE_SEE((*)))(7))

/* .... P4_SKIPS_DCELL */
P4_CODE_SEE(p4_lit_dcell_SEE);
/* .... P4_SKIPS_STRING */
P4_CODE_SEE(p4_lit_string_SEE);
/* .... P4_SKIPS_2STRINGS */
P4_CODE_SEE(p4_lit_2strings_SEE);
/* .... P4_SKIPS_TO_TOKEN */
P4_CODE_SEE(p4_lit_to_token_SEE);

struct p4_Semant		/* for words with different compilation */
{				            /* and execution semantics: */
    long magic;			/* mark begin of structure */
    P4_CODE_SEE((*skips));    /* to decompile the data following xt */
    p4_namebuf_t const *name;	/* compiled by */
    p4code comp;		          /* compilation/interpretation semantics */
    p4code exec[1];		        /* execution semantics */
};

struct p4_Runtime2              /* describes characteristics of CFA code */
{
    long magic;                 /* mark begin of structure */
    p4cell flag;                /* the call-threading flags for the exec[]s */
    char const *name;           /* the header name for it */
    p4code comp;             /* the word that will CREATE new headers */
    p4code exec[2];          /* and the values contained in created CFAs */
};

typedef struct p4_Runtime2 p4_Runtime2; /* and also for the CFA themselves */

#define P4WLIST(SET)  SET ## _LTX_p4_WLIST
#define P4WORDS(SET)  SET ## _LTX_p4_WORDS

#define P4_LISTWORDS( SET )  \
    static const p4Word P4WLIST(SET)[]

# define P4_COUNTWORDS(SET,NAME) \
    const p4Words P4WORDS(SET) = \
    {                           \
        DIM (P4WLIST (SET)),    \
        P4WLIST (SET),          \
        NAME                    \
    }
  
/* Encoding the kind of definition i.e. which runtime to fill into the cfa. 
 * 
 * the original pfe had 3 (or 4?) levels of load-tables.
 * the definitions above define a two level approach.
 * the following definitions define a one level approach,
 * and up to now, no heritage-defines are given.
 *
 * each uppercase-name is an immediate word, all others are not.
 */
 
#define p4_FXCO 'p'     /* CO */ /* ordinary primitive (code) */
#define p4_IXCO 'P'     /* CI */ 
#define p4_SXCO 'X'     /* CS */ /* smart-word (semant) */
#define p4_RTCO 'r'     /* RT */ /* creates a word with special runtime */
 
#define p4_OCON 'c'     /* OC */ /* ordinary constant */
#define p4_OVAL 'l'     /* OL */ /* ordinary value */
#define p4_OVAR 'v'     /* OV */ /* ordinary variable */

#define p4_DCON 'd'     /* DV */ /* dict constant (threaded) */
#define p4_DVAL 'm'     /* DV */ /* dict value (idem constant) */
#define p4_DVAR 'w'     /* DV */ /* dict variable (threaded) */

/* return the byte offset of a given component to beginning of structure: */
#define OFFSET_OF(T,C)	((char *)&(((T *)0)->C) - (char *)0)

/* macros to build entries in the wordlists: 
 * until all sematic-words have a proper name along, we need to help
 * the decompiler here to print the name. Since PFE uses %.*s to print,
 * it is okay to the upper bound of the length as a dummy count byte,
 * which is 0xFF (0377) for immediate words and 0x9F (0237) for the others.
 */
#define P4_FXco( NM, PCODE)     { "p\237"NM, &P4CODE (PCODE) }
#define P4_IXco( NM, PCODE)     { "P\377"NM, &P4CODE (PCODE) }
#define P4_SXco( NM, SEM)       { "X\377"NM, (p4code)&P4SEMANTICS(SEM) }
#define P4_RTco( NM, RUN)       { "r\237"NM, (p4code)&P4RUNTIME_(RUN) }

#define P4_OCON( NM, VAL)       { "c\237"NM, (p4code)(VAL) }

#define P4_DCON( NM, VAL)       { "d\237"NM, (p4code)OFFSET_OF(p4_Thread, VAL) }
#define P4_DVAL( NM, VAL)       { "m\237"NM, (p4code)OFFSET_OF(p4_Thread, VAL) }
#define P4_DVAR( NM, VAL)       { "w\237"NM, (p4code)OFFSET_OF(p4_Thread, VAL) }

#endif 
