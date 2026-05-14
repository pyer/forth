#ifndef __PF_MACRO_H                              /* -*- width: 100 -*- */
#define __PF_MACRO_H

/** 
 * -- macro definitions for the portable forth environment
 * 
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *  Copyright (C) Pierre Bazonnard 2013 - 2026.
 *
 */

/* declare a primitive */
#define FCode(X) void X##_(void)

/* either have a seperate Flag-Field-Area before name or use flags
 * integrated in the (hi bits of the) count-byte of a bstring */
# if defined PF_WITH_FFA
#   define NAMEFLAGS(X)   (*((char*)(X)-1))    /* FFA is before NFA */
#   define NAMEPTR(X)     (((char*)(X))+1)
#   define NAMELEN(X)     (*(char*)X)
#   define NAME_SIZE_MAX     127                 /* C99 defines SIZE_MAX for size_t */
# else
#   define NAMEFLAGS(X)   (*(char*)X)          /* FFA is the hi bits of NFA */
#   define NAMEPTR(X)     (((char*)(X))+1)
#   define NAMELEN(X)     ((*(char*)X)&31)
#   define NAME_SIZE_MAX     31                  /* used for buffer-sizes */
# endif


#define FX_SKIP_STRING  (*(char **)&(IP) += (pf_aligned (*(char*)IP + 1)))

/* X:value T:typedef */
#define P4_COMMA_(X,T) (*(T *)(DP) = (T)(X), ((*(T **)&(DP))++))

#define FX_PCOMMA(X)     P4_COMMA_((X),void*)  // pointer into the dictionary
#define FX_RCOMMA(X)     P4_COMMA_((X),p4code) // runtime = pointer to C-routine = code-field (needs .so relocation!)
#define FX_SCOMMA(X)     P4_COMMA_((X),p4cell) // singlecell-wide (signed) number or item of cell-element stringspan
#define FX_XCOMMA(X)     P4_COMMA_((X),p4xt)   // exectoken = pointer to pointer to C-routine =~ pointer to code-field


/* Encoding the kind of definition i.e. which runtime to fill into the cfa. 
 * 
 * the original pfe had 3 (or 4?) levels of load-tables.
 * the definitions above define a two level approach.
 * the following definitions define a one level approach,
 * and up to now, no heritage-defines are given.
 *
 * each uppercase-name is an immediate word, all others are not.
 */
 
/* macros to build entries in the wordlists: 
 * until all sematic-words have a proper name along, we need to help
 * the decompiler here to print the name. Since PFE uses %.*s to print,
 * it is okay to the upper bound of the length as a dummy count byte,
 * which is 0xFF (0377) for immediate words and 0x9F (0237) for the others.
 */
#define P4_FXco( NM, PCODE)     { "p\237"NM, &PCODE##_ }
#define P4_IXco( NM, PCODE)     { "P\377"NM, &PCODE##_ }
#define P4_RTco( NM, RUN)       { "r\237"NM, (p4code)&RUN##_Runtime }
#define P4_SXco( NM, SEM)       { "X\377"NM, (p4code)&SEM##_Semant }

#define P4_CONSTANT( NM, VAL)   { "c\237"NM, (p4code)(VAL) }
#define P4_END                  { "E\377", 0 }

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

#define WORDS(set) const p4Word set##_WORDS[]

#endif 
