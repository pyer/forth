/**
 * --  Subroutines for the Internal Forth-System
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.13 $
 *     (modified $Date: 2008-05-11 03:37:57 $)
 */
/*@{*/
#if defined(__version_control__) && defined(__GNUC__)
static char* id __attribute__((unused)) =
"@(#) $Id: engine-sub.c,v 1.13 2008-05-11 03:37:57 guidod Exp $";
#endif

#define _P4_SOURCE 1

#include <pfe/pfe-base.h>
#include <pfe/def-limits.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <pfe/os-string.h>

#include <pfe/double-sub.h>
#include <pfe/debug-ext.h>
#include <pfe/floating-ext.h>
#include <pfe/file-sub.h>
#include <pfe/term-sub.h>
#include <pfe/_missing.h>
#include <pfe/exception-sub.h>

#include <pfe/logging.h>

FCode(p4_noop)
{
    /* well, nothing... */
}


/* **********************************************************************
 * inner and outer interpreter
 */

#ifndef PFE_SBR_CALL_THREADING

/**
 * longjmp via (Except->jmp) following inline
 * - purpose: stop the inner interpreter
 */
FCode_XE (p4_call_stop)
{   FX_USE_CODE_ADDR {
    p4_Except *buf = (p4_Except *) *IP;

# ifdef P4_REGRP		/* save global register variables */
    buf->rpp = RP;
# endif
# ifdef P4_REGSP
    buf->spp = SP;
# endif
# ifdef P4_REGLP
    buf->lpp = LP;
# endif
# ifndef P4_NO_FP
# ifdef P4_REGFP
    buf->fpp = FP;
# endif
# endif
    p4_longjmp (buf->jmp, 1);
    /*FX_USE_CODE_EXIT;*/
}}

#endif /* ! SBR_THREADING */

/**
 * Run a forth word from within C-code
 * - this is the inner interpreter
 */
_export void
p4_call_loop (p4xt xt)
{
#if defined PFE_SBR_CALL_THREADING
    p4_sbr_call (xt);
    return;
#else

    p4_Except stop;

#  if ! defined PFE_CALL_THREADING
    static p4code call_stop = PFX (p4_call_stop);
    p4xcode list[3];
    list[0] = xt;
    list[1] = &call_stop;
    list[2] = (p4xcode) &stop;
#  else
    /* sbr-stub, xt-code, xt-data, sbr-stub, jump-code, jump-data */
    p4xcode list[6] /* = { 0,0,0,0,0,0 } */ ;
    *p4_compile_xcode(
	p4_compile_comma(
	    list,
	    xt),
	PFX(p4_call_stop)) = (p4xcode) &stop;
#  endif

    IP = list;
#  if !defined P4_WP_VIA_IP && !defined PFE_CALL_THREADING
    p4WP = *IP;
#  endif

    if (p4_setjmp (stop.jmp))
    {
#     ifdef P4_REGRP		/* restore global register variables */
        RP = stop.rpp;		/* clobbered by longjmp() */
#     endif
#     ifdef P4_REGSP
        SP = stop.spp;
#     endif
#     ifdef P4_REGLP
        LP = stop.lpp;
#     endif
#     ifndef P4_NO_FP
#     ifdef P4_REGFP
        FP = stop.fpp;
#     endif
#     endif
        return;
    }

    /* next_loop */
    for (;;)
    {
#    ifdef PFE_CALL_THREADING
#      define NVAR register p4code c;
#      define NEXT c = *IP++, (c)()
#    elif defined P4_WP_VIA_IP
#      define NVAR register p4xt w;
#      define NEXT w = *IP++, (*w) ()	/* ip is register but p4WP isn't */
#    else
#      define NVAR
#      define NEXT p4WP = *IP++, (*p4WP) ()
				/* ip and p4WP are same: register or not */
#    endif

        NVAR;
#     ifdef UNROLL_NEXT		/* USER-CONFIG: if it helps */
        NEXT; NEXT; NEXT; NEXT;	/* do a little loop unrolling for speed */
        NEXT; NEXT; NEXT; NEXT;
#     else
        NEXT;			/* on some machines it doesn't do any good */
#     endif
    }

#endif /* ! PFE_SBR_CALL_THREADING */
}

/**
 */
_export void
p4_call (p4xt xt)
{
# if !defined PFE_SBR_CALL_THREADING
    p4xcode *saved_ip = IP;
    p4_call_loop (xt);
    IP = saved_ip;
# else
    p4_sbr_call (xt);
# endif
}

/**
 * the NEXT call. Can be replaced by p4_debug_execute to
 * trace the inner forth interpreter.
 */
_export void
p4_normal_execute (p4xt xt)
{
    p4_call(xt);
}

/**
 * quick execute - unsafe and slow and simple
 *
 * use this routine for callbacks that might go through some forth
 * colon-routines - that code shall not THROW or do some other nifty
 * tricks with the return-stack or the inner interpreter.
 * Just simple things - use only for primitives or colon-routines,
 * nothing curried with a DOES part in SBR-threading or sth. like that.
 */
_export void
p4_simple_execute (p4xt xt)
{
#  if defined PFE_SBR_CALL_THREADING /*FIXME: BODY / CODE ADDR needed? */
    (*p4_to_code(xt))();
#  elif defined PFE_CALL_THREADING
    P4_REGIP_T ip = IP;
    P4_REGRP_T rp = RP;
    p4xcode body = (p4xcode) P4_TO_BODY(xt);
    IP = &body; /* fake the body-field, just in case it is needed */
    (*p4_to_code(xt))();
    while (RP < rp) { NVAR; NEXT; }
    IP = ip;
#  else /* ITC: */
    P4_REGIP_T ip = IP;
    P4_REGRP_T rp = RP;
    IP = & xt;
    do { NVAR; NEXT; }  while (RP < rp);
    IP = ip;
#  endif
}

/* ================= INTERPRET =================== */

static p4ucell
FXCode (p4_interpret_find_word) /* hereclean */
{
    register p4char *nfa;
    register p4xt xt;

    /* WORD-string is at HERE and at PFE.word.ptr / PFE.word.len */
    nfa = p4_find (PFE.word.ptr, PFE.word.len);
    if (! nfa) return (p4cell) nfa; /* quick path, even alias null-return */

    xt = p4_name_from (nfa);
    if (! STATE || P4_NFA_xIMMEDIATE(nfa))
    {
	p4_call (xt);           /* execute it now */
	FX (p4_Q_stack);        /* check stack */
    }else{
	FX_COMPILE_COMMA (xt);  /* comma token */
    }
    return 1;
}
static FCode (p4_interpret_find_execution)
{
    FX_USE_CODE_ADDR;
    if (FX (p4_interpret_find_word)) FX_BRANCH; else FX_SKIP_BRANCH;
    FX_USE_CODE_EXIT;
}
FCode (p4_interpret_find)
{
    p4_Q_pairs (P4_DEST_MAGIC); /* BEGIN ... AGAIN */
    FX_COMPILE (p4_interpret_find);
    FX (p4_dup);
    FX (p4_backward_resolve);
    FX_PUSH (P4_DEST_MAGIC);
}
P4COMPILES (p4_interpret_find, p4_interpret_find_execution,
  P4_SKIPS_OFFSET, P4_NEW1_STYLE);
/** INTERPRET-FIND ( CS: dest* -- dest* ) executes ( -- ) experimental
 *  check the next word from => QUERY and try to look it up
 *  with => FIND - if found then execute the token right away
 *  and branch out of the loop body (usually do it => AGAIN )
 */

static p4ucell
FXCode (p4_interpret_number_word) /* hereclean */
{
    p4dcell d;

    /* WORD-string is at HERE and at PFE.word.ptr / PFE.word.len */
    if (! p4_number_question (PFE.word.ptr, PFE.word.len, &d))
	return 0; /* quick path */

    if (STATE)
    {
	if (p4_DPL >= 0)
	{
	    FX_COMPILE (p4_two_literal);
	    FX_COMMA_ (d.hi,'D');
            FX_COMMA_ (d.lo,'d');
	}else{
	    FX_COMPILE (p4_literal);
            FX_SCOMMA (d.lo);
	}
    }else{
	*--SP = d.lo;
	if (p4_DPL >= 0)
	    *--SP = d.hi;
    }
    return 1;
}
static FCode (p4_interpret_number_execution)
{
    FX_USE_CODE_ADDR;
    if (FX (p4_interpret_find_word)) FX_BRANCH; else FX_SKIP_BRANCH;
    FX_USE_CODE_EXIT;
}
FCode (p4_interpret_number)
{
    p4_Q_pairs (P4_DEST_MAGIC); /* BEGIN ... AGAIN */
    FX_COMPILE (p4_interpret_number);
    FX (p4_dup);
    FX (p4_backward_resolve);
    FX_PUSH (P4_DEST_MAGIC);
}
P4COMPILES (p4_interpret_number, p4_interpret_number_execution,
  P4_SKIPS_OFFSET, P4_NEW1_STYLE);
/** INTERPRET-NUMBER ( CS: dest* -- dest* ) executes ( -- ) experimental
 *  check the next word from => QUERY and try to parse it up
 *  with => ?NUMBER - if parseable then postpone the number for execution
 *  and branch out of the loop body (usually do it => AGAIN )
 */

static unsigned FXCode (p4_interpret_next_word)
{
    for (;;)
    {
	/* the parsed string is in PFE.word.ptr / PFE.word.len,
	 * and by setting the HERE-string to length null, THROW
	 * will not try to report it but instead it prints PFE.word.
	 */
	p4_word_parseword (' '); *DP = 0; /* PARSE-WORD-NOHERE */
	if (PFE.word.len) return PFE.word.len;

	switch (SOURCE_ID)
	{
	default:
	    if (p4_next_line ())
	    {
		PFE.last_here = PFE.dp;
		continue;
	    }
	case 0:
	case -1:
	    return 0;
	}
    }
}

static FCode (p4_interpret_loop)
{
    for (;;)
    {
    again:
	if (! FX (p4_interpret_next_word)) return;

	if (FX (p4_interpret_find_word)) goto again;
	if (FX (p4_interpret_number_word)) goto again;
        p4_throw (P4_ON_UNDEFINED);
    }
}

/**
 * the => INTERPRET as called by the outer interpreter
 */
FCode (p4_interpret)
{
    PFE.last_here = PFE.dp;
    FX (p4_interpret_loop);
}

/**
 * => INTERPRET buffer
 */
_export void
p4_evaluate (const p4_char_t *p, int n)
{
#  if !defined P4_RP_IN_VM
    Iframe saved;
    p4_link_saved_input (&saved);
#  else
    RP = (p4xcode **) p4_save_input (RP);
#  endif
    SOURCE_ID = -1;
    TIB = p;                /* leave that warning for a while... */
    NUMBER_TIB = n;
    TO_IN = 0;
    FX (p4_interpret);
#  if defined P4_RP_IN_VM
    RP = (p4xcode **) p4_restore_input (RP);
#  else
    p4_unlink_saved_input (&saved);
#  endif
}

/**
 */
_export void
p4_include_file (p4_File *fid)
{
    if (fid == NULL || fid->f == NULL)
        p4_throwstr (P4_ON_FILE_NEX, fid->name);
    else
    {
#      if !defined P4_RP_IN_VM
	Iframe saved;
	p4_link_saved_input (&saved);
#      else
	RP = (p4xcode **) p4_save_input (RP);
#      endif
	SOURCE_ID = (p4cell) fid;
	TO_IN = 0;
	FX (p4_interpret);
#      if defined P4_RP_IN_VM
	RP = (p4xcode **) p4_restore_input (RP);
#      else
	p4_unlink_saved_input (&saved);
#      endif
    }
}


/**
 * called by INCLUDED and INCLUDE
 */
_export int
p4_included1 (const p4_char_t *name, int len, int throws)
{
    File* f = p4_open_file (name, len, FMODE_RO);
    if (!f)
    {
        if (throws)
        {
            p4_throwstr (P4_ON_FILE_NEX, (const char *)name);
        }else{
            P4_fail1 ("- could not open '%s'\n", name );
            return 0;
        }
    }
    p4_include_file (f);
    p4_close_file (f);
    return 1;
}

/**
 * INCLUDED
 */
_export void
p4_included (const p4_char_t* name, int len)
{
    p4_included1 (name, len, 1);
}

/*
 */
_export void
p4_unnest_input (p4_Iframe *p)
{
    while (PFE.saved_input && PFE.saved_input != p)
    {
        switch (SOURCE_ID)
	{
        case -1:
        case 0:
            break;
        default:
            p4_close_file (SOURCE_FILE);
	}
#     if defined P4_RP_IN_VM
        RP = (p4xcode **) p4_restore_input (PFE.saved_input);
#     else
	p4_unlink_saved_input (PFE.saved_input);
#     endif
    }
}

/**
 * walk the filedescriptors and close/free the fds. This function
 * is usefully called from => ABORT - otherwise it may rip too
 * many files in use.
 */
FCode (p4_closeall_files)
{
    /*FIXME: look at p4_close_all_files, is it the same?? */
    File* f;

    /* see => p4_free_file_slot for an example */
    for (f = PFE.files; f < PFE.files_top; f++)
        if (f->f != NULL)
        {
            if (f->name && f->name[0] == '<')
                continue; /* stdIn, stdOut, stdErr, a.k.a. "<STDIN>" etc. */
            else
                p4_close_file(f);
        }
}

/* **********************************************************************
 *  QUIT, ABORT, INTERPRET
 */

/**
 * a little helper that just emits "ok", called in outer interpreter,
 * also useful on the command line to copy lines for re-execution
 */
FCode (p4_ok)
{
//    if (!STATE)
//    {
        p4_outs ("ok");
//    }
}

/*
 * things => QUIT has to initialize
 */
void quit_system (void)
{
#  ifdef P4_RP_IN_VM
    CSP = (p4cell*) RP;         /* come_back marker */
    RP = p4_R0;			/* return stack to its bottom */
#  endif
    LP = NULL;			/* including all local variables */
    STATE = P4_FALSE;		/* interpreting now */
    PFE.catchframe = NULL;	/* and no exceptions to be caught */
    p4_debug_off ();		/* turn off debugger */
}

/*
 * things => ABORT has to initialize
 */
void abort_system (void)
{
    SP = p4_S0;				/* stacks */
//    if (PFE.abort[2]) { (PFE.abort[2]) (FX_VOID); } /* -> floating */
//    if (PFE.abort[3]) { (PFE.abort[3]) (FX_VOID); } /* -> dstrings */
    if (p4_RESET_ORDER)  { FX (p4_reset_order); }   /* reset search order */
    FX (p4_decimal);			/* number i/o base */
    FX (p4_standard_io);		/* disable i/o redirection */
    FX (p4_closeall_files);             /* close open filedescriptors */
    if (PFE.dictlimit - PFE_MINIMAL_UNUSED > PFE.dp)
        return;
    else
    {
        P4_fail2 ("DICT OVER - reset HERE from %+li to %+li",
                  (long)(PFE.dp - PFE.dict),
		  (long)(PFE.last_here - PFE.dict));

        PFE.dp = PFE.last_here;
    }
}

/* **************************************************** compiled interpret */

static const p4_char_t p4_lit_interpret[] = "(INTERPRET)";

FCode (p4_interpret_next_execution)
{
    FX_PUSH (FX (p4_interpret_next_word));
}
FCode (p4_interpret_next)
{
    FX_COMPILE (p4_interpret_next);
}
P4COMPILES(p4_interpret_next, p4_interpret_next_execution,
	   P4_SKIPS_NOTHING, P4_DEFAULT_STYLE);

FCode (p4_interpret_undefined_execution)
{
    p4_type ((p4_char_t*) "oops... ", 8); // FIXME: for debugging...
    p4_throw (P4_ON_UNDEFINED);
}
FCode (p4_interpret_undefined)
{
    FX_COMPILE (p4_interpret_undefined);
}
P4COMPILES(p4_interpret_undefined, p4_interpret_undefined_execution,
	   P4_SKIPS_NOTHING, P4_DEFAULT_STYLE);

FCode_XE (p4_interpret_nothing_execution) {
    FX_USE_CODE_ADDR;
    FX_SKIP_BRANCH;
    FX_USE_CODE_EXIT;
}
FCode (p4_interpret_nothing) {
    FX_COMPILE (p4_interpret_nothing);
    FX_COMMA (0);
}
P4COMPILES(p4_interpret_nothing, p4_interpret_nothing_execution,
	   P4_SKIPS_OFFSET, P4_NEW1_STYLE);

FCode (p4_preload_interpret)
{
    p4_header_comma (p4_lit_interpret, sizeof(p4_lit_interpret)-1,
		     PFE.forth_wl);
    FX_RUNTIME1 (p4_colon);
    PFE.state = P4_TRUE;
    FX (p4_begin); // compiling word
    PFE.interpret_compile_resolve = ((p4cell*) p4_HERE) - 1;
    FX (p4_interpret_next); // like NEXT-WORD but returns FALSE for TERMINAL
    FX (p4_while);
    FX (p4_interpret_find);
    PFE.interpret_compile_extra = ((p4cell*) p4_HERE);
    FX (p4_interpret_nothing);
    FX (p4_interpret_number);
    PFE.interpret_compile_float = ((p4cell*) p4_HERE);
    FX (p4_interpret_nothing);
    FX (p4_interpret_undefined);
    FX (p4_repeat); // compiling word
    FX (p4_semicolon);
}

/* **********************************************************************
 * Initialize dictionary, and system variables, include files
 */

static const p4_char_t p4_lit_precision[] = "precision";
static const p4_char_t p4_lit_source_any_case[] = "source-any-case";
static const p4_char_t p4_lit_source_upper_case[] = "source-upper-case";
static const p4_char_t p4_lit_lower_case_filenames[] = "lower-case-filenames";

/**
 * setup all system variables and initialize the dictionary
 * to reach a very clean status as if right after cold boot.
 */
FCode (p4_cold_system)
{
    SP = p4_S0;
#  ifndef P4_NO_FP
    FP = p4_F0;
#  endif
#  ifdef P4_RP_IN_VM
    RP = p4_R0;
#  endif
    TIB = PFE.tib;
    BASE = 10;
    p4_DPL = -1;
    PRECISION = 6;
    WORDL_FLAG = 0; /* implicitly enables HASHing */
    WORDL_FLAG |= WORDL_NOCASE;
    WORDL_FLAG |= WORDL_UPPER_CASE;
    FLOAT_INPUT = P4_opt.float_input;

    PFE.local = (char (*)[P4_LOCALS]) PFE.stack; /* locals are stored as zstrings */
    PFE.pocket = PFE.pockets_ptr;

    p4_memset (PFE.files_top - 3, 0, sizeof (File) * 3);

    PFE.stdIn = PFE.files_top - 3;
    PFE.stdIn->f = stdin;
    p4_strcpy (PFE.stdIn->name, "<STDIN>");
    p4_strcpy (PFE.stdIn->mdstr, "r");
    PFE.stdIn->mode = FMODE_RO;

    PFE.stdOut = PFE.files_top - 2;
    PFE.stdOut->f = stdout;
    p4_strcpy (PFE.stdOut->name, "<STDOUT>");
    p4_strcpy (PFE.stdOut->mdstr, "a");
    PFE.stdOut->mode = FMODE_WO;

    PFE.stdErr = PFE.files_top - 1;
    PFE.stdErr->f = stderr;
    p4_strcpy (PFE.stdErr->name, "<STDERR>");
    p4_strcpy (PFE.stdErr->mdstr, "a");
    PFE.stdErr->mode = FMODE_WO;

    REDEFINED_MSG = P4_FALSE;

    /* Wipe the dictionary: */
    p4_memset (PFE.dict, 0, (PFE.dictlimit - PFE.dict));
    p4_preload_only ();
/*
    if (! PFE.abort_wl)     PFE.abort_wl  = p4_new_wordlist (0);
    if (! PFE.prompt_wl)    PFE.prompt_wl = p4_new_wordlist (0);
*/
    FX (p4_preload_interpret);
    FX (p4_only_RT);
    {
        /* Defines the following default search order:
         * FORTH EXTENSIONS ONLY */
        extern const p4Words P4WORDS (forth);
        p4_load_words (&P4WORDS (forth), ONLY, 0);
    }
    /* last step of bootup default search-order is
       FORTH DEFINITIONS a.k.a.  FORTH-WORDLIST CONTEXT ! DEFINITIONS
    */
    CURRENT = CONTEXT[0] = PFE.forth_wl; /* points to FORTH vocabulary */
    FX (p4_default_order);

    REDEFINED_MSG = P4_TRUE;
}

/**
 * setup all system variables and initialize the dictionary
 */
FCode (p4_boot_system)
{
    /* Action of COLD ABORT and QUIT, but don't enter the interactive QUIT */
    RESET_ORDER = P4_TRUE;
    REDEFINED_MSG = P4_FALSE;
    {
        extern const p4Words P4WORDS (extensions);
        p4_load_words (&P4WORDS (extensions), ONLY, 0);
    }
    abort_system ();
    quit_system ();

    REDEFINED_MSG = P4_FALSE;
    p4_included1 ((const p4_char_t*) PFE_BOOT_FILE, p4_strlen (PFE_BOOT_FILE), 0 );

    /* According to the ANS Forth description, the order after BOOT must
     * include the FORTH-WORDLIST, and the CURRENT definition-wordlist
     * must be the FORTH-WORDLIST. Here we assume that the various LOADs
     * before have kept atleast one occurence of FORTH-WORDLIST in the
     * search-order but we explicitly set the CURRENT definition-wordlist
     * Then we do DEFAULT-ORDER so it can pop up in a RESET-ORDER on ABORT
     * BEWARE: a bootscript can arrange the items in the search-order but
     * it can not arrange to set the CURRENT definitions-wordlist as well.
     * Note that ONLY is always searched, so one can always get back at FORTH
     * OTOH, in main-sub, the first include-file is loaded after boot_system
     * so it can arrange for a different the DEFAULT-ORDER incl. CURRENT.
     */
    CURRENT = PFE.forth_wl;
    FX (p4_default_order);

    FENCE = DP;
    LAST  = NULL;

    REDEFINED_MSG = P4_TRUE;
}

/*@}*/

/*
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */
