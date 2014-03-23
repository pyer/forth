#ifndef __DICTIONARY_H
#define __DICTIONARY_H

/** 
 *  Implements dictionary and wordlists.
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *  Copyright (C) Pierre Bazonnard 2013 - 2014.
 *
 *  @see     GNU LGPL
 */

#define ORDER_LEN 64            /* maximum wordlists in search order */

#define CONTEXT	(PFE.context)
#define ONLY	(PFE.context[ORDER_LEN])
#define CURRENT	(PFE.current)

/* you must differentiate between VAR-style body and DOES-style body */
#define P4_TO_LINK(C)     ((p4char**)(C) -1 )
#define P4_LINK_FROM(C)   ((p4xt)(C) + 1 )
#define P4_TO_BODY(C)     ((p4cell *)((p4xt)(C) + 1))
#define P4_BODY_FROM(P)   ((p4xt)((p4cell *)(P) - 1))
#define P4_TO_DOES_BODY(C)  ((p4cell *)((p4xt)(C) + 2))
#define P4_TO_DOES_CODE(C)  ((p4xcode **)((p4xt)(C) + 1))

#ifdef PFE_CALL_THREADING
extern const p4xcode* p4_to_code (p4xt xt);
# define P4_TO_CODE(C)     (p4_to_code((p4xt)C))
#else
# define P4_TO_CODE(C)     (C)
#endif

p4char ** p4_topmost (p4_Wordl *w);

void pf_dot_name (const p4char *nfa);
p4cell * pf_to_body (p4xt xt);

p4char** pf_name_to_link (const p4char* p);
p4char * pf_to_name (p4xt xt);
p4xt pf_name_from (const p4char *p);

void p4_preload_only (void);
void p4_load_words (const p4Words* ws, p4_Wordl* wid, int unused);

int p4_wl_hash (const p4_char_t *s, int l);

#endif
