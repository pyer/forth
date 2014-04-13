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
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <sys/select.h>
#include <termios.h>
#include <errno.h>

#include "config.h"
#include "types.h"
#include "const.h"
#include "macro.h"
#include "listwords.h"
#include "session.h"
#include "terminal.h"
#include "history.h"

/************************************************************************/
int cols  = 144;
int rows  = 39;
int out   = 0;
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
    return out;
}

/************************************************************************/
static struct termios tty_system;

void pf_init_terminal(void)
{
struct termios new_termios;
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

    /* set the keyboard in raw mode */
    if (isatty (STDIN_FILENO)) {
        /* take two copies - one for now, one for later */
        tcgetattr(STDIN_FILENO, &tty_system);
        memcpy(&new_termios, &tty_system, sizeof(new_termios));
        /* set the new terminal modei: nearly cfmakeraw but  */
//        cfmakeraw(&new_termios);
           new_termios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                           | INLCR | IGNCR | ICRNL | IXON);
           //new_termios.c_oflag &= ~OPOST;
           new_termios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
           new_termios.c_cflag &= ~(CSIZE | PARENB);
           new_termios.c_cflag |= CS8;

        tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    }
    /* init history */
    using_history();
    read_history();
}

void pf_cleanup_terminal (void)
{
    write_history();
    if (isatty (STDIN_FILENO)) {
        tcsetattr(STDIN_FILENO, TCSANOW, &tty_system);
    }
}

/************************************************************************/
static void pf_putc(char c) { fputc(c, stdout); out++; }
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
    out = 0;
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
    pf_emits (n - out % n, ' ');
}

FCode (pf_tab)
{
    pf_tab (*SP++);
}

/* -------------------------------------------------------------- */
/** GOTOXY ( col line -- )
 * move the cursor to the specified position on the screen -
 * this is usually done by sending a corresponding esc-sequence
 * to the terminal. 
 */
FCode (pf_gotoxy)
{
    printf ("\033[%d;%dH", (int)SP[0] + 1, (int)SP[1] + 1);
    SP += 2;
}

FCode (pf_clear_screen)
{
    printf ("\033[2J");
}


/************************************************************************/
/* Input from keyboard.                                                 */
/************************************************************************/

int kbhit(void)
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

/* ********************************************************************** */
/** KEY? ( -- flag )
 * must be in facility.c
 */
FCode (pf_key_question)
{
    if ( kbhit() )
        *--SP = P4_TRUE;
    else
        *--SP = P4_FALSE;
}

/* ********************************************************************** */
#define K_ESC     0x1B
#define K_F1      0x1B5B40
#define K_F12     0x1B5B40

#define K_END     0x1B4F46
#define K_HOME    0x1B4F48

#define K_INSERT  0x1B5B32
#define K_DELETE  0x1B5B33

#define K_PRIOR   0x1B5B35
#define K_NEXT    0x1B5B36

#define K_UP      0x1B5B41
#define K_DOWN    0x1B5B42
#define K_RIGHT   0x1B5B43
#define K_LEFT    0x1B5B44

int pf_escape_sequence(void)
{
    unsigned char c;
    int key = 0x1B;	// Esc
//    sleep(1);
    if ( !kbhit() )
        return key;
    if ( read(0, &c, sizeof(c)) < 0 )
        return -1;
    key = (key<<8) + c;
    if ( !kbhit() )
        return key;
    if ( read(0, &c, sizeof(c)) < 0 )
        return -1;
    key = (key<<8) + c;
    if ( !kbhit() )
        return key;
    if ( c < 0x40 ) {
        if ( read(0, &c, sizeof(c)) < 0 )
            return -1;
    }
    return key;
}

/** KEY ( -- char# ) [ANS]
 * return a single character from the keyboard - the
 * key is not echoed.
 */
int pf_getkey (void)
{
    unsigned char c;
    while (!kbhit()) {
        /* do some work */
    }

    if ( read(0, &c, sizeof(c)) < 0 )
        return -1;
    if ( c == 0x1B ) {
        return pf_escape_sequence();
    }
    return c;
}

FCode (pf_key)
{
    *--SP = pf_getkey();
}

/* ********************************************************************** */
void refresh_line(char *tib, int i, int j)
{
    pf_putc('\r');
    if( i>0 )
        pf_type(tib,i);
    if( j<i ) {
        pf_putc('\r');
        pf_type(tib,j);
    }
    pf_flush();
}

void clear_line(char *tib, int i, int j)
{
    pf_putc('\r');
    while (i-- > 0)
        pf_putc(' ');
}

int pf_accept (char *tib, int tiblen)
{
    HIST_ENTRY * history;
    int j=0;
    int i;
    int x;
    int c;
    for (i = 0; i < tiblen;)
    {
        refresh_line(tib,i,j);
        switch (c = pf_getkey ())
	{
         case K_ESC:
             clear_line(tib,i,j);
             j=i=0;
	     continue;
         case K_LEFT:
             if (j>0) {
                 j--;
             }
	     continue;
         case K_RIGHT:
             if (j<i) {
                 j++;
             }
	     continue;
         case K_UP:
             history = previous_history();
             if( history ) {
               clear_line(tib,i,j);
               strcpy(tib,history->line);
               i=strlen(tib);
               j=i;
             }
	     continue;
         case K_DOWN:
             history = next_history();
             clear_line(tib,i,j);
             if( history ) {
               strcpy(tib,history->line);
               i=strlen(tib);
               j=i;
             } else {
               j=i=0;
             }
	     continue;
         case '\t':
             while (i < tiblen)
             {
                 tib[i++] = ' ';
                 FX (pf_space);
                 if (out % 4 == 0)
                     break;
             }
             continue;
         case K_DELETE:
             if (j<i) {
               clear_line(tib,i,j);
               i--;
               for( x=j; x<i; x++ )
                  tib[x]=tib[x+1];
             }
             continue;
         case 127:
         case '\b':
             if (j>0) {
               clear_line(tib,i,j);
               j--;
               i--;
               for( x=j; x<i; x++ )
                  tib[x]=tib[x+1];
             }
             continue;
         case '\r':
         case '\n':
             goto fin;
         default:
             if( j<i ) {
               for( x=i; x>j; x-- )
                  tib[x]=tib[x-1];
               tib[j++] = c;
               i++;
             } else {
               tib[j++] = c;
               i++;
             }
             continue;
	}
    }
 fin:
    tib[i] = 0;
    if( i>0 )
        add_history (tib);
    return i;
}


/*
int pf_accept_old (char *tib, int tiblen)
{
    if ( fgets (tib, tiblen, stdin) == NULL )
       return 0;
    return strlen(tib);
}
*/

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

/* ----------------------------------------------------------------------- */
/** HISTORY ( -- )
 */
FCode (pf_history)
{
    register HIST_ENTRY **the_list = history_list();
    register int i = 0;

    FX (pf_more);
    while (the_list[i])
    {
        FX (pf_more_Q);
        printf("%d: ",i);
        pf_outs( the_list[i]->line);
        i++;
    }
}

/* ----------------------------------------------------------------------- */
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

    P4_OCoN ("K-LEFT",		K_LEFT),
    P4_OCoN ("K-RIGHT",		K_RIGHT),
    P4_OCoN ("K-UP",		K_UP),
    P4_OCoN ("K-DOWN",		K_DOWN),
    P4_OCoN ("K-HOME",		K_HOME),
    P4_OCoN ("K-END",		K_END),
    P4_OCoN ("K-PRIOR",		K_PRIOR),
    P4_OCoN ("K-NEXT",		K_NEXT),
    P4_OCoN ("K-INSERT",	K_INSERT),
    P4_OCoN ("K-DELETE",        K_DELETE),
/*
    P4_OCoN ("K-F1",			K_k1),
    P4_OCoN ("K-F2",			K_k2),
    P4_OCoN ("K-F3",			K_k3),
    P4_OCoN ("K-F4",			K_k4),
    P4_OCoN ("K-F5",			K_k5),
    P4_OCoN ("K-F6",			K_k6),
    P4_OCoN ("K-F7",			K_k7),
    P4_OCoN ("K-F8",			K_k8),
    P4_OCoN ("K-F9",			K_k9),
    P4_OCoN ("K-F10",			K_k0),
    P4_OCoN ("K-F11",		K_F1),
    P4_OCoN ("K-F12",		K_F2),
*/
    P4_FXco ("KEY",          pf_key),
    P4_FXco ("KEY?",         pf_key_question),
    P4_FXco ("ACCEPT",       pf_accept),
    P4_FXco ("HISTORY",      pf_history),

    P4_FXco ("CR",           pf_cr),
    P4_FXco ("EMIT",         pf_emit),
    P4_FXco ("EMITS",        pf_emits),
    P4_FXco ("TYPE",         pf_type),
    P4_FXco ("BACKSPACE",    pf_backspace),
    P4_FXco ("SPACE",        pf_space),
    P4_FXco ("SPACES",       pf_spaces),
    P4_FXco ("TAB",          pf_tab),
    P4_FXco ("GOTOXY",       pf_gotoxy),
    P4_FXco ("CLEAR-SCREEN", pf_clear_screen),

    P4_FXco ("MORE",         pf_more),
    P4_FXco ("MORE?",        pf_more_Q),
};
P4_COUNTWORDS (terminal, "Terminal words");

