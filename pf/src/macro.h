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
# if defined PFE_WITH_FFA
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

#   define NAMEPTR(X)   (((p4_namebuf_t*)(X))+1)
#   define NAMELEN(X)   P4_NAME_MASK_LEN(*(p4_namebuf_t*)X)

/* useful shortcuts */
#define P4_INC(P,T)	((*(T **)&(P))++)

# define WP_CFA		(PFE.wp)		 
# define WP_NFA		(pf_to_name(PFE.wp))
# define WP_PFA		((p4cell *)&PFE.wp [1]) 

#define FX_SKIP_STRING  (*(char **)&(IP) += (pf_aligned (*(p4char*)IP + 1)))

/* P:dictpointer X:value Y:hintchar T:typedef */
#define	P4_COMMA_(P,X,Y,T) (*(T *)(P) = (T)(X), P4_INC (P, T))

#define P4_COMMA(P,X,Y)  P4_COMMA_(P,X,Y,p4cell)
#define P4_BCOMMA(P,X)   P4_COMMA_(P,X,0,unsigned char)
#define P4_WCOMMA(P,X)   P4_COMMA_(P,X,0,unsigned short)
#define P4_LCOMMA(P,X)   P4_COMMA_(P,X,0,p4ucell)
#define P4_PCOMMA(P,X)   P4_COMMA_(P,X,0,void*)

#define	FX_COMMA(X)	 P4_COMMA(p4_DP,X,0)
//#define	FX_COMMA_(X,Y)	 P4_COMMA(p4_DP,X,Y)

#define FX_FCOMMA(X)	 P4_COMMA_(p4_DP,(X),'F',double)
#define FX_XCOMMA(X)     P4_COMMA_(p4_DP,(X),'X',p4xt)
#define FX_ZCOMMA(X)     P4_COMMA_(p4_DP,(X),'Z',p4xt)
#define FX_RCOMMA(X)     P4_COMMA_(p4_DP,(X),'R',p4code)
#define FX_PCOMMA(X)     P4_COMMA_(p4_DP,(X),'P',void*)
#define FX_QCOMMA(X)     P4_COMMA_(p4_DP,(X),'Q',void*)
#define FX_BCOMMA(X)     P4_COMMA_(p4_DP,(X),'B',unsigned char)
#define FX_WCOMMA(X)     P4_COMMA_(p4_DP,(X),'W',unsigned short)
#define FX_LCOMMA(X)     P4_COMMA_(p4_DP,(X),'L',p4ucell)
#define FX_UCOMMA(X)     P4_COMMA_(p4_DP,(X),'U',p4ucell)
#define FX_VCOMMA(X)     P4_COMMA_(p4_DP,(X),'V',p4ucell)
#define FX_SCOMMA(X)     P4_COMMA_(p4_DP,(X),'S',p4cell)

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

#ifndef P4_NO_FP
#define P4_IF_FP_(X) X
#else
#define P4_IF_FP_(X)
#endif

#endif 
