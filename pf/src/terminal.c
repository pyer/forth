/**
 *  TERMINAL
 *
 *  @description
 *      The Terminal Wordset contains the console I/O
 */
/*
Forth83
BLOCK  BUFFER  CR  EMIT  EXPECT  FLUSH  KEY  SAVE-BUFFERS  
SPACE  SPACES  TYPE  UPDATE 

PF
CR  EMIT  EXPECT  FLUSH  KEY  SPACE  SPACES  TYPE
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>

#include "config.h"
#include "types.h"
#include "macro.h"
#include "listwords.h"
#include "session.h"
#include "terminal.h"

/************************************************************************/
int cols  = 144;
int rows  = 39;
int outs  = 0;
int lines = 0;
int more  = 39;
/************************************************************************/
int get_cols(void)
{
    return cols;
}
FCode (pf_cols)
{
    *--SP = cols;
}

int get_rows(void)
{
    return rows;
}
FCode (pf_rows)
{
    *--SP = rows;
}

int get_outs(void)
{
    return outs;
}

/************************************************************************/
void pf_init_terminal(void)
{
cols = 144;
rows = 39;
#ifdef TIOCGSIZE
    struct ttysize ts;
    ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
    cols = ts.ts_cols;
    rows = ts.ts_lines;
#elif defined(TIOCGWINSZ)
    struct winsize ws;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    cols = ws.ws_col;
    rows = ws.ws_row;
/*
#else
    if (getenv("COLUMNS")!=NULL)
        cols = atoi(getenv("COLUMNS"));
    if (getenv("LINES")!=NULL)
        rows = atoi(getenv("LINES"));
*/
#endif /* TIOCGSIZE */
}

/************************************************************************/
static void pf_putc(char c) { fputc(c, stdout); PFE.out++; }
static void pf_flush(void) { fflush (stdout); }
static void pf_puts(const char* s) { fputs (s, stdout); }

/************************************************************************/

void pf_outc (char c)
{
    pf_putc(c);
    pf_flush();
}

void pf_outs (const char *str)
{
    char* s = (char*) str;
    while (*s)
        pf_putc(*s++);
    pf_flush();
}

int pf_outf (const char *s,...)
{
    char buf[512];
    va_list p;
    int r;

    va_start (p, s);
    r = vsprintf (buf, s, p);
    pf_outs (buf);
    va_end (p);
    return r;
}

/** KEY ( -- char# ) [ANS]
 * return a single character from the keyboard - the
 * key is not echoed.
 */
//extern P4_CODE(pf_key);
int  pf_getkey (void)
{
    return fgetc(stdin);
}

FCode (pf_key)
{
    *--SP = pf_getkey();
}

/* ********************************************************************** */
int pf_accept (char *tib, int tiblen)
{
    if ( fgets (tib, tiblen, stdin) == NULL )
       return 0;
    return strlen(tib);
}

/** ACCEPT ( buffer-ptr buffer-max -- buffer-len ) [ANS]
 * get a string from terminal into the named input
 * buffer, returns the number of bytes being stored
 * in the buffer. May provide line-editing functions.
 */
FCode (pf_accept)
{
    SP[1] = pf_accept ((char *)SP[1], SP[0]);
    SP += 1;
}

/* ********************************************************************** */

/** EMIT ( char# -- ) [ANS]
 * print the char-value on stack to stdout
 */
FCode (pf_emit)
{
    char c = (*SP++);
    pf_outc (c);
}

/** EMITS ( n# ch -- )
 * type a string of chars by repeating a single character which
 * is usually a space, see => SPACES
 * (output adjusting the OUT variable, see => _type_ and => _outs_ )
 : _emits_ swap 0 do dup _putc_ loop drop _flush_ _?xy_ drop out ! ;
 */
void pf_emits(int n, const char c)
{
    while (--n >= 0)
        pf_putc(c);
    pf_flush();
}

FCode (pf_emits)
{
    char c = (*SP++);
    int n = (*SP++);

    while (--n >= 0)
        pf_putc(c);
    pf_flush();
}

/** CR ( -- ) [ANS]
 * print a carriage-return/new-line on stdout
 */
FCode (pf_cr)
{
    pf_putc('\n');
    pf_flush();
    outs = 0;
    lines++;
}

/** _type_ ( str* str# -- )
 * type counted string to terminal
 * (output adjusting the => OUT variable, see => _puts_ and => _outs_ )
 : _type_ 0 do c@++ _putc_ loop drop _flush_ _?xy drop out ! ;
 */
void pf_type (const char *str, p4cell len)
{
    while (--len >= 0)
        pf_putc(*str++);
    pf_flush();
}

/** TYPE ( string-ptr string-len -- ) [ANS]
 * prints the string-buffer to stdout, see => COUNT and => EMIT
 */
FCode (pf_type)
{
    p4cell n = (*SP++);
    const char *s = (const char *)(*SP++);
    pf_type(s,n);
}


/** BACKSPACE ( -- ) [FTH]
 * reverse of => SPACE
 */
FCode (pf_backspace)
{
    pf_puts ("\b \b");
    pf_flush();
}

/** SPACE ( -- ) [ANS]
 * print a single space to stdout, see => SPACES
 simulate:    : SPACE  BL EMIT ;
 */
FCode(pf_space)
{
    pf_putc(' ');
    pf_flush();
}

/** SPACES ( space-count -- ) [ANS]
 * print n space to stdout, actually a loop over n calling => SPACE ,
 * but the implemenation may take advantage of printing chunks of
 * spaces to speed up the operation.
 */
FCode(pf_spaces)
{
    int n = (*SP++);
    while (--n >= 0)
        pf_putc(' ');
    pf_flush();
}

/** TAB ( tab-n# -- ) [FTH]
 * jump to next column divisible by n
 */
void pf_tab (int n)
{
    pf_emits (n - outs % n, ' ');
}

FCode (pf_tab)
{
    pf_tab (*SP++);
}

/* -------------------------------------------------------------- */
/** MORE ( -- )
 * initialized for more-like effect
 * - see => MORE?
 */
FCode (pf_more)
{
    more = rows - 2;
    lines = 0;
}

/** _more?_ ( -- ?stopped )
 * Like CR but stop after one screenful and return flag if 'q' pressed.
 * Improved by aph@oclc.org (Andrew Houghton)
 */
int pf_more_Q (void)
{
    static char help[] = "\r[next line=<return>, next page=<space>, quit=q] ";

    pf_cr_();
    if (lines < more)
        return 0;
    lines = 0;
    for (;;)
    {
        pf_outs ("more? ");
        register int ch = pf_getkey();
        switch (ch)
	{
         case 'n':		/* no more */
         case 'N':
         case 'q':		/* quit    */
         case 'Q':
             return 1;
         case 'y':		/* more    */
         case 'Y':
         case ' ':		/* page    */
             more = rows - 1;
             return 0;
         case '\r':		/* line    */
         case '\n':		/* line    */
             more = 1;
             return 0;
         default:		/* unknown */
             pf_bell();
             /* ... */
         case '?':		/* help    */
         case 'h':		/* help    */
         case 'H':
             pf_outs (help);
             break;
	}
    }
}

/** MORE? ( -- cr-flag )
 */
FCode (pf_more_Q)
{
    *--SP = pf_more_Q();
}

/* -------------------------------------------------------------- */
/*
SPAN
CR  EMIT  EXPECT  FLUSH  KEY  SPACE  SPACES  TYPE

Forth200x
This standard removes six words that were marked 'obsolescent' in the ANS Forth '94 document. These are:
6.2.0060	#TIB		6.2.1390	EXPECT		6.2.2240	SPAN
6.2.0970	CONVERT		6.2.2040	QUERY		6.2.2290	TIB

Words affected:
#TIB, CONVERT, EXPECT, QUERY, SPAN, TIB, WORD.
Reason:
Obsolescent words have been removed.
Impact:
WORD is no longer required to leave a space at the end of the returned string.
It is recommended that, should the obsolete words be included, they have the behaviour described in
Forth 94. The names should not be reused for other purposes.
Transition/Conversion:
The functions of TIB and #TIB have been superseded by SOURCE.
The function of CONVERT has been superseded by >NUMBER.
The functions of EXPECT and SPAN have been superseded by ACCEPT.
The function of QUERY may be performed with ACCEPT and EVALUATE.


 */

P4_LISTWORDS (terminal) =
{
//    P4_INTO ("FORTH", 0),
    P4_FXco ("COLS",         pf_cols),
    P4_FXco ("ROWS",         pf_rows),

    P4_FXco ("KEY",          pf_key),
    P4_FXco ("ACCEPT",       pf_accept),

    P4_FXco ("CR",           pf_cr),
    P4_FXco ("EMIT",         pf_emit),
    P4_FXco ("EMITS",        pf_emits),
    P4_FXco ("TYPE",         pf_type),
    P4_FXco ("BACKSPACE",    pf_backspace),
    P4_FXco ("SPACE",        pf_space),
    P4_FXco ("SPACES",       pf_spaces),
    P4_FXco ("TAB",          pf_tab),

    P4_FXco ("MORE",         pf_more),
    P4_FXco ("MORE?",        pf_more_Q),
};
P4_COUNTWORDS (terminal, "Terminal words");

