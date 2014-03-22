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

#define P4WLIST(SET)  SET ## _LTX_p4_WLIST
#define P4WORDS(SET)  SET ## _LTX_p4_WORDS

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

typedef struct p4_Wordl  p4_Wordl;  /* body of a word list */
typedef struct p4_File 	 p4_File;   /* neccessary information about open files */
typedef struct p4_Except p4_Except; /* an exception frame */

struct p4_Wordl			/* a word list */
{				/* a word list identifier is a (Wordl *) */
    p4char* thread[THREADS];	/* field of ptrs to first NFA of chain */
    p4_Wordl *prev;		/* word lists are chained */
    p4_Wordl *also;		/* search also this dictionary, kinda FIG*/
    p4_namebuf_t* nfa;          /* corresponding vocabulary NFA */
    p4cell  flag;		/* do to_upper, a copy of sys.world_flag */
#define WORDL_NOCASE     (1<<0) /* FIND will match any case with any case */
#define WORDL_NOHASH     (1<<1) /* WORDLIST not hashed into multiple buckets */
#define WORDL_CURRENT    (1<<2) /* on new WORDLIST, make it chain to CURRENT */
#define WORDL_UPPER_CASE (1<<3) /* FIND will match uppercased entries too */
#define WORDL_UPPER_DEFS (1<<4) /* new definitions are placed uppercase */
    p4ucell id;                 /* used to speed up order search (p4_find) */
};

/* ---------------------- Decomp support -------------------- */

typedef struct p4_Decomp p4_Decomp; /* informations for the decompiler */
typedef struct p4_Semant p4_Semant; /* pointer set for state smart words */
typedef struct p4_Seman2 p4_Seman2; /* dito for even smarter words like TO */

#define P4_CODE_SEE(func) p4xcode* func (p4xcode* ip, char* p, p4_Semant* s)
#define P4_CODE_RUN(func) p4xcode* func (char* p, p4xt xt, p4char* nfa)

/* encodings for what information follows the compiled word inline */
#define  P4_SKIPS_NOTHING            ((P4_CODE_SEE((*)))(0))
#define  P4_SKIPS_OFFSET             ((P4_CODE_SEE((*)))(1))
#define  P4_SKIPS_CELL               ((P4_CODE_SEE((*)))(2))
#define  P4_SKIPS_DCELL              ((P4_CODE_SEE((*)))(3))
#define  P4_SKIPS_STRING             ((P4_CODE_SEE((*)))(5))
#define  P4_SKIPS_2STRINGS           ((P4_CODE_SEE((*)))(6))
#define  P4_SKIPS_TO_TOKEN           ((P4_CODE_SEE((*)))(7))

/* .... P4_SKIPS_DCELL */
_extern P4_CODE_SEE(p4_lit_dcell_SEE);
/* .... P4_SKIPS_STRING */
_extern P4_CODE_SEE(p4_lit_string_SEE);
/* .... P4_SKIPS_2STRINGS */
_extern P4_CODE_SEE(p4_lit_2strings_SEE);
/* .... P4_SKIPS_TO_TOKEN */
_extern P4_CODE_SEE(p4_lit_to_token_SEE);

struct p4_Decomp		/* informations for the decompiler */
{                               /* (skips is now basically enum'd, see above)*/
    P4_CODE_SEE((*skips));      /* to decompile the data following xt */
    unsigned space:3;		/* additional spaces past the word */
    unsigned cr_bef:2;		/* carriage return before printing */
    signed ind_bef:4;		/* changed indentation before print */
    unsigned cr_aft:2;		/* carriage return after print */
    signed ind_aft:4;		/* changed indentation after print */
    unsigned unused:3;
};

struct p4_Semant		/* for words with different compilation */
{				/* and execution semantics: */
    long magic;			/* mark begin of structure */
    p4_Decomp decomp;		/* decompiler aid */
    p4_namebuf_t const *name;	/* compiled by */
    p4code comp;		/* compilation/interpretation semantics */
    p4code exec[1];		/* execution semantics */
};

struct p4_Seman2		/* for words with different compilation */
{				/* and two different execution semantics: */
    long magic;			/* mark begin of structure */
    p4_Decomp decomp;		/* decompiler aid */
    p4_namebuf_t const *name;	/* compiled by */
    p4code comp;		/* compilation/interpretation semantics */
    p4code exec[2];		/* two different execution semantics */
};				/* for cases like TO (value/local variable) */




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
#define p4_XXCO 'x'     /* CX */ /* auto-init */
#define p4_RTCO 'r'     /* RT */ /* creates a word with special runtime */
 
#define p4_OVAR 'v'     /* OV */ /* ordinary variable */
#define p4_IVAR 'V'     /* IV */
#define p4_OCON 'c'     /* OC */ /* ordinary constant */
#define p4_ICON 'C'     /* IC */
#define p4_OVAL 'l'     /* OL */ /* ordinary value */
#define p4_IVAL 'L'     /* IL */

/* -def p4_DVAH 'h'     // DW */ /* dict chainvar (threaded) [REMOVED] */
#define p4_DVAR 'm'     /* DV */ /* dict variable (threaded) */
#define p4_DCON 'n'     /* DC */ /* dict constget (threaded) */
/* -def p4_DSET 't'     // DS */ /* dict valueset (threaded) [REMOVED] */
#define p4_OFFS 'o'     /* offset word */

#define p4_SNYM 'Y'     /* immediate synonym */
#define p4_FNYM 'y'     /*  ordinary synonym */
#define p4_iOLD 'Z'     /* immediate synonym of obsolete word */
#define p4_xOLD 'z'     /*  ordinary synonym of obsolete word */

#define p4_OVOC 'w'     /* explicitly create a vocabulary (normal) */
#define p4_IVOC 'W'     /* explicitly create a vocabulary (immediate) */

#define p4_NEED 'q'     /* issue a NEEDS check - which may load a module */
#define p4_INTO 'i'     /* make sure the vocabulary exists, and put all */
                        /* of the following words into it, and with nonzero */
                        /* arg, make sure the voc is in the default search */ 
                        /* order (so they are visible after load) */
#define p4_LOAD 'I'     /* load the referenced wordset, now, recursively */
#define p4_EXPT 'e'     /* set an exception-id and descriptive string */

#define p4_SHOW 'd'     /* show one-time message notes to the user */
#define p4_STKi 'M'     /* stackhelp info for static tracing/checking */
#define p4_STKx 'N'     /* stackhelp info for static tracing/checking */

#define p4_ITEM '@'     /* Runtime item (e.g. compiled by RTCO word) */
#define p4_DTOR '~'     /* Destroyer item (e.g. compiled by forget_word) */
#define p4_NEST '_'     /* NestedCall item (e.g. compiled by : ... ; ) */

/* return the byte offset of a given component to beginning of structure: */
#define OFFSET_OF(T,C)	((char *)&(((T *)0)->C) - (char *)0)

/* macros to build entries in the wordlists: 
 * until all sematic-words have a proper name along, we need to help
 * the decompiler here to print the name. Since PFE uses %.*s to print,
 * it is okay to the upper bound of the length as a dummy count byte,
 * which is 0xFF (0377) for immediate words and 0x9F (0237) for the others.
 */

#define P4_FXCO( NM, PCODE)     { "p\237"NM, &P4CODE (PCODE) }
#define P4_IXCO( NM, PCODE)     { "P\377"NM, &P4CODE (PCODE) }
#define P4_SXCO( NM, SEM)       { "X\377"NM, (p4code)&P4SEMANTICS(SEM) }
#define P4_XXCO( NM, PCODE)     { "x\237"NM, &P4CODE (PCODE) } /* AUTO-INIT */
#define P4_RTCO( NM, RUN)       { "r\237"NM, (p4code)&P4RUNTIME_(RUN) }

#define P4_FXco( NM, PCODE)     { "p\237"NM, &P4CODE (PCODE) }
#define P4_IXco( NM, PCODE)     { "P\377"NM, &P4CODE (PCODE) }
#define P4_SXco( NM, SEM)       { "X\377"NM, (p4code)&P4SEMANTICS(SEM) }
#define P4_XXco( NM, PCODE)     { "x\237"NM, &P4CODE (PCODE) } /* AUTO-INIT */
#define P4_RTco( NM, RUN)       { "r\237"NM, (p4code)&P4RUNTIME_(RUN) }

#define P4_OVAR( NM)            { "v\237"NM, ((p4code)0) }
#define P4_OCON( NM, VAL)       { "c\237"NM, (p4code)(VAL) }
#define P4_OVAL( NM, VAL)       { "l\237"NM, (p4code)(VAL) }
#define P4_OFFS( NM, VAL)       { "o\237"NM, (p4code)(VAL) }

#define P4_IVAR( NM)            { "V\377"NM, ((p4code)0) }
#define P4_ICON( NM, VAL)       { "C\377"NM, (p4code)(VAL) }
#define P4_IVAL( NM, VAL)       { "L\377"NM, (p4code)(VAL) }

#define P4_OVaR( NM)            { "v\237"NM, ((p4code)0) }
#define P4_OCoN( NM, VAL)       { "c\237"NM, (p4code)(VAL) }
#define P4_OVaL( NM, VAL)       { "l\237"NM, (p4code)(VAL) }
#define P4_OFFs( NM, VAL)       { "o\237"NM, (p4code)(VAL) }

#define P4_IVaR( NM)            { "V\377"NM, ((p4code)0) }
#define P4_ICoN( NM, VAL)       { "C\377"NM, (p4code)(VAL) }
#define P4_IVaL( NM, VAL)       { "L\377"NM, (p4code)(VAL) }

/* -def      VO( NM, LIST)      { "\112"NM, (p4code)(&P4WORDS(LIST,WList)) } */

#define P4_DVAR( NM, MEMBER)    { "m\237"NM, (p4code)OFFSET_OF(p4_Thread, MEMBER) }
#define P4_DCON( NM, MEMBER)    { "n\237"NM, (p4code)OFFSET_OF(p4_Thread, MEMBER) }
/* -def P4_DSET( NM, MEMBER)    { "t\237"NM, (p4code)OFFSET_OF(p4_Thread, MEMBER) } */

/* -def P4_DVaH( NM, MEMBER)    { "h\237"NM, (p4code)OFFSET_OF(p4_Thread, MEMBER) } */
#define P4_DVaR( NM, MEMBER)    { "m\237"NM, (p4code)OFFSET_OF(p4_Thread, MEMBER) }
#define P4_DVaL( NM, MEMBER)    { "n\237"NM, (p4code)OFFSET_OF(p4_Thread, MEMBER) }
/* -def P4_DSeT( NM, MEMBER)    { "t\237"NM, (p4code)OFFSET_OF(p4_Thread, MEMBER) } */

#define P4_SNYM( NM, OLDNAME)   { "Y\377"NM, (p4code)OLDNAME }
#define P4_FNYM( NM, OLDNAME)   { "y\237"NM, (p4code)OLDNAME }
#define P4_iOLD( NM, OLDNAME)   { "Z\377"NM, (p4code)OLDNAME }
#define P4_xOLD( NM, OLDNAME)   { "z\237"NM, (p4code)OLDNAME }

#define P4_OVOC( NM, CHAIN)     { "w\237"NM, (p4code)(CHAIN) } 
#define P4_IVOC( NM, CHAIN)     { "W\237"NM, (p4code)(CHAIN) } 
#define P4_NEED( NM)            { "q\237"NM }
#ifndef __WATCOMC__
#define P4_INTO( NM, DESCR)     { "i\377"NM, (p4code)(DESCR) } /*search-also*/
#else
#define P4_INTO( NM, DESCR)     { "i\377"NM, 0 } /*search-also*/
#endif
#define P4_LOAD( NM, WORDS)     { "I\377"NM, (p4code)(&P4WORDS(WORDS)) }

#define P4_EXPT( NM, ID)        { "e\237"NM, (p4code)(ID) } /*exception*/
#define P4_SHOW( NM, ID)        { "d\337"NM, (p4code)(ID) } /*message*/

#define P4_STKi( NM, INFO)      { "M\377"NM, (p4code)(INFO) } /*stackhelp*/
#define P4_STKx( NM, PCODE)     { "N\377"NM, &P4CODE(PCODE) } /*stackhelp*/

#ifdef _P4_SOURCE
/* macros to build entries in the wordlists: */

#define CO(NM,PCODE)    { "p\237"NM, &P4CODE (PCODE) }
#define CI(NM,PCODE)    { "P\377"NM, &P4CODE (PCODE) }
#define CS(NM,SEM)      { "X\377"NM, (p4code)&P4SEMANTICS(SEM) }
#define CX(NM,PCODE)    { "x\237"NM, &P4CODE (PCODE) } /* AUTO-INIT */

#define OV(NM)          { "v\237"NM, ((p4code)0) }
#define OC(NM,VAL)      { "c\237"NM, (p4code)(VAL) }
#define OL(NM,VAL)      { "l\237"NM, (p4code)(VAL) }
#define OT(NM,VAL)      { "o\237"NM, (p4code)(VAL) }

#define IV(NM)          { "V\377"NM, (p4code)0) }
#define IC(NM,VAL)      { "C\377"NM, (p4code)(VAL) }
#define IL(NM,VAL)      { "L\377"NM, (p4code)(VAL) }

/* #define VO(NM,LIST)   { "\112"NM, (p4code)(&P4WORDS(LIST,WList)) } */

# if defined __IBMC__ && defined __OS2__
  /* IBM C Set/2 thinks OFFSET_OF is no constant expr */
# define DV(NM,MEMBER)  { "m\237"NM, (p4code)do_##MEMBER }
# define DC(NM,MEMBER)  { "n\237"NM, (p4code)do_##MEMBER }
# else
# define DV(NM,MEMBER)  { "m\237"NM, (p4code)OFFSET_OF(p4_Thread, MEMBER) }
# define DC(NM,MEMBER)  { "n\237"NM, (p4code)OFFSET_OF(p4_Thread, MEMBER) }
# endif

#endif

/*@}*/
#endif 

