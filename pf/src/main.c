/** 
 * -- Process command line, get memory and start up.
 * 
 *  Copyright (C) Tektronix, Inc. 1999 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.3 $
 *     (modified $Date: 2008-04-20 04:46:30 $)
 *
 *  @description
 *  Process command line, get memory and start up the interpret loop of PFE
 */
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>

#ifndef P4_NO_FP
#include <float.h>
#endif
#include <errno.h>
#include <locale.h>

#include "macro.h"
#include "types.h"
#include "const.h"
#include "listwords.h"
#include "session.h"

#include "dictionary.h"
#include "interpret.h"
#include "terminal.h"

void init_stdio( void );
void p4_load_words (const p4Words* ws, p4_Wordl* wid, int unused);

#define ___ {
#define ____ }

#ifndef ORDER_LEN               /* USER-CONFIG: */
#define ORDER_LEN 64            /* maximum wordlists in search order */
#endif
#ifndef HISTORY_SIZE	        /* USER-CONFIG: */
#define HISTORY_SIZE	0x1000  /* size of command line history buffer */
#endif

/* minimum space for <# # #S HOLD #> etc. */
#define MIN_HOLD	0x100
/* minimum free space in PAD */
#define MIN_PAD 	0x400

void pf_include(const char *name, int len);
int p4_close_file (p4_File *fid);
FCode    (p4_only_RT);
#define p4_longjmp_exit()	(p4_longjmp_loop('X'))
/************************************************************************/
int exitcode = 0;
void* dictfence = NULL;

jmp_buf loop_jump;       /* QUIT and ABORT do a THROW which longjmp() */
		    /* here thus C-stack gets cleaned up too */

/************************************************************************/
/**
 * just call longjmp on PFE.loop
 */
void p4_longjmp_loop(int arg)
{
    longjmp (PFE.loop, arg);
}

/**
 * fill the session struct with precompiled options
 */
void pf_default_options(p4_Session* set)
{
    if (! set) return;
    int len = sizeof(*set);

    memset(set, 0, len);

    /* newstyle option-ext support */
    set->opt.dict = set->opt.space;
    set->opt.dp = set->opt.dict;
    set->opt.last = 0;
    set->opt.dictlimit = ((p4char*)set) + len;

    set->argv = 0;
    set->argc = 0;
    set->optv = 0;
    set->optc = 0;
    set->boot_name = 0;
    set->isnotatty = 0;
    set->stdio = 0;
    set->caps_on = 0;
    set->find_any_case = 1;
//    set->lower_case_fn = 1;
    set->upper_case_on = 1;
#  ifndef P4_NO_FP
    set->float_input = 1;
#  else
    set->float_input = 0;
#  endif
    set->debug = 0;
//    set->cols = TEXT_COLS;
//    set->rows = TEXT_ROWS;
    set->total_size = TOTAL_SIZE;
    /* TOTAL_SIZE dependent defaults are moved to dict_allocate */
    //set->stack_size = (set->total_size / 32 + 256)  / sizeof(p4cell); 
    set->stack_size = (TOTAL_SIZE / 32 + 256)  / sizeof(p4cell); 
    //set->ret_stack_size = (set->total_size / 64 + 256) / sizeof(p4cell);
    set->ret_stack_size = (TOTAL_SIZE / 64 + 256) / sizeof(p4cell);
    //set->float_stack_size = (TOTAL_SIZE / 32) / sizeof(double);
    set->float_stack_size = (TOTAL_SIZE / 32) / sizeof(double);

}

/************************************************************************/
/**
 * helper routine to allocate a portion of the dictionary
 * especially for some stack-areas of the forth system
 * ... just decreases PFE.dictlimit, returns 0 if impossible.
 */
_export void*
p4_dict_allocate (int items, int size, int align, 
		  void** lower, void** upper)
{
    register p4char* memtop = PFE.dictlimit;
    if (! align) align = sizeof(p4cell);
    memtop =(p4char*)( ((p4cell)memtop) &~ ((p4cell)(align) -1) );
    if (upper) *upper = memtop;
    memtop -= items * size;
    if (lower) *lower = memtop;
    if (upper) PFE.dictlimit = memtop; /* always save if upper-ref given */
    if (memtop < PFE.dp + 256) return 0; /* error condition */
    return (PFE.dictlimit = memtop);
}
/************************************************************************/
void close_files (void)
{
    File *f = 0;
    for (f = PFE.files; f < PFE.files_top - 3; f++)
    {
        if (f->f)
	{
            p4_close_file (f);
	}
    }
}

/*
 * things => QUIT has to initialize
 */
void quit_system (void)
{
    CSP = (p4cell*) PFE.rp;     /* come_back marker */
    PFE.rp = PFE.r0;		/* return stack to its bottom */
    STATE = P4_FALSE;		/* interpreting now */
    PFE.catchframe = NULL;	/* and no exceptions to be caught */
//    PFE.debugging = 0;          /* turn off debugger */
    PFE.execute = pf_normal_execute;
}


/*
 * things => ABORT has to initialize
 */
void abort_system (void)
{
    PFE.sp = PFE.s0;				/* stacks */
//    if (PFE.abort[2]) { (PFE.abort[2]) (FX_VOID); } /* -> floating */
//    if (PFE.abort[3]) { (PFE.abort[3]) (FX_VOID); } /* -> dstrings */
    if (p4_RESET_ORDER)  {
        /* reset search order:
         * load the => DEFAULT-ORDER into the current search => ORDER
         * - this is implicitly done when a trap is encountered.
         */
        memcpy (p4_CONTEXT, p4_DFORDER, PFE_set.wordlists);
        p4_CURRENT = p4_DFCURRENT;
    }

    BASE = 10;
    /* close open filedescriptors except stdin, stdout and stderr */
    close_files();
}

/** BYE ( -- ) no-return
 * should quit the forth environment completly
 */
FCode (p4_bye)
{
    close_files();
    pf_outs ("\nGoodbye!\n");
    p4_longjmp_exit ();
}

/************************************************************************/
/** DEFAULT-ORDER ( -- )
 * nail the current search => ORDER so that it will even
 * survive a trap-condition. This default-order can be
 * explicitly loaded with => RESET-ORDER
 */
FCode (pf_default_order)
{
    memcpy (p4_DFORDER, p4_CONTEXT, PFE_set.wordlists);
    p4_DFCURRENT = p4_CURRENT;
}

/************************************************************************/
extern const p4Words
    P4WORDS(core),
    P4WORDS(exception),
    P4WORDS(compiler),
    P4WORDS(dictionary),
    P4WORDS(interpret),
    P4WORDS(file),
    P4WORDS(terminal),
    P4WORDS(shell),
    P4WORDS(signals),
    P4WORDS(tools),
    P4WORDS(debug),
    P4WORDS(version);

P4_LISTWORDS(forth) =
{
    P4_FXco ("BYE", p4_bye),
    P4_INTO ("FORTH", 0),
    P4_LOAD ("", core),
    P4_LOAD ("", exception),
    P4_LOAD ("", compiler),
    P4_LOAD ("", dictionary),
    P4_LOAD ("", interpret),
    P4_LOAD ("", file),
    P4_LOAD ("", terminal),
    P4_LOAD ("", shell),
    P4_LOAD ("", signals),
    P4_LOAD ("", tools),
    P4_LOAD ("", debug),
    P4_LOAD ("", version),
};
P4_COUNTWORDS(forth, "Forth Base system");
/************************************************************************/
/**
 * setup all system variables and initialize the dictionary
 * to reach a very clean status as if right after cold boot.
 */
void pf_cold_system(void)
{
    PFE.sp = PFE.s0;
#  ifndef P4_NO_FP
    PFE.fp = PFE.f0;
#  endif
    PFE.rp = PFE.r0;
    PFE.word.len = -1;
    BASE = 10;
    p4_DPL = -1;
    PRECISION = 6;
    WORDL_FLAG = 0; /* implicitly enables HASHing */
    WORDL_FLAG |= WORDL_NOCASE;
    WORDL_FLAG |= WORDL_UPPER_CASE;
    FLOAT_INPUT = P4_opt.float_input;

//    PFE.local = (char (*)[P4_LOCALS]) PFE.stack; /* locals are stored as zstrings */

    memset (PFE.files_top - 3, 0, sizeof (File) * 3);

    init_stdio();

    REDEFINED_MSG = P4_FALSE;

    /* Wipe the dictionary: */
    memset (PFE.dict, 0, (PFE.dictlimit - PFE.dict));
    p4_preload_only ();
    p4_only_RT_();
    //FX (p4_only_RT);
    p4_load_words (&P4WORDS (forth), ONLY, 0);
    /* last step of bootup default search-order is
       FORTH DEFINITIONS a.k.a.  FORTH-WORDLIST CONTEXT ! DEFINITIONS
    */
    CURRENT = CONTEXT[0] = PFE.forth_wl; /* points to FORTH vocabulary */
    pf_default_order_();
    //FX (pf_default_order);

    REDEFINED_MSG = P4_TRUE;
}

/**
 * setup all system variables and initialize the dictionary
 */
void pf_boot_system(void)
{
    /* Action of COLD ABORT and QUIT, but don't enter the interactive QUIT */
    RESET_ORDER = P4_TRUE;
    REDEFINED_MSG = P4_FALSE;
    abort_system ();
    quit_system ();
    REDEFINED_MSG = P4_FALSE;
    pf_include((const char *)PF_BOOT_FILE, strlen(PF_BOOT_FILE) );

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
//    FX (pf_default_order);

    FENCE = DP;
    LAST  = NULL;

    REDEFINED_MSG = P4_TRUE;
}

/************************************************************************/
/* Here's main()                                                        */
/************************************************************************/

/**
 * note the argument
 */
int pf_init_system (p4_Thread* th) /* main_init */
{
    setlocale (LC_ALL, "C");
#  if defined SYS_EMX
    _control87 (EM_DENORMAL | EM_INEXACT, MCW_EM);
#  endif

    /* ............................................................*/
    p4TH = th;

#  if !defined __WATCOMC__
    if (! isatty (STDIN_FILENO))
        PFE_set.stdio = 1;
#  endif
/*
    if (PFE_set.stdio)
        PFE_set.isnotatty = P4_TTY_ISPIPE;
    else
    {
        if (! p4_prepare_terminal ())
	{
            PFE_set.isnotatty = P4_TTY_ISPIPE;
	}
//	p4_interactive_terminal ();
    }

    if (PFE_set.isnotatty == P4_TTY_ISPIPE && ! PFE.term)
    {
        extern p4_term_struct p4_term_stdio;
        PFE.term = &p4_term_stdio;
    }
*/
    pf_init_terminal();

//    if (! PFE_set.debug)
//        p4_install_signal_handlers ();


    /* _______________ dictionary block __________________ */

    if (! dictfence)
    {
        dictfence = calloc (1, (size_t) PFE_set.total_size);
        if (dictfence)
        {
            printf("[%p] newmem at %p len %lu\n",
		      p4TH, dictfence, PFE_set.total_size);
        }else{
            printf("[%p] FAILED to alloc any base memory (len %lu): %s\n",
		      p4TH, PFE_set.total_size,
		      strerror(errno));
            puts ("ERROR: out of memory");
	    exitcode = 6;
	    p4_longjmp_exit ();
        }
    }

    /* ________________ initialize dictionary _____________ */

    PFE.dict = dictfence;
    PFE.dictlimit = PFE.dict + PFE_set.total_size;

    p4_dict_allocate (HISTORY_SIZE, sizeof(char), sizeof(char),
                      (void**) & PFE.history, (void**) & PFE.history_top);
    p4_dict_allocate (P4_MAX_FILES, sizeof(File), PFE_ALIGNOF_CELL,
                      (void**) & PFE.files, (void**) & PFE.files_top);
//    p4_dict_allocate (TIB_SIZE, sizeof(char), sizeof(char),
//                      (void**) & PFE.tib, (void**) & PFE.tib_end);
    p4_dict_allocate (PFE_set.ret_stack_size, sizeof(p4xt*),
                      PFE_ALIGNOF_CELL,
                      (void**) & PFE.rstack, (void**) & PFE.r0);
    p4_dict_allocate (PFE_set.stack_size, sizeof(p4cell),
                      PFE_ALIGNOF_CELL,
                      (void**) & PFE.stack, (void**) & PFE.s0);

    PFE_set.wordlists = ORDER_LEN;
    p4_dict_allocate (PFE_set.wordlists+1, sizeof(void*), sizeof(void*),
                      (void**) & PFE.context, (void**) 0);
    p4_dict_allocate (PFE_set.wordlists, sizeof(void*), sizeof(void*),
                      (void**) & PFE.dforder, (void**) 0);

    if (PFE.dictlimit < PFE.dict + MIN_PAD + MIN_HOLD + 0x4000)
    {
	puts ("ERROR: impossible memory map");
	exitcode = 3;
    	return exitcode;
    }

    /*  -- cold boot stage -- */
    pf_cold_system();
    /* -------- warm boot stage ------- */
    pf_boot_system();
    return exitcode;
}


/************************************************************************/
static char memory[TOTAL_SIZE]; /* BSS */
//struct p4_Thread* p4TH;

/************************************************************************/
int main (int argc, char** argv)
{
    char buffer[256];
    int len;
    p4_Thread* thread = (p4_Thread*) memory;
    p4_Session session;
  
    pf_default_options(&session);
    memset (thread, 0, sizeof(p4_Thread));
    thread->set = &session;
    pf_init_system(thread);
    exitcode = 0;
    switch (setjmp (thread->loop))
    {           /* classify unhandled throw codes */
    case 'A': /* do abort */
         abort_system();
         break;
    case 'Q': /* do quit */
         quit_system();
         break;
    default:
    	return exitcode;
    case 0:     break;
    }
    for (;;) {
             if (isatty (STDIN_FILENO))
                 pf_outs (" ok\n");
             len = pf_accept (buffer, 255);
             pf_interpret(buffer, len);
             FCode (pf_Q_stack);
    }
    return exitcode;
}
