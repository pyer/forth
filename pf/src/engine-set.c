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

#ifndef STACK_SIZE              /* USER-CONFIG: */
#define	STACK_SIZE	0	/* 0 -> P4_KB*1024 / 32 + 256 */
#endif
#ifndef RET_STACK_SIZE          /* USER-CONFIG: */
#define	RET_STACK_SIZE	0	/* 0 -> P4_KB*1024 / 64 + 256 */
#endif

#ifndef ORDER_LEN               /* USER-CONFIG: */
#define ORDER_LEN 64            /* maximum wordlists in search order */
#endif
#ifndef HISTORY_SIZE	        /* USER-CONFIG: */
#define HISTORY_SIZE	0x1000  /* size of command line history buffer */
#endif

static char const * const empty = "";

/************************************************************************/
/* Initialize memory map:                                               */
/************************************************************************/

void p4_SetDictMem (p4_threadP thread, void* dictmem, long size)
{
    if (!dictmem) return;
    thread->p[P4_MEM_SLOT] = dictmem;
    thread->moptrs = P4_MEM_SLOT;   /* _cleanup shall not free this one */
    thread->set->total_size = size; /* or any later module mem pointer */
}

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
#  ifdef VXWORKS
    extern int taskVarAdd (int, int*);
    extern int taskIdSelf ();
    taskVarAdd (taskIdSelf (), (int*) &p4_current);
#  endif
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

# ifdef USE_MMAP
    if ((s = p4_search_option_string ("map-file", 8, 0, PFE.set)))
    {
        p4ucell l = p4_search_option_value ("map-base", 8, 0, PFE.set);
	PFE.mapfile_fd = p4_mmap_creat (s, l, PFE_set.total_size);
	if (! PFE.mapfile_fd)
	{
	    P4_fail1 ("[%p] mapfile failed", p4TH);
	}else{
	    P4_info3 ("[%p] mapped at %8p len %d",
		      p4TH, PFE_MEM, PFE_set.total_size);
	}
    }
# endif
#define p4_search_option_value_static(__option,__default,__wordlist) \
        p4_search_option_value( \
          (p4_char_t*) __option, sizeof(__option)-1, __default, __wordlist)
#define p4_search_option_string_static(__option,__default,__wordlist) \
        p4_search_option_string( \
          (p4_char_t*) __option, sizeof(__option)-1, __default, __wordlist)
#define p4_lookup_option_string_static(__option,__default,__wordlist) \
        p4_lookup_option_string( \
          (p4_char_t*) __option, sizeof(__option)-1, __default, __wordlist)
    if (! PFE_MEM)
    {
#      ifndef P4_MIN_KB
#      define P4_MIN_KB 60
#      endif
        unsigned long total_size = p4_search_option_value_static("/total",
	    PFE_set.total_size, PFE.set);
        if (total_size < P4_MIN_KB*1024) total_size = P4_MIN_KB*1024;

        PFE_MEM = p4_xcalloc (1, (size_t) total_size);
        if (PFE_MEM)
        {
            P4_info3 ("[%p] newmem at %p len %lu",
		      p4TH, PFE_MEM, total_size);
        }else{
            P4_fail3 ("[%p] FAILED to alloc any base memory (len %lu): %s",
		      p4TH, total_size,
		      strerror(errno));
        }
        if (total_size != PFE_set.total_size)
        {
            P4_info3 ("[%p] OVERRIDE total_size %lu -> %lu",
                      p4TH, (unsigned long) PFE_set.total_size, total_size);
            PFE_set.total_size = total_size;
        }
    }

    /* ________________ initialize dictionary _____________ */

    PFE.dict = PFE_MEM;
    PFE.dictlimit = PFE.dict + PFE_set.total_size;

    ___ register int v; /* FIXME: we should update the option-ext entries in
                         * the cases where we clamp the value to sane ranges*/
    v = p4_search_option_value_static ("#pockets", POCKETS, PFE.set);
    if (v < 0) v = POCKETS; if (v < 1) v = 1;
    p4_dict_allocate (v, sizeof(p4_pocket_t), sizeof(char),
                      (void**) & PFE.pockets_ptr, (void**) & PFE.pockets_top);
    v = p4_search_option_value_static ("/history", HISTORY_SIZE, PFE.set);
    if (v < 0) v = HISTORY_SIZE;
    p4_dict_allocate (v, sizeof(char), sizeof(char),
                      (void**) & PFE.history, (void**) & PFE.history_top);
    v = p4_search_option_value_static ("#files", MAX_FILES, PFE.set);
    if (v < 0) v = MAX_FILES; if (v < 4) v = 4;
    p4_dict_allocate (v, sizeof(File), PFE_ALIGNOF_CELL,
                      (void**) & PFE.files, (void**) & PFE.files_top);
    v = p4_search_option_value_static ("/tib", TIB_SIZE, PFE.set);
    if (v < 0) v = TIB_SIZE; if (v < 64) v = 64;
    p4_dict_allocate (v, sizeof(char), sizeof(char),
                      (void**) & PFE.tib, (void**) & PFE.tib_end);
    ____;

    if (! PFE_set.ret_stack_size)
        PFE_set.ret_stack_size =
            p4_search_option_value_static ("return-stack-cells",
                RET_STACK_SIZE ? RET_STACK_SIZE
                : (PFE_set.total_size / 64 + 256) / sizeof(p4cell), PFE.set);
    p4_dict_allocate (PFE_set.ret_stack_size, sizeof(p4xt*),
                      PFE_ALIGNOF_CELL,
                      (void**) & PFE.rstack, (void**) & PFE.r0);

    if (! PFE_set.stack_size)
        PFE_set.stack_size =
            p4_search_option_value_static ("stack-cells",
                STACK_SIZE ? STACK_SIZE
                : (PFE_set.total_size / 32 + 256)  / sizeof(p4cell), PFE.set);
    p4_dict_allocate (PFE_set.stack_size, sizeof(p4cell),
                      PFE_ALIGNOF_CELL,
                      (void**) & PFE.stack, (void**) & PFE.s0);

    PFE_set.wordlists =
        p4_search_option_value_static ("wordlists", ORDER_LEN, PFE.set);
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

    PFE.set->inc_ext   = p4_lookup_option_string_static (
        "INC-EXT",     (const char**) & empty, PFE.set);
    PFE.set->inc_paths = p4_lookup_option_string_static (
        "INC-PATH",    (const char**) & empty, PFE.set);
    PFE.set->lib_paths = p4_lookup_option_string_static (
        "LIB-PATH",    (const char**) & empty, PFE.set);

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




/* wrapping a catch domain around p4_script_files above. The lower
 * routine is also called from COLD which does run EMPTY followed
 * by re-including the SCRIPT-FILE to re-initialize the system */
static int p4_Run_script_files(p4_Thread* th);
static int p4_run_script_files(p4_Thread* th)
{
    switch (p4_setjmp (th->loop))
    {           /* classify unhandled throw codes */
    case 'A':
    case 'Q':	P4_fatal ("Script File Throw/Quit");
        {   extern FCode(p4_come_back); /*:debug-ext:*/
#         ifdef P4_RP_IN_VM
            if (p4_R0) th->rp = RP = p4_R0; /* quit_system */
            FX (p4_come_back);
#         endif
        }
        return -1;
    default:
    	P4_warn ("Script File Kill");
    	/*fallthrough*/
    case 'X':
    	P4_info ("Script File Exit/Bye");
    	return th->exitcode;
    case 0:     break;
    }
    return p4_Run_script_files (th);
}

/* boot_includes
 * This routine is ususally run right after p4_boot_system. Perhaps
 * some other boot routines have run, and then script-files shall
 * be included - we set the environment => MARKER => EMPTY in this
 * routine so you can always go back to the dictionary state just
 * before this routine. That is actually done in => COLD for example.
 */
static FCode(p4_run_script_files)
{
    register char const * s;
    {
        register p4_Wordl* old = CURRENT; CURRENT = PFE.environ_wl;
        FX_PUSH("EMPTY"); FX_PUSH(5); FX(p4_paren_marker);
        CURRENT = old;
    }

    /* USER-CONF --load-image=<file>            (alias --image-file=<name>) */
    s = (const char*) p4_search_option_string_static ("image-file",
	0, PFE.set); /* gforth's */
    s = (const char*) p4_search_option_string_static ("load-image",
        s, PFE.set); /* pfe's */
    if (s) { P4_fail2 ("[%p] load wordset image-file not implemented: %s",
                       p4TH, s); }

#  if 0
    /* already loaded during p4_boot_system */
    s = (const char*) p4_search_option_string_static ("boot-file",
        0, PFE.set);
    if (s && *s) { p4_included1 (s, p4_strlen(s), 0); }
#  endif

    /* process the boot command: */
    s = (const char*) p4_search_option_string_static ("boot-init",
      0, PFE.set);
    if (s && *s) { p4_evaluate ((const p4_char_t*) s, p4_strlen(s)); }

    /* Include file from command line: */
    s = (const char*) p4_search_option_string_static ("script-file",
      0, PFE.set);
    if (s && *s) { p4_included1 ((const p4_char_t*) s, p4_strlen(s), 0); }

    /* possibly subselect a section from that script */
    s = (const char*) p4_search_option_string_static ("script-init",
      0, PFE.set);
    if (s && *s) { p4_evaluate ((const p4_char_t*) s, p4_strlen(s)); }
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


static int p4_Run_script_files(p4_Thread* th)
{
    P4_CALLER_SAVEALL;
    PFE_VM_LOAD(th);
    FX (p4_run_script_files);
    PFE_VM_SAVE(th);
    P4_CALLER_RESTORE;
    return th->exitcode;
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
    if (! retval) retval = p4_run_script_files(th);
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

#  ifdef USE_MMAP
    if (PFE.mapfile_fd)
    {
	p4_mmap_close(PFE.mapfile_fd, PFE_MEM, PFE_set.total_size);
        PFE_MEM = 0; PFE.mapfile_fd = 0;
        P4_info1 ("[%p] unmapped basemem", p4TH);
    }
#  endif

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
