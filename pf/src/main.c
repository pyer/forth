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


#define ___ {
#define ____ }

/************************************************************************/

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
    p4_longjmp_exit ();
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
//    P4_INTO ("FORTH", 0),
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
p4_Wordl * _make_wordlist (p4char* nfa)
{
    p4_Wordl *w = (Wordl *) DP; /* allocate word list in HERE */
    P4_INC (DP, Wordl);
    w->link = nfa;
    LATEST = nfa;
    return w;
}

void pf_preload_forth (void)
{
    auto p4_Wordl only;                   /* scratch ONLY word list */
    memset (&only, 0, sizeof only);

    p4_header_comma ("FORTH", 5, &only);
    CURRENT = _make_wordlist (LATEST); 
    P4_NAMEFLAGS(LATEST) |= P4xIMMEDIATE;
}

//    LATEST = pf_create_header ("FORTH", 5);
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

    if (! dictfence)
    {
        dictfence = calloc (1, (size_t) total_size);
        if (dictfence)
        {
            printf("[%p] newmem at %p len %lu\n",
		      p4TH, dictfence, total_size);
        }else{
            printf("[%p] FAILED to alloc any base memory (len %lu): %s\n",
		      p4TH, total_size, strerror(errno));
            puts ("ERROR: out of memory");
	    exitcode = 6;
	    p4_longjmp_exit ();
        }
    }

    /* ________________ initialize dictionary _____________ */

    PFE.dict = dictfence;
    PFE.dictlimit = PFE.dict + total_size;

    p4_dict_allocate (HISTORY_SIZE, sizeof(char), sizeof(char),
                      (void**) & PFE.history, (void**) & PFE.history_top);
//    p4_dict_allocate (TIB_SIZE, sizeof(char), sizeof(char),
//                      (void**) & PFE.tib, (void**) & PFE.tib_end);
    p4_dict_allocate (ret_stack_size, sizeof(p4xt*),
                      PFE_ALIGNOF_CELL,
                      (void**) & PFE.rstack, (void**) & PFE.r0);
    p4_dict_allocate (stack_size, sizeof(p4cell),
                      PFE_ALIGNOF_CELL,
                      (void**) & PFE.stack, (void**) & PFE.s0);

//    p4_dict_allocate (ORDER_LEN+1, sizeof(void*), sizeof(void*),
//                      (void**) & PFE.context, (void**) 0);

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
    DPL = -1;
    PRECISION = 6;
    /* Wipe the dictionary: */
    memset (PFE.dict, 0, (PFE.dictlimit - PFE.dict));
    DP = (p4char *) PFE.dict;
    pf_preload_forth();
    //p4_load_words (&P4WORDS (forth), ONLY, 0);
    p4_load_words (&P4WORDS (forth), CURRENT, 0);
    /* -------- warm boot stage ------- */
    abort_system ();
    quit_system ();
    pf_include((const char *)PF_BOOT_FILE, strlen(PF_BOOT_FILE) );
    FENCE = DP;
    return exitcode;
}


/************************************************************************/
static char memory[TOTAL_SIZE]; /* BSS */
//struct p4_Thread* p4TH;

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
