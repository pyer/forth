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

#include "exception.h"
#include "interpret.h"
#include "terminal.h"


#define ___ {
#define ____ }

/************************************************************************/
int exitcode = 0;

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
/*
 * things => QUIT has to initialize
 */
void quit_system (void)
{
    CSP = (p4cell*) PFE.rp;     /* come_back marker */
    PFE.rp = PFE.r0;		/* return stack to its bottom */
    STATE = P4_FALSE;		/* interpreting now */
    PFE.catchframe = NULL;	/* and no exceptions to be caught */
    PFE.execute = pf_normal_execute;
}

/*
 * things => ABORT has to initialize
 */
void abort_system (void)
{
    PFE.sp = PFE.s0;		/* stacks */
    BASE = 10;
}

/** BYE ( -- ) no-return
 * should quit the forth environment completly
 */
FCode (p4_bye)
{
    pf_outs ("\nGoodbye!\n");
    pf_longjmp_exit ();
}

/************************************************************************/
extern const p4Words
    P4WORDS(core),
    P4WORDS(exception),
    P4WORDS(compiler),
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
    P4_LOAD ("", core),
    P4_LOAD ("", exception),
    P4_LOAD ("", compiler),
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
 * note the argument
 */
int pf_init_system (p4_Thread* th) /* main_init */
{
    long int total_size = TOTAL_SIZE;
    long int stack_size = (TOTAL_SIZE / 32 + 256)  / sizeof(p4cell); 
    long int ret_stack_size = (TOTAL_SIZE / 64 + 256) / sizeof(p4cell);
//    long int float_stack_size = (TOTAL_SIZE / 32) / sizeof(double);

    setlocale (LC_ALL, "C");
#  if defined SYS_EMX
    _control87 (EM_DENORMAL | EM_INEXACT, MCW_EM);
#  endif

    /* ............................................................*/
    p4TH = th;

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

    PFE.dict = calloc (1, (size_t) total_size);
    if (PFE.dict) {
            printf("[%p] newmem at %p len %lu\n",
		      p4TH, PFE.dict, total_size);
    }else{
            printf("[%p] FAILED to alloc any base memory (len %lu): %s\n",
		      p4TH, total_size, strerror(errno));
            puts ("ERROR: out of memory");
	    exitcode = 6;
	    pf_longjmp_exit ();
    }

    /* ________________ initialize dictionary _____________ */

    PFE.dictlimit = PFE.dict + total_size;

    p4_dict_allocate (HISTORY_SIZE, sizeof(char), sizeof(char),
                      (void**) & PFE.history, (void**) & PFE.history_top);
    p4_dict_allocate (ret_stack_size, sizeof(p4xt*),
                      PFE_ALIGNOF_CELL,
                      (void**) & PFE.rstack, (void**) & PFE.r0);
    p4_dict_allocate (stack_size, sizeof(p4cell),
                      PFE_ALIGNOF_CELL,
                      (void**) & PFE.stack, (void**) & PFE.s0);

    if (PFE.dictlimit < PFE.dict + MIN_PAD + MIN_HOLD + 0x4000)
    {
	puts ("ERROR: impossible memory map");
	exitcode = 3;
    	return exitcode;
    }

    /*  -- cold boot stage -- */
    PFE.sp = PFE.s0;
#  ifndef P4_NO_FP
    PFE.fp = PFE.f0;
#  endif
    PFE.rp = PFE.r0;
    PFE.word.len = -1;
    BASE = 10;
    PRECISION = 6;
    /* Wipe the dictionary: */
    memset (PFE.dict, 0, (PFE.dictlimit - PFE.dict));
    DP = (p4char *) PFE.dict;
    /* Create first word */
    p4_header_comma ("FORTH", 5);
    P4_NAMEFLAGS(LATEST) |= P4xIMMEDIATE;
    /* and load other words */
    pf_load_words (&P4WORDS (forth));
    /* -------- warm boot stage ------- */
    abort_system ();
    quit_system ();
    pf_include((const char *)PF_BOOT_FILE, strlen(PF_BOOT_FILE) );
    return exitcode;
}

/************************************************************************/
static char memory[TOTAL_SIZE]; /* BSS */

/************************************************************************/
/* Here's main()                                                        */
/************************************************************************/
int main (int argc, char** argv)
{
    char buffer[256];
    int len;
    p4_Thread* thread = (p4_Thread*) memory;
  
    memset (thread, 0, sizeof(p4_Thread));
    pf_init_system(thread);
    exitcode = 0;
    //switch (setjmp (thread->loop))
    switch (setjmp (jump_loop))
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
