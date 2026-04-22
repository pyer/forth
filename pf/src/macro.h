#ifndef __PF_MACRO_H                              /* -*- width: 100 -*- */
#define __PF_MACRO_H

/** 
 * -- macro definitions for the portable forth environment
 * 
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *  Copyright (C) Pierre Bazonnard 2013 - 2014.
 *
 */

/* either have a seperate Flag-Field-Area before name or use flags
 * integrated in the (hi bits of the) count-byte of a bstring */
# if defined PF_WITH_FFA
#   define P4_NAMEFLAGS(X)   (((p4char*)X)[-1]) /* == (*P4_NFA2FLAGS(X)) */
#   define P4_NAMESTART(X)  (&((p4char*)X)[-1]) /* NFA -> FFA w/ FFA-byte */
#   define P4_NAME_MASK_LEN(X)  (X)
#   define NAME_SIZE_MAX     127                 /* C99 defines SIZE_MAX for size_t */
# else
#   define P4_NAMEFLAGS(X) (*(p4char*)X)        /* == (*P4_NFA2FLAGS(X)) */
#   define P4_NAMESTART(X)  ((p4char*)X)        /* NFA -> FFA w/o FFA-byte */
#   define P4_NAME_MASK_LEN(X)  ((X)&31)            /* NFA -> count of namefield */
#   define NAME_SIZE_MAX     31                  /* used for buffer-sizes */
# endif

#   define NAMEPTR(X)   (((p4char*)(X))+1)
#   define NAMELEN(X)   P4_NAME_MASK_LEN(*(p4char*)X)

/* useful shortcuts */
# define WP_PFA  ((p4cell *)&PFE.wp [1]) 

#define FX_SKIP_STRING  (*(char **)&(IP) += (pf_aligned (*(p4char*)IP + 1)))

/* P:dictpointer X:value Y:hintchar T:typedef */
#define P4_COMMA_(P,X,Y,T) (*(T *)(P) = (T)(X), ((*(T **)&(P))++))

#define FX_COMMA(X)      P4_COMMA_(DP,(X), 0 ,p4cell)
#define FX_FCOMMA(X)     P4_COMMA_(DP,(X),'F',double)
#define FX_XCOMMA(X)     P4_COMMA_(DP,(X),'X',p4xt)
#define FX_ZCOMMA(X)     P4_COMMA_(DP,(X),'Z',p4xt)
#define FX_RCOMMA(X)     P4_COMMA_(DP,(X),'R',p4code)
#define FX_PCOMMA(X)     P4_COMMA_(DP,(X),'P',void*)
#define FX_QCOMMA(X)     P4_COMMA_(DP,(X),'Q',void*)
#define FX_BCOMMA(X)     P4_COMMA_(DP,(X),'B',unsigned char)
#define FX_WCOMMA(X)     P4_COMMA_(DP,(X),'W',unsigned short)
#define FX_LCOMMA(X)     P4_COMMA_(DP,(X),'L',p4ucell)
#define FX_UCOMMA(X)     P4_COMMA_(DP,(X),'U',p4ucell)
#define FX_VCOMMA(X)     P4_COMMA_(DP,(X),'V',p4ucell)
#define FX_SCOMMA(X)     P4_COMMA_(DP,(X),'S',p4cell)

/* typed comma:
   X = exectoken = pointer to pointer to C-routine =~ pointer to code-field
   R = runtime = pointer to C-routine = code-field (needs .so relocation!)
   P = pointer into the dictionary
   Q = pointer into compiled colon word =~ P (i.e. HERE)
   C = character =~ byte-wide number
   S = singlecell-wide (signed) number or item of cell-element stringspan
   U = singlecell-wide (unsigned) number or item of cell-element stringspan
   D = doublecell (signed) number or item of doubecell-element stringspan
   B = byte-wide number or item of byte-element stringspan
   W = word-wide number or item of word-element stringspan
   L = long-wide number or item of long-element stringspan
   F = floating-pointer number
   I = bitfield (e.g. flags-byte of headers)
   Z = pointer to runtime-address in C-datafield =~ X (e.g. semantics)
   V = value, probably a number but can not be sure, must better check
   ... bigcaps for the start of the element ... lowcaps for extensions ...
*/

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
 
/* macros to build entries in the wordlists: 
 * until all sematic-words have a proper name along, we need to help
 * the decompiler here to print the name. Since PFE uses %.*s to print,
 * it is okay to the upper bound of the length as a dummy count byte,
 * which is 0xFF (0377) for immediate words and 0x9F (0237) for the others.
 */
#define P4_FXco( NM, PCODE)     { "p\237"NM, &P4CODE (PCODE) }
#define P4_IXco( NM, PCODE)     { "P\377"NM, &P4CODE (PCODE) }
#define P4_SXco( NM, SEM)       { "X\377"NM, (p4code)&SEM##_Semant }
#define P4_RTco( NM, RUN)       { "r\237"NM, (p4code)&RUN##_Runtime }

#define P4_CONSTANT( NM, VAL)   { "c\237"NM, (p4code)(VAL) }

/* return the byte offset of a given component to beginning of structure: */
#define OFFSET_OF(T,C) ((char *)&(((T *)0)->C) - (char *)0)
#define P4_VARIABLE( NM, VAL)   { "v\237"NM, (p4code)OFFSET_OF(p4_Thread, VAL) }
//#define P4_VARIABLE( NM, VAL)   { "v\237"NM, (p4code)((char *)&(((p4_Thread *)0)->VAL) - (char *)0) }

#ifndef P4_MAX_FILES /* maximum number of open files */
#define P4_MAX_FILES 0x10
#endif

/*
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
  
#endif 
