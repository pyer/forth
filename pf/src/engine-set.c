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

static void
init_accept_lined (void)
{
    extern void (*p4_fkey_default_executes[10]) (int);

    p4_memset (&PFE.accept_lined, 0, sizeof PFE.accept_lined);
    PFE.accept_lined.history = PFE.history;
    PFE.accept_lined.history_max = PFE.history_top - PFE.history;
    PFE.accept_lined.complete = p4_complete_dictionary ;
    PFE.accept_lined.executes = p4_fkey_default_executes;
    PFE.accept_lined.caps = PFE_set.caps_on != 0;
}

/************************************************************************/
/* Here's main()                                                        */
/************************************************************************/

static void p4_atexit_cleanup (void);

/* distinct for each tread ! */
_export p4_threadP p4_main_threadP = NULL;

#define p4_current p4_main_threadP

/**
 * note the argument
 */
static int
p4_run_boot_system (p4_threadP th) /* main_init */
{
    p4_current = th;

#  ifdef PFE_USE_THREAD_BLOCK
#  define PFE_VM_p4TH(_th_)
#  else
#  define PFE_VM_p4TH(_th_)    p4TH = _th_
#  endif

    PFE_VM_p4TH(p4_current);

#  ifdef PFE_HAVE_LOCALE_H
    setlocale (LC_ALL, "C");
#  endif
#  if defined SYS_EMX
    _control87 (EM_DENORMAL | EM_INEXACT, MCW_EM);
#  endif

    PFE.exitcode = 0;
    switch (p4_setjmp (PFE.loop))
    {           /* classify unhandled throw codes */
    case 'A':
    case 'Q':	P4_fatal ("Boot System Failure");
        {   extern FCode(p4_come_back); /*:debug-ext:*/
#         ifdef P4_RP_IN_VM
            if (p4_R0) RP = p4_R0; /* quit_system */
            FX (p4_come_back);
#         endif
        }
        return -1;
    default:
		P4_warn ("Boot System Kill");
		/*fallthrough*/
    case 'X':
    	P4_info ("Boot System Exit/Bye");
    	return PFE.exitcode;
    case 0:     break;
    }

    /* ............................................................*/
    PFE_VM_p4TH(p4_current);

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

	if (PFE_set.bye)
            PFE_set.isnotatty = P4_TTY_NOECHO;
        else
	{
	    p4_interactive_terminal ();
	    PFE.system_terminal = &p4_system_terminal;
	}
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

    p4TH->atexit_cleanup = &p4_atexit_cleanup;

    /* _______________ dictionary block __________________ */

    if (! PFE_MEM)
    {
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
    PFE_VM_p4TH(p4_current);
    FX (p4_cold_system);
    init_accept_lined ();

    /* -------- warm boot stage ------- */
    FX (p4_boot_system);
    PFE_VM_p4TH(p4_current);

    /* FX (p4_boot_files); complete boot with it */
    /* FX (p4_script_files); complete application */
    /* FX (p4_main_loop); usually run as application */

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
static int p4_Run_application(p4_Thread* th);
static int p4_run_application(p4_Thread* th) /* main_loop */
{
    th->exitcode = 0;
    switch (p4_setjmp (th->loop))
    {           /* classify unhandled throw codes */
    case 'A':	P4_fatal ("Application Failure");
    case 'Q':	P4_info ("Application Throw/Quit");
        {    extern FCode(p4_come_back); /*:debug-ext:*/
#         ifdef P4_RP_IN_VM
            if (p4_R0) th->rp = RP = p4_R0; /* quit_system */
            FX (p4_come_back);
#         endif
        }
        return -1;
    default:
    	P4_warn ("Application Kill");
    	/*fallthrough*/
    case 'X':
    	P4_info ("Application Exit/Bye");
    	return th->exitcode;
    case 0:     break;
    }
    return p4_Run_application (th);
}

static FCode (p4_run_application)
{
    /* If it's a turnkey-application, start it: */
    if (APPLICATION)
    {
        p4_call_loop (APPLICATION);
        return;
    }

    /* If running in a pipe, process commands from stdin: */
    if (PFE_set.stdio)
    {
        p4_include_file (PFE.stdIn);
        return;
    }

    if (! PFE_set.bye)
	p4_interpret_loop (); /* will catch QUIT, ABORT .. and BYE */
}


static int p4_Run_application(p4_Thread* th)
{
    P4_CALLER_SAVEALL;
    PFE_VM_LOAD(th);
    FX (p4_run_application);
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
_export int
p4_Exec(p4_threadP th)
{
    auto volatile int retval;
    P4_CALLER_SAVEALL;
    retval =               p4_run_boot_system(th);
    if (! retval) retval = p4_run_application(th);
    if (1 /* always */)    p4_atexit_cleanup ();
    P4_CALLER_RESTORE;
    return retval;
}


static void
p4_atexit_cleanup (void)
{
    extern void p4_cleanup_terminal (void);
    P4_enter ("atexit cleanup");

    PFE.atexit_running = 1;
    p4_forget ((FENCE = PFE_MEM));

    if (PFE.system_terminal)    /* call this once, with the first cpu */
        PFE.system_terminal ();
    p4_cleanup_terminal ();

    { /* see if there's some memory chunk still to be freed */
        register int i;
        register int moptrs = PFE.moptrs ? PFE.moptrs : P4_MOPTRS;
        for ( i=0; i < moptrs; i++) {
            if (PFE.p[i]) {
                P4_info3 ("[%p] free %d. %p", p4TH, i, PFE.p[i]);
                p4_xfree (PFE.p[i]); PFE.p[i] = 0;
            }
        }
    }

    P4_leave ("atexit cleanup done");
}

/*@}*/

/*
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */
