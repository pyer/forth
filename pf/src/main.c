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

#if defined PF_WITH_FLOATING
#include <float.h>
#endif
#include <errno.h>
#include <locale.h>

#include "macro.h"
#include "types.h"
#include "const.h"
#include "listwords.h"
#include "thread.h"

#include "compiler.h"
#include "exception.h"
#include "interpret.h"
#include "terminal.h"

const char* pf_version_string(void);
void pf_init_signal_handlers (void);

/************************************************************************/

static char memory[TOTAL_SIZE]; /* BSS */

/************************************************************************/
int exitcode = 0;

/* dictionary */
char* dict;
char* dictlimit;

/* NFA of most recently CREATEd header */
p4_namebuf_t *LATEST;
/************************************************************************/
/**
 * helper routine to allocate a portion of the dictionary
 * especially for some stack-areas of the forth system
 * ... just decreases dictlimit, returns 0 if impossible.
 */
void* p4_dict_allocate (int items, int size, int align, void** lower, void** upper)
{
    register p4char* memtop = dictlimit;
    if (! align) align = sizeof(p4cell);
    memtop =(p4char*)( ((p4cell)memtop) &~ ((p4cell)(align) -1) );
    if (upper) *upper = memtop;
    memtop -= items * size;
    if (lower) *lower = memtop;
    if (upper) dictlimit = memtop; /* always save if upper-ref given */
    if (memtop < DP + 256) return 0; /* error condition */
    return (dictlimit = memtop);
}
/************************************************************************/
/*
 * things => QUIT has to initialize
 */
void quit_system (void)
{
    CSP = (p4cell*) RP;		/* come_back marker */
    RP = R0;			/* return stack to its bottom */
    STATE = P4_FALSE;		/* interpreting now */
    PFE.execute = pf_normal_execute;
}

/*
 * things => ABORT has to initialize
 */
void abort_system (void)
{
    BASE = 10;
    SP = S0;		/* stacks */
#if defined PF_WITH_FLOATING
    FP = F0;
    PRECISION = 6;
#endif
}

/************************************************************************/
extern const p4Words
    P4WORDS(compiler),
    P4WORDS(core),
    P4WORDS(debug),
    P4WORDS(exception),
    P4WORDS(facility),
    P4WORDS(file),
#if defined PF_WITH_FLOATING
    P4WORDS(floating),
#endif
    P4WORDS(interpret),
    P4WORDS(shell),
    P4WORDS(signals),
    P4WORDS(terminal),
    P4WORDS(tools),
    P4WORDS(version);

/************************************************************************/
/**
 * note the argument
 */
void pf_init_system (p4_Thread* th) /* main_init */
{
    long int total_size = TOTAL_SIZE;
    long int stack_size = (TOTAL_SIZE / 32 + 256)  / sizeof(p4cell); 
    long int ret_stack_size = (TOTAL_SIZE / 64 + 256) / sizeof(p4cell);
#if defined PF_WITH_FLOATING
    long int float_stack_size = (TOTAL_SIZE / 32) / sizeof(double);
#endif

    setlocale (LC_ALL, "C");
    /* ............................................................*/
    p4TH = th;
    pf_init_terminal();
    pf_init_signal_handlers();

    /* _______________ dictionary block __________________ */

    dict = calloc (1, (size_t) total_size);
    if (dict == NULL) {
            printf("[%p] FAILED to alloc any base memory (len %lu): %s\n",
		      p4TH, total_size, strerror(errno));
            puts ("ERROR: out of memory");
	    exit(6);
    }

    /* ________________ initialize dictionary _____________ */

    dictlimit = dict + total_size;

    p4_dict_allocate (ret_stack_size, sizeof(p4xt*),
                      SIZEOF_CELL,
                      (void**) & PFE.rstack, (void**) & R0);
    p4_dict_allocate (stack_size, sizeof(p4cell),
                      SIZEOF_CELL,
                      (void**) & PFE.stack, (void**) & S0);
#if defined PF_WITH_FLOATING
    p4_dict_allocate (float_stack_size, sizeof(double),
                      SIZEOF_FCELL,
                      (void**) &PFE.fstack, (void**) &F0);
#endif

    if (dictlimit < dict + MIN_PAD + MIN_HOLD + 0x4000)
    {
	puts ("ERROR: impossible memory map");
	exitcode = 3;
	pf_longjmp_exit ();
    }

    /*  -- cold boot stage -- */
    RP = R0;
    SP = S0;
#if defined PF_WITH_FLOATING
    FP = F0;
    PRECISION = 6;
#endif
    BASE = 10;
    /* Wipe the dictionary: */
    memset (dict, 0, (dictlimit - dict));
    DP = dict;
    /* Create first word */
    p4_header_comma ("FORTH", 5);
    FX_XCOMMA ((p4xt)(pf_noop_));
    /* and load other words */
    pf_load_words (&P4WORDS (core));
    pf_load_words (&P4WORDS (compiler));
    pf_load_words (&P4WORDS (interpret));
    pf_load_words (&P4WORDS (file));
    pf_load_words (&P4WORDS (terminal));
    pf_load_words (&P4WORDS (shell));
    pf_load_words (&P4WORDS (exception));
#if defined PF_WITH_FLOATING
    pf_load_words (&P4WORDS (floating));
#endif
    pf_load_words (&P4WORDS (signals));
    pf_load_words (&P4WORDS (tools));
    pf_load_words (&P4WORDS (facility));
    pf_load_words (&P4WORDS (debug));
    pf_load_words (&P4WORDS (version));
}

void help_opt(char *progname)
{
    printf("Usage: %s [-f file] [-e word] [v] [h]\n", progname);
    puts("   -f file  : load file   (before -e option if any)");
    puts("   -e word  : execute word (after -f option if any)");
    puts("   -v       : print version");
    puts("   -h       : print this help");
}
 
/************************************************************************/
/* Here's main()                                                        */
/************************************************************************/
int main (int argc, char** argv)
{
    char cmd = 0;
    char buffer[256];
    int len = 0;
    int opt;
    p4_Thread* thread = (p4_Thread*) memory;
  
    while ((opt = getopt(argc, argv, "e:f:vh")) != -1) {
        switch (opt) {
        case 'e':
        case 'f':
            cmd = opt;
            strcpy( buffer, optarg );
            len = strlen(buffer);
            break;
        case 'v':
//            puts(PF_VERSION);
            puts(pf_version_string());
            return 0;
        case 'h':
            help_opt(argv[0]);
            return 0;
        default: /* '?' */
            help_opt(argv[0]);
            return 1;
        }
    }

    memset (thread, 0, sizeof(p4_Thread));
    /* -------- cold boot stage ------- */
    pf_init_system(thread);
    /* -------- warm boot stage ------- */
    abort_system ();
    quit_system ();
    exitcode = 0;

    switch (setjmp (jump_loop))
    {           /* classify unhandled throw codes */
    case 'A': /* do abort */
        abort_system();
    case 'Q': /* do quit */
        quit_system();
        break;
    case 0:
        pf_include((const char *)PF_BOOT_FILE, strlen(PF_BOOT_FILE) );
        if ( cmd == 'f' )
            pf_include(buffer,len);
        if ( cmd == 'e' ) {
            pf_interpret(buffer, len, 0);
	    pf_longjmp_exit ();
        }
        break;
    default:
        pf_cleanup_terminal();
        free(dict);
    	return exitcode;
    }

    for (;;) {
            if (isatty (STDIN_FILENO))
                pf_outs (" ok\n");
            len = pf_accept (buffer, 255);
            pf_interpret(buffer, len, 0);
            FCode (pf_Q_stack);
    }
//    return exitcode;
}
