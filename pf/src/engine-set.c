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
void p4_default_options(p4_sessionP set)
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
    set->license = 0;
    set->warranty = 0;
    set->quiet = 0;
    set->verbose = 0;
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
int p4_run_boot_system (p4_threadP th) /* main_init */
{
#  ifdef PFE_USE_THREAD_BLOCK
#  define PFE_VM_p4TH(_th_)
#  else
#  define PFE_VM_p4TH(_th_)    p4TH = _th_
#  endif

#  ifdef PFE_HAVE_LOCALE_H
    setlocale (LC_ALL, "C");
#  endif
#  if defined SYS_EMX
    _control87 (EM_DENORMAL | EM_INEXACT, MCW_EM);
#  endif

    /* ............................................................*/
    PFE_VM_p4TH(th);
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
            if (! PFE_set.quiet)
                P4_fatal (
		    "[unknown terminal, "
#                  if defined ASSUME_VT100
		    "assuming vt100"
#                  elif defined ASSUME_DUMBTERM
		    "assuming dumb terminal"
#                  else
		    "running without terminal mode"
#                  endif
		    "]");
#          if !defined ASSUME_VT100 && !defined ASSUME_DUMBTERM
            PFE_set.isnotatty = P4_TTY_ISPIPE;
#          endif
	}
	p4_interactive_terminal ();
	PFE.system_terminal = &p4_system_terminal;
    }

    if (PFE_set.isnotatty == P4_TTY_ISPIPE && ! PFE.term)
    {
        extern p4_term_struct p4_term_stdio;
        PFE.term = &p4_term_stdio;
    }

    if (! PFE_set.debug)
        p4_install_signal_handlers ();

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
	p4_longjmp_exit ();
    }

    /*  -- cold boot stage -- */
    PFE_VM_p4TH(th);
    FX (p4_cold_system);
    init_accept_lined ();

    /* -------- warm boot stage ------- */
    FX (p4_boot_system);

    PFE_VM_p4TH(th);
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
    P4_CALLER_SAVEALL;
    PFE_VM_LOAD(th);
//    p5_interpret_loop();
         for (;;)
         {
             FX (p4_ok);
             FX (p4_cr);
             FX (p4_query);
             FX (p4_interpret);
             FX (p4_Q_stack);
         }
    PFE_VM_SAVE(th); /* ... */
    P4_CALLER_RESTORE;
    return th->exitcode;
}

/**
 * init and execute the previously allocated forth-machine,
 * e.g. pthread_create(&thread_id,0,p4_Exec,threadP);
 *
 * The following words have been extracted from a big boot init
 * procedure previously existing in PFE. In the boot_system we
 * do initialize all inputs/outputs and load the wordset extensions
 * and the boot-preinit-block or boot-preinit-script. After that,
 * we run script_files to init the application code, and finally
 * the application is started - and if no APPLICATION was set then
 * we do the fallback to the forth interactive INTERPRET loop. The
 * latter is the usual case, use BYE to exit that inifinite loop.
 *
 * When the mainloop returns, we run the cleanup-routines. They are
 * registered seperatly so they can be run asynchronously - if the
 * application has broken down or it blocks hard on some hardware
 * then we can still run cleanup code in a new forthish context.
 */
void p4_atexit_cleanup (void)
{
    extern void p4_cleanup_terminal (void);
    P4_enter ("atexit cleanup");
    p4_forget ((FENCE = PFE_MEM));

    if (PFE.system_terminal)    /* call this once, with the first cpu */
        PFE.system_terminal ();
    p4_cleanup_terminal ();
    P4_leave ("atexit cleanup done");
}

/*@}*/
/*
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */
