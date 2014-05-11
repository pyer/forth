/**
 * PFE-DEBUG --- analyze compiled code
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.15 $
 *     (modified $Date: 2008-05-11 12:48:04 $)
 *
 *  @description
 *	The Portable Forth Environment provides a decompiler for
 *      colon words and a single stepper for debugging. After 
 *      setting a breakpoint at a word saying => DEBUG <tt>word</tt>.
 *  	The next time the <tt>word</tt> gets executed the single
 * 	stepper takes control.
 *
 * 	When this happens you see the top stack items displayed in one
 *	line. The topmost stack item is the first in line, the second and
 *	following stack items are displayed throughout the end of line.
 *	This line is empty if the stack is empty when the word in question
 *	executes.
 *
 *	On the next line you see the first word to become executed inside
 *	the debugged <tt>word</tt>. There is a prompt <tt>&gt;</tt> to
 *	the right of the displayed word. At this prompt you have several
 *	options. Choose one by typing a key (<tt>[h]</tt> shows helpscreen):
 *
 *	<dl>
 *	<dt> <tt>[enter], [x], [k], [down]</tt> </dt>  <dd>
 *	The displayed word will be executed without single stepping.
 *	Note that the execution of the word is slowed down a little
 *	compared to execution outside the single stepper. This is
 *	because the single stepper has to keep control to detect when
 *	the word has finished.
 *
 *	After the actual word finished execution the resulting stack
 *	is printed on the current line. The next line shows the next
 *	word to become executed.
 *
 *	Having repeated this step several times, you can see to the
 *	the right of every decompiled word what changes to the stack
 *	this word caused by comparing with the stack display just
 *	one line above.
 *      </dd>
 *	<dt> <tt>[d], [l], [right]</tt> </dt><dd>
 *	Begin single step the execution of the actual word. The first
 *	word to become executed inside the definition is displayed on
 *	the next line. The word's display is intended by two spaces
 *	for each nesting level.
 * 
 *   	You can single step through colon-definitions and the children
 *	of defining words. Note that most of the words in PFE are
 *	rewritten in C for speed, and you can not step those kernel
 *	words.
 *      </dd>
 *      <dt> <tt>[s], [j], [left]</tt> </dt><dd>
 *	Leaves the nesting level. The rest of the definition currently
 *	being executed is run with further prompt. If you leave the
 *	outmost level, the single stepper won't get control again.
 *	Otherwise the debugger stops after the current word is
 *	finished and offers the next word in the previous nesting level.
 *	</dd>
 *	<dt> <tt>[space]</tt> </dt><dd>
 *	The next word to be executed is decompiled. This should help 
 *	to decide as if to single step that word.
 *	</dd>
 *	<dt> <tt>[q]</tt> </dt><dd>
 *	Quits from the debugger. The execution of the debugged word is
 *	not continued. The stacks are not cleared or changed.
 *	</dd>
 *	<dt> <tt>[c]</tt> </dt><dd>
 *	Displays the profiling instruction counter.
 *	<dt> <tt>[r]</tt> </dt><dd>
 *	Reset the instruction counter, to profile some code. The
 *	debugger counts how often the inner interpreter i.e. how
 *	many Forth-primitives are executed. Use this option to 
 *      reset the counter to 0 to measure an arbitrary part of code.
 *	</dd>
 *	</dl>
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <setjmp.h>
#include <errno.h>

#include "config.h"
#include "types.h"
#include "const.h"
#include "macro.h"
#include "listwords.h"
#include "thread.h"

#include "compiler.h"
#include "exception.h"
#include "interpret.h"
#include "terminal.h"

#define ___ {
#define ____ }

#define PF_SKIP_STRING (*(char **)&(ip) += (pf_aligned (*(p4char*)ip + 1)))
/* ----------------------------------------------------------------------- */

int debugging = 0;
int level;
int maxlevel;
long opcounter ;

/* ----------------------------------------------------------------------- */
FCode (pf_debug_colon_RT);
FCode (pf_debug_colon);
FCode (pf_debug_does_RT);
FCode (pf_debug_does);

FCode_RT (pf_defer_RT);

p4_Semant * p4_to_semant (p4xt xt);

/* ----------------------------------------------------------------------- */
/* debug-ext: we walk registered wordsets looking for
 * items that can be used for decompiling. No need to register each
 * item in the forth dictionary anymore. Here we define a walker struct
 * that can be referenced in decompiler-routines in the wordsets around
 */

typedef struct p4_Decompile p4_Decompile;
struct p4_Decompile
{
    p4char* next;            /* initially set to LATEST */
    char const* wordset;     /* current wordset reference, the name info */ 
    int left;                /* how many left to check: */
    struct {                 /* compatible with p4Words !! */
	struct{
	    const char type;    /* loader-type */
	    const char lencode; /* forth-name compatibility */
	    const char name[1]; /* zero-terminated... */
	} *loader;
	union {                 /* the value part from the wordset item: */
	    const p4cell cell;             /* as type p4cell */
	    void * ptr;                    /* generic pointer */
	    p4_Runtime2 const * runtime;   /* runtime reference */
	    p4_Seman2   const * semant;    /* semant reference */
	    char* name;                    /* name, zero-terminated */
	} value;
    } *word;
};

/* ----------------------------------------------------------------------- */

/* LOADER refers to the compiled WORDLIST tables at the end of C modules.
 * Each loader is registered in the extension wordlist so that we simply
 * walk the wordlist looking for those entries. If a loader table entry has 
 * been found then we can simply iterate through the element list. Each
 * element has really just a name and a value. For primitives the value is
 * simply the code pointer but other types may refer to an info block.
 */

FCode_RT (pf_forget_wordset_RT)
{
    /* do nothing so far, P4CODE(forget_wordset_RT) is just a type-marker */
}

static const char* p4_loader_next_wordset (p4_Decompile* decomp)
{
    p4xt xt;

    do {
//printf("decomp->next=%lx\n",decomp->next);
	if (! decomp->next) return 0;
	xt = name_to_cfa (decomp->next);
	decomp->next = *CFA_TO_LINK(xt);
    } while (*xt != P4CODE(pf_forget_wordset_RT));
puts("decomp->word");
    /* assert xt is wordset_RT */
    /* FIXME: forget-layout? BODY[0] has the value? */
    ___ p4Words* ws = *(p4Words**) P4_TO_BODY(xt); 
    decomp->left = ws->n;
    decomp->word = (void*) ws->w;
    decomp->wordset = ws->name;
    ____;
    return decomp->wordset;
}
    
static p4_char_t p4_loader_next (p4_Decompile* decomp)
{

    if (! decomp->word) /* after first initializing a decomp-struct */
	goto nothing_left;
    decomp->word ++;
    if (! -- decomp->left)
	goto nothing_left;
 next_loader:
    if (! *(void**) decomp->word->loader)
	goto nothing_left;
    return decomp->word->loader->type;
 nothing_left:
puts("before");
    if (! p4_loader_next_wordset (decomp))
	return '\0';
puts("after");
    if (! decomp->left) 
	goto nothing_left;
    goto next_loader;
}

/************************************************************************/
/* decompiler                                                           */
/************************************************************************/

typedef p4xt* (*func_SEE) (p4xt* , char*, p4_Semant*);

p4xt* pf_literal_SEE (p4xt* ip, char* p, p4_Semant* s)
{
    char buf[80];
    if (s) 
    {
        if (s->name && ! memcmp (s->name+1, "LITERAL", 7)) /* 'bit fuzzy... */
            sprintf (p, "0x%lX ", (unsigned long) *(p4ucell*)ip);
        else
            sprintf (p, "( %.*s) 0x%lX ", 
              NAMELEN(s->name), NAMEPTR(s->name), (unsigned long) *(p4ucell*)ip);
    }else{
        strcpy (p, p4_str_dot (*(p4ucell *) ip, buf + sizeof buf, BASE));
    }
    return ++ip;
}

p4xt* p4_lit_to_token_SEE (p4xt* ip, char* p, p4_Semant* s)
{
    register p4xt xt = ip[-1];
    if (*xt == s->exec[0])
    {
	register p4char* itemnfa;
        xt = *ip++;
        itemnfa = cfa_to_name (xt);
        sprintf (p, "%.*s %.*s ", 
          NAMELEN(s->name), NAMEPTR(s->name),
          NAMELEN(itemnfa), NAMEPTR(itemnfa));
        { /* make-recognition, from yours.c */
            if (s->decomp.space > 1) ip++;
            if (s->decomp.space > 2) ip++;
        }
        return ip;
    }else{
        sprintf (p, "%.*s <%c> ", 
          NAMELEN(s->name), NAMEPTR(s->name),
          'A' - 1 + (int) *(p4cell *) ip);
        { /* make-recognition, from yours.c */
            if (s->decomp.space > 1) ip++;
            if (s->decomp.space > 2) ip++;
        }
        return ++ip;
    }
}

p4xt* p4_lit_string_SEE (p4xt* ip, char* p, p4_Semant* s)
{
    sprintf (p, "%.*s %.*s\" ",
      NAMELEN(s->name), NAMEPTR(s->name),
      (int) *(p4char *) ip, (p4char *) ip + 1);
    PF_SKIP_STRING;
    return ip;
}

p4xt* p4_lit_2strings_SEE (p4xt* ip, char* p, p4_Semant* s)
{
    p4char *s1 = (p4char *) ip;
    PF_SKIP_STRING;
    sprintf (p, "%.*s %.*s %.*s ",
      NAMELEN(s->name), NAMEPTR(s->name), (int) *s1, s1 + 1,
      (int) *(p4char *) ip, (p4char *) ip + 1);
    PF_SKIP_STRING;
    return ip;
}

static P4_CODE_RUN(p4_code_RT_SEE)
{
    sprintf(p, "CODE %.*s ", NAMELEN(nfa), NAMEPTR(nfa));
    ___ p4xt* ip = (p4xt*) P4_TO_BODY(xt);
    return ip;
    ____;
}

static const p4_Decomp default_style = {P4_SKIPS_NOTHING, 0, 0, 0, 0, 0};

static p4xt * p4_decompile_comma (p4xt* ip, char *p)
{
    p4char* x = (p4char*) ip;
    sprintf (p, "$%02x C, ", *x); ++x;
    return (p4xt*) (x);
}

static p4xt * p4_decompile_code (p4xt* ip, char *p, p4_Decomp *d)
{
    memcpy (d, (& default_style), sizeof (*d));
    return p4_decompile_comma (ip, p);
}

static p4xt * p4_decompile_word (p4xt* ip, char *p, p4_Decomp *d)
{
    /* assert SKIPS_NOTHING == 0 */
    register p4xt xt = *ip++;
    register p4_Semant *s;

    s = (p4_Semant*) p4_to_semant (xt);
    memcpy (d, ((s) ? (& s->decomp) : (& default_style)), sizeof(*d));

    /* some tokens are (still) compiled without a semant-definition */
    if (*xt == P4CODE (pf_literal_execution))
        return pf_literal_SEE (ip, p, s);
/*
    if (*xt == P4CODE (p4_locals_bar_execution))
        return p4_locals_bar_SEE (ip, p, s);
    if (*xt == P4CODE (p4_local_execution))
        return p4_local_SEE (ip, p, s);
*/
    if (d->skips == P4_SKIPS_CELL 
      || d->skips == P4_SKIPS_OFFSET)
    {
        ((*(p4cell **)&(ip))++);
        sprintf (p, "%.*s ", NAMELEN(s->name), NAMEPTR(s->name));
        return ip;
    }

//    if (d->skips == P4_SKIPS_DCELL)
//        return p4_lit_dcell_SEE (ip, p, s);
    if (d->skips == P4_SKIPS_STRING)
        return p4_lit_string_SEE (ip, p, s);
    if (d->skips == P4_SKIPS_2STRINGS)
        return p4_lit_2strings_SEE (ip, p, s);
    if (d->skips == P4_SKIPS_TO_TOKEN)
        return p4_lit_to_token_SEE (ip, p, s);

    /* per default, just call the skips-decomp routine */
    if (d->skips) /* SKIPS_NOTHING would be NULL */
	return (*d->skips)(ip, p, s);

    if (s != NULL)
    {
        /* use the semant-name (or compiled-by name) */
        sprintf (p, "%.*s ", NAMELEN(s->name), NAMEPTR(s->name));
        return ip;
    }else{
	/* the prim-name (or colon-name) */
        register p4char* nfa = cfa_to_name (xt);
        sprintf (p, (P4_NAMEFLAGS(nfa) & P4xIMMEDIATE) ? "POSTPONE %.*s " : "%.*s ",
		 NAMELEN(nfa), NAMEPTR(nfa));
        return ip;
    }
}

void p4_decompile_rest (p4xt *ip, int nl, int indent, p4_bool_t iscode)
{
    char buf[256];
    /* p4_Seman2 *seman; // unused ? */
    p4_Decomp decomp;
    *buf = '\0';
    
    FCode (pf_more);
    for (;;)
    {
        if (!*ip) break;
        /* seman = (p4_Seman2 *) p4_to_semant (*ip); // unused ? */
        if (iscode) 
        {
            p4xt* old_ip = ip;
            ip = p4_decompile_code (ip, buf, &decomp);
            if (! strcmp (buf, "] ;") ) 
            {
                strcpy(buf, "END-CODE ");
            } else if (! strncmp (buf, "] ", 2)) {
                static const p4_Decomp call_style = {P4_SKIPS_NOTHING, 0, 1, 0, 1, 0};
                /* if not STC then show just as comment - each decompiled code
                 * will then be presented on a seperate line - on x86 ITC just try
                 *   CODE uu $90 c, $e8 c, ' DUP @ HERE cell+ - , END-CODE
                 *   SEE uu \ results in ->
                 *   CODE uu      $90 C, 
                 *       ( DUP ) $e8 C, $ec C, $ff C, $ff C, $ff C, 
                 *       END-CODE
                 *  and the execution of uu will actually perform a DUP ( $90 is NOP )
                 */
                char* append = strchr(buf, '\0');
                *buf = '('; strcpy (append, ") ");
                memcpy (& decomp, & call_style, sizeof(decomp));
                while (old_ip < ip) {
                    append = strchr(append, '\0');
                    old_ip = p4_decompile_comma(old_ip, append);
                }
            }
        } else 
        {
            ip = p4_decompile_word (ip, buf, &decomp);
        }
        indent += decomp.ind_bef;
        if ((!nl && decomp.cr_bef) || get_outs() + strlen (buf) >= get_cols())
	{
            if (pf_more_Q())
                break;
            nl = 1;
	}
        if (nl)
	{
            pf_emits (indent, ' ');
            nl = 0;
	}
        pf_outs (buf);
        pf_emits (decomp.space, ' ');
        indent += decomp.ind_aft;
        if (decomp.cr_aft)
	{
            if (pf_more_Q())
                break;
            nl = 1;
	}
        if (decomp.cr_aft > 2)  /* instead of exec[0] == P4CODE(semicolon_execution) */
            break;
    }
}

static P4_CODE_RUN(pf_colon_RT_SEE)
{
    strcat (p, ": ");
    strncat (p, (char*) NAMEPTR(nfa), NAMELEN(nfa));
    strcat (p, "\n");
    return (p4xt*) cfa_to_body (xt);
}

static P4_CODE_RUN(pf_does_RT_SEE)
{
    strcat (p, "CREATE ");
    strncat (p, (char*) NAMEPTR(nfa), NAMELEN(nfa));
    strcat (p, " ( ALLOT )");
    return (*P4_TO_DOES_CODE(xt))-1;
}

static void print_comment (const char* prefix, const char* wordset)
{
    char* end = strchr (wordset, ' ');
    pf_outs ("  ( ");
    if (! prefix) prefix = "";

    if (! end) { 
	pf_outf ("%s%s", prefix, wordset); }
    else { 
	pf_outf ("%s%.*s", prefix, (int)(end - wordset), wordset); }
    pf_outs (" Word ) ");
}

void p4_decompile (p4_namebuf_t* nfa, p4xt xt)
{
    char buf[256];
    register p4xt* rest = 0;
    p4_bool_t iscode = P4_FALSE;
    *buf = '\0';
    FX (pf_cr);
//puts("decompile");

    if (     *xt == P4CODE(pf_colon_RT) || 
	     *xt == P4CODE(pf_debug_colon_RT))
    { rest = pf_colon_RT_SEE(buf,xt,nfa); goto decompile; }
    else if (*xt == P4CODE(pf_does_RT)|| 
	     *xt == P4CODE(pf_debug_does_RT))
    { rest = pf_does_RT_SEE(buf,xt,nfa); goto decompile; }

    if (*xt == (p4code) P4_TO_BODY(xt)) {
        iscode = P4_TRUE;
        rest = p4_code_RT_SEE(buf,xt,nfa); goto decompile;
    }
    
    /* new variant: we walk the atexit-list looking for WORDSET 
     * registerations. We walk each entry in the wordset looking for
     * RTco items and comparing their values with what we have as CODE(xt).
     * When there is a SEE decompile-routine registered, then we use it.
     */
    ___ auto p4_Decompile decomp = {}; decomp.next = LATEST;
    while (p4_loader_next (&decomp))
    {
	switch (decomp.word->loader->type)
	{
	case p4_RTCO:
	    if (*xt != decomp.word->value.runtime->exec[0])
		continue;
	    /* we have it! */
	    if (decomp.word->value.runtime->see)
	    {
		rest = decomp.word->value.runtime->see (buf,xt,nfa);
		if (rest) goto decompile;
		pf_outs (buf); pf_outs (" ");
	    }else
	    {
		pf_outf (buf, "%s %.*s ", decomp.word->loader->name,
			 (int) NAMELEN(nfa), NAMEPTR(nfa));
	    }
	    pf_outs ((P4_NAMEFLAGS(nfa) & P4xIMMEDIATE) ? " IMMEDIATE " : "        ");
	    print_comment ("From ", decomp.wordset);
	    return;
	case p4_FXCO:
	case p4_IXCO:
	    if (*xt != (p4code) decomp.word->value.ptr)
	    	continue;
	    pf_dot_name (nfa);
	    pf_outs ((P4_NAMEFLAGS(nfa) & P4xIMMEDIATE) ? " IMMEDIATE " : "        ");
	    print_comment ("A Prim ", decomp.wordset);
	    goto primitive;
	case p4_SXCO:
	    if (*xt != (p4code) decomp.word->value.semant->comp)
		continue;
	    pf_dot_name (nfa);
	    pf_outs (" ..."); 
	    if (strchr (decomp.word->loader->name, '"'))
		pf_outc ('"');
	    pf_outs ((P4_NAMEFLAGS(nfa) & P4xIMMEDIATE) ? " IMMEDIATE " : "        ");
	    print_comment ("A Smart ", decomp.wordset);
	    goto primitive;
	default:
	    continue;
	} /* switch */
	pf_outs (buf); pf_outs (" ");
	return;
    };____; /* nothing found */
/* else: */
    pf_dot_name (nfa);
    if (P4_NAMEFLAGS(nfa) & P4xIMMEDIATE)
    	pf_outs ("is IMMEDIATE ");
    else
    	pf_outs ("is prim CODE ");
    if (P4xISxRUNTIME)
        if (P4_NAMEFLAGS(nfa) & P4xISxRUNTIME)
    	    pf_outs ("RUNTIME ");
 primitive:
    return;

 decompile:
    /* assert (*buf) */
    pf_outs (buf); pf_outs (" ");
    if (rest) 
	p4_decompile_rest (rest , 1, 4, iscode);
    if (P4_NAMEFLAGS(nfa) & P4xIMMEDIATE)
	pf_outs (" IMMEDIATE ");
    return;
}

/************************************************************************/
/* debugger                                                             */
/************************************************************************/

static void prompt_col (void)
{
    pf_emits (24 - get_outs(), ' ');
}

static void display (p4xt *ip)
{
    p4_Decomp style;
    char buf[80];
    int indent = maxlevel * 2;
    int depth = S0 - SP, i;

    prompt_col ();
    for (i = 0; i < depth; i++)
    {
        pf_outf ("%10ld ", (long) SP[i]);
        if (get_outs() + 11 >= get_cols())
            break;
    }
    FX (pf_cr);
    p4_decompile_word (ip, buf, &style);
    pf_outf ("%*s%c %s", indent, "", pf_category (**ip), buf);
}

static void interaction (p4xt *ip)
{
    int c;

    for (;;)
    {
        display (ip);
        
        prompt_col ();
        pf_outs ("> ");
        c = pf_getkey ();
        pf_outs ("\r");
        if (isalpha (c))
            c = tolower (c);

        switch (c)
	{
         default:
             pf_bell ();
             continue;
//         case P4_KEY_kr:
         case 'd':
         case 'l':
             maxlevel++;
             return;
//         case P4_KEY_kd:
         case '\r':
         case '\n':
         case 'k':
         case 'x':
             return;
//         case P4_KEY_kl:
         case 's':
         case 'j':
             maxlevel--;
             return;
         case 'q':
             pf_outf ("\nQuit!");
             debugging = 0;
             p4_throw (P4_ON_QUIT);
         case ' ':
             switch (pf_category (**ip))
             {
              default:
                  p4_decompile (cfa_to_name (*ip), *ip);
                  break;
              case ':':
                  FX (pf_cr);
                  p4_decompile_rest ((p4xt *) cfa_to_body (*ip), 1, 4, P4_FALSE);
                  break;
              case 'D':
                  pf_outs ("\nDOES>");
                  p4_decompile_rest ((p4xt *) (*ip)[-1], 0, 4, P4_FALSE);
                  break;
             }
             FX (pf_cr);
             continue;
         case 'r':
             opcounter = 0;
             pf_outf ("\nOperation counter reset\n");
             continue;
         case 'c':
             pf_outf ("\n%ld Forth operations\n", opcounter);
             continue;
         case 'h':
         case '?':
             pf_outf ("\nDown,  'x', 'k', CR\t" "execute word"
               "\nRight, 'd', 'l'\t\t" "single step word"
               "\nLeft,  's', 'j'\t\t" "finish word w/o single stepping"
               "\nSpace\t\t\t" "SEE word to be executed"
               "\n'C'\t\t\t" "display operation counter"
               "\n'R'\t\t\t" "reset operation counter"
               "\n'Q'\t\t\t" "QUIT"
		"\n'?', 'H'\t\t" "this message"
               "\n");
             continue;
	}
    }
}

static void do_adjust_level (const p4xt xt)
{
    if (*xt == P4CODE(pf_colon_RT) ||
	*xt == P4CODE(pf_debug_colon_RT) ||
	*xt == P4CODE(pf_does_RT) ||
	*xt == P4CODE(pf_debug_does_RT))
        level++;
    else if (*xt == P4CODE (pf_semicolon_execution))
        level--;
}

static void pf_debug_execute (p4xt xt)
{
    do_adjust_level (xt);
    pf_normal_execute (xt);
}

static void pf_debug_on (void)
{
    debugging = 1;
    opcounter = 0;
    PFE.execute = pf_debug_execute;
    level = maxlevel = 0;
    pf_outf ("\nSingle stepping, type 'h' or '?' for help\n");
}

static void pf_debug_off (void)
{
    debugging = 0;
    PFE.execute = pf_normal_execute;
}

static void			/* modified inner interpreter for */
do_single_step (void)		/* single stepping */
{
    while (level >= 0)
    {
        if (level <= maxlevel)
	{
            maxlevel = level;
            interaction (IP);
	}
        do_adjust_level (*IP);
        opcounter++;
        WP = *IP++;	/* ip and W are same: register or not */
        (*WP) ();
    }
}

FCode (pf_debug_colon_RT)
{
    FCode (pf_colon_RT);
    if (!debugging)
    {
        pf_debug_on ();
        do_single_step ();
        pf_debug_off ();
    }
}
FCode (pf_debug_colon) { /* dummy */ }
P4RUNTIME1(pf_debug_colon, pf_debug_colon_RT);

FCode (pf_debug_does_RT)
{
    FCode (pf_does_RT);
    if (!debugging)
    {
        pf_debug_on ();
        do_single_step ();
        pf_debug_off ();
    }
}
FCode (pf_debug_does) { /* dummy */ }
P4RUNTIME1(pf_debug_does, pf_debug_does_RT);

/** DEBUG ( "word" -- ) [FTH]
 * this word will place an debug-runtime into
 * the => CFA of the following word. If the
 * word gets executed later, the user will
 * be prompted and can decide to single-step
 * the given word. The debug-stepper is
 * interactive and should be self-explanatory.
 * (use => NO-DEBUG to turn it off again)
 */
FCode (pf_debug)
{
    p4xt xt = pf_tick_cfa ();
    if (P4_XT_VALUE(xt) == FX_GET_RT (pf_debug_colon) 
      || P4_XT_VALUE(xt) == FX_GET_RT (pf_debug_does))
        return;
    else if (P4_XT_VALUE(xt) == FX_GET_RT (pf_colon))
        P4_XT_VALUE(xt) = FX_GET_RT (pf_debug_colon);
    else if (P4_XT_VALUE(xt) == FX_GET_RT (pf_does))
        P4_XT_VALUE(xt) = FX_GET_RT (pf_debug_does);
    else
        p4_throw (P4_ON_ARG_TYPE);
}

/** NO-DEBUG ( "word" -- ) [FTH]
 * the inverse of " => DEBUG word "
 */
FCode (pf_no_debug)
{
    p4xt xt = pf_tick_cfa ();
    if (P4_XT_VALUE(xt) == FX_GET_RT (pf_debug_colon))
        P4_XT_VALUE(xt) = FX_GET_RT (pf_colon);
    else if (P4_XT_VALUE(xt) == FX_GET_RT (pf_debug_does))
        P4_XT_VALUE(xt) = FX_GET_RT (pf_does);
    else
        p4_throw (P4_ON_ARG_TYPE);
}

/** SEE ( "word" -- )
 *  tries to display the word as a reasonable indented source text
 *  or displays the type of word.
 */
FCode (pf_see)
{
    p4char *nfa = pf_tick_nfa();
    p4xt cfa = name_to_cfa(nfa);
    printf("\n%.*s is a ", NAMELEN(nfa), NAMEPTR(nfa));
    char c = pf_show_category(*cfa);
    if ( c==':' || c=='c' || c=='D' || c=='d' )
       p4_decompile (nfa, cfa);
    FX (pf_cr);
}

P4_LISTWORDS (debug) =
{
    P4_FXco ("DEBUG",        pf_debug),
    P4_FXco ("NO-DEBUG",     pf_no_debug),
    P4_FXco ("SEE",          pf_see),
};
P4_COUNTWORDS (debug, "Debugger words");

