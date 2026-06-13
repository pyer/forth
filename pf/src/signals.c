/** 
 * -- Handle signals in forth
 *
 */

#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "config.h"
#include "const.h"
#include "macro.h"

#include "compiler.h"
#include "exception.h"
#include "terminal.h"

/* -------------------------------------------------------------- */
/*
 * Actions to take when job control interferes or on window size change:
 */

static void sigint_handler(int sig)
{
    printf("Ctrl C\nTerminate process %d\n", getpid());
    pf_longjmp_exit();
}

static void sigtstp_handler(int sig)
{
    puts("Ctrl Z");
}

static void noop_handler(int sig)
{
}

#ifdef SIGWINCH
static void sigwinch_handler(int sig)
{
    query_winsize();
}
#endif

#ifdef SIGALRM
#endif

/* -------------------------------------------------------------- */
/**
 * install all signal handlers:
 */
void pf_init_signals (void)
{
    if ( isatty (STDIN_FILENO)) {
      signal (SIGINT,  sigint_handler);
      signal (SIGTSTP, sigtstp_handler);
      signal (SIGTTIN, noop_handler);
      signal (SIGTTOU, noop_handler);

#ifdef SIGWINCH
      signal (SIGWINCH, sigwinch_handler);
#endif
#ifdef SIGALRM
      signal (SIGALRM, noop_handler);
#endif
    }
}

/* -------------------------------------------------------------- */
static void sig_handler(int sig)    /* Signal handler for Forth word */
{
    puts("Forth callback does not work yet !"); /* FIXME: */
}

/* -------------------------------------------------------------- */
/** RAISE ( signal# -- )
 * Send a signal to self.
 */
FCode (pf_raise_signal)
{
    raise(*SP++);
}

/** SIGNAL ( handler-xt* signal# -- )
 * Install signal handler
 */
FCode (pf_install_signal)    
{

    int sig = *SP++;      // signal#
//    p4xt xt = (p4xt)*SP;  // handler-xt*
    SP++;
    signal (sig, sig_handler);
}

/* -------------------------------------------------------------- */
WORDS (signals) =
{
    P4_FXco ("RAISE",  pf_raise_signal),
    P4_FXco ("SIGNAL", pf_install_signal),
    P4_END
};

