/**
 * -- setup forth memory and start up.
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2003.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.5 $
 *     (modified $Date: 2008-05-03 14:20:20 $)
 *
 *  @description
 *  Process options via options block (set in option-set), get memory
 *  and init variables, and finally start up the interpret loop of PFE
 */
/*@{*/
#if defined(__version_control__) && defined(__GNUC__)
static char* id __attribute__((unused)) =
"@(#) $Id: engine-set.c,v 1.5 2008-05-03 14:20:20 guidod Exp $";
#endif

#define	_P4_SOURCE 1

#ifndef _export
#define _export
#include <pfe/pfe-base.h>
#endif

#include <pfe/def-limits.h>

#include <stdlib.h>
#include <stdarg.h>
#include <pfe/os-string.h>
#ifndef P4_NO_FP
#include <float.h>
#endif
#include <errno.h>
#ifdef PFE_HAVE_LOCALE_H
#include <locale.h>
#endif
#ifdef PFE_HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <pfe/term-sub.h>
#include <pfe/version-sub.h>
#include <pfe/exception-sub.h>
#include <pfe/lined.h>
#include <pfe/_nonansi.h>
#include <pfe/_missing.h>

#include <pfe/option-ext.h>
#include <pfe/floating-ext.h>
#include <pfe/logging.h>

#include <pfe/def-restore.h>

#define ___ {
#define ____ }

#ifndef ORDER_LEN               /* USER-CONFIG: */
#define ORDER_LEN 64            /* maximum wordlists in search order */
#endif
#ifndef HISTORY_SIZE	        /* USER-CONFIG: */
#define HISTORY_SIZE	0x1000  /* size of command line history buffer */
#endif

static char const * const empty = "";

static void init_accept_lined (void)
{
    extern void (*p4_fkey_default_executes[10]) (int);

    p4_memset (&PFE.accept_lined, 0, sizeof PFE.accept_lined);
    PFE.accept_lined.history = PFE.history;
    PFE.accept_lined.history_max = PFE.history_top - PFE.history;
    PFE.accept_lined.complete = p4_complete_dictionary ;
    PFE.accept_lined.executes = p4_fkey_default_executes;
    PFE.accept_lined.caps = PFE_set.caps_on != 0;
}

/**
 * fill the session struct with precompiled options
 */
void p4_default_options(p4_Session* set)
{
    if (! set) return;
    int len = sizeof(*set);

    p4_memset(set, 0, len);

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
    set->lower_case_fn = 1;
    set->upper_case_on = 1;
#  ifndef P4_NO_FP
    set->float_input = 1;
#  else
    set->float_input = 0;
#  endif
    set->debug = 0;
    set->cols = TEXT_COLS;
    set->rows = TEXT_ROWS;
    set->total_size = TOTAL_SIZE;
    /* TOTAL_SIZE dependent defaults are moved to dict_allocate */
    //set->stack_size = (set->total_size / 32 + 256)  / sizeof(p4cell); 
    set->stack_size = (TOTAL_SIZE / 32 + 256)  / sizeof(p4cell); 
    //set->ret_stack_size = (set->total_size / 64 + 256) / sizeof(p4cell);
    set->ret_stack_size = (TOTAL_SIZE / 64 + 256) / sizeof(p4cell);
    //set->float_stack_size = (TOTAL_SIZE / 32) / sizeof(double);
    set->float_stack_size = (TOTAL_SIZE / 32) / sizeof(double);

    set->cpus = P4_MP;
}

/************************************************************************/
/* Here's main()                                                        */
/************************************************************************/

/**
 * note the argument
 */
int p4_run_boot_system (p4_Thread* th) /* main_init */
{
#  ifdef PFE_HAVE_LOCALE_H
    setlocale (LC_ALL, "C");
#  endif
#  if defined SYS_EMX
    _control87 (EM_DENORMAL | EM_INEXACT, MCW_EM);
#  endif

    /* ............................................................*/
    p4TH = th;
    PFE.exitcode = 0;

#  if !defined __WATCOMC__
    if (! isatty (STDIN_FILENO))
        PFE_set.stdio = 1;
#  endif

    if (PFE_set.stdio) /* FIXME: PFE.term must still be prepare()'d */
        PFE_set.isnotatty = P4_TTY_ISPIPE;
    else
    {
        if (! p4_prepare_terminal ())
	{
            PFE_set.isnotatty = P4_TTY_ISPIPE;
	}
	p4_interactive_terminal ();
    }

    if (PFE_set.isnotatty == P4_TTY_ISPIPE && ! PFE.term)
    {
        extern p4_term_struct p4_term_stdio;
        PFE.term = &p4_term_stdio;
    }

//    if (! PFE_set.debug)
//        p4_install_signal_handlers ();

    if (PFE.rows == 0)
        PFE.rows = PFE_set.rows;
    if (PFE.cols == 0)
        PFE.cols = PFE_set.cols;

    /* _______________ dictionary block __________________ */

    if (! PFE_MEM)
    {
//puts("PFE_MEM alloc");
        PFE_MEM = p4_xcalloc (1, (size_t) PFE_set.total_size);
        if (PFE_MEM)
        {
            P4_info3 ("[%p] newmem at %p len %lu",
		      p4TH, PFE_MEM, PFE_set.total_size);
        }else{
            P4_fail3 ("[%p] FAILED to alloc any base memory (len %lu): %s",
		      p4TH, PFE_set.total_size,
		      strerror(errno));
        }
    }

    /* ________________ initialize dictionary _____________ */

    PFE.dict = PFE_MEM;
    PFE.dictlimit = PFE.dict + PFE_set.total_size;

    p4_dict_allocate (P4_POCKETS, sizeof(p4_pocket_t), sizeof(char),
                      (void**) & PFE.pockets_ptr, (void**) & PFE.pockets_top);
    p4_dict_allocate (HISTORY_SIZE, sizeof(char), sizeof(char),
                      (void**) & PFE.history, (void**) & PFE.history_top);
    p4_dict_allocate (P4_MAX_FILES, sizeof(File), PFE_ALIGNOF_CELL,
                      (void**) & PFE.files, (void**) & PFE.files_top);
    p4_dict_allocate (TIB_SIZE, sizeof(char), sizeof(char),
                      (void**) & PFE.tib, (void**) & PFE.tib_end);
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
	P4_fatal ("impossible memory map");
	PFE.exitcode = 3;
    	return PFE.exitcode;
    }

    /*  -- cold boot stage -- */
    FX (p4_cold_system);
    init_accept_lined ();

    /* -------- warm boot stage ------- */
    FX (p4_boot_system);

    PFE_VM_SAVE(th);
    return PFE.exitcode;
}

/**
 * The run-application routine will check the various variants of
 * the pfe execution model including to start APPLICATION or to
 * process lines via stdin. If nothing like that is the case then
 * we enter the QUIT mainloop - this is doing an infinite loop of
 * QUERY INTERPRET which can only be left via BYE. Note that in PFE
 * the mainloop is not exported and the word QUIT is actually a
 * THROW-code that jumps to the CATCH-domain of that mainloop.
 * (the EXITCODE of this routine can be set by the forth application)
 */

void abort_system (void);
void quit_system (void);

int p4_run_application(p4_Thread* th) /* main_loop */
{
    th->exitcode = 0;
    switch (p4_setjmp (th->loop))
    {           /* classify unhandled throw codes */
    case 'A': /* do abort */
         abort_system();
         break;
    case 'Q': /* do quit */
         quit_system();
         break;
    default:
    	return th->exitcode;
    case 0:     break;
    }
//    P4_CALLER_SAVEALL;
    PFE_VM_LOAD(th);
    for (;;) {
             FX (p4_ok);
             FX (p4_cr);
             FX (p4_query);
             FX (p4_interpret);
             FX (p4_Q_stack);
    }
    PFE_VM_SAVE(th); /* ... */
    return th->exitcode;
}

/*@}*/
/*
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */
