/** 
 * -- os-like / shell-like commands for pfe
 * 
 *  Copyright (C) Tektronix, Inc. 1998 - 2001. 
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.4 $
 *     (modified $Date: 2008-05-02 03:03:35 $)
 *
 *  @description
 *        These builtin words are modelled after common shell commands,
 *        so that the Portable Forth Environment can often 
 *        be put in the place of a normal OS shell.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <ctype.h>
#include <setjmp.h>
#include <errno.h>

//#include <direct.h> /* getcwd, mkdir */

#include <dirent.h>
//#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <time.h>

#include "config.h"
#include "types.h"
#include "const.h"
#include "macro.h"
#include "listwords.h"
#include "session.h"

#include "compiler.h"
#include "exception.h"
#include "interpret.h"
#include "terminal.h"

/* ----------------------------------------------------------------------- */
#define P4_ARG_MAX 4096

/* UNIX-like file and path names */
# define PFE_PATH_DELIMITER	':'
# define PFE_DIR_DELIMSTR       "/"
# define PFE_LLCMD		"ls -alF"
# define PFE_LSCMD		"ls -C"


/* ----------------------------------------------------------------------- */
typedef int (*syscall_f)( const char* ); 
		/*GD* used in do_one, so we don't get warnings */

/* ----------------------------------------------------------------------- */
char current_dir[PATH_LENGTH];
char buffer1[PATH_LENGTH];
char buffer2[PATH_LENGTH];
/* ----------------------------------------------------------------------- */
/** $PID ( -- pid )
 * calls system's <c> getpid </c>
 */
FCode (p4_getpid)	{ *--SP = (p4cell)getpid (); }

/** $UID ( -- val )
 * calls system's <c> getuid </c>
 */
FCode (p4_getuid)	{ *--SP = (p4cell)getuid (); }

/** $EUID ( -- val )
 * calls system's <c> geteuid </c>
 */
FCode (p4_geteuid)	{ *--SP = (p4cell)geteuid (); }

/** $GID ( -- val )
 * calls system's <c> getgid </c>
 */
FCode (p4_getgid)	{ *--SP = (p4cell)getgid (); }

/** UMASK ( val -- ret )
 * calls system's <c> umask </c>
 */
FCode (p4_umask)	{ *SP = (p4cell)umask (*SP); }

/** _strpush_ ( zstr* -- S: str* str# )
 * push a C-string onto the SP runtime-stack, as if => S" string" was used
 : _strpush_ s! _strlen_ s! ;
 */
void p4_strpush (const char *s)
{
    if (s) {
        *--SP = (p4cell)s; *--SP = strlen (s);
    } else {
        *--SP = 0; *--SP = 0;
    }
}

/** $HOME ( -- str-ptr str-len )
 * calls system's <c> getenv(HOME) </c>
 */
FCode (p4_home)	{ p4_strpush (getenv ("HOME")); }

/** $USER ( -- str-ptr str-len )
 * calls system's <c> getenv(USER) </c>
 */
FCode (p4_user)	{ p4_strpush (getenv ("USER")); }

/** $CWD ( -- str-ptr str-len )
 * calls system's <c> getcwd </c>
 */
FCode (p4_cwd)	{ p4_strpush (getcwd (current_dir, PATH_LENGTH)); }

/** PWD ( -- )
 * calls system's <c> getcwd </c> and prints it to the screen
 : PWD  $CWD TYPE ;
 */
FCode (p4_pwd)
{
    pf_outs (getcwd (current_dir, PATH_LENGTH));
    pf_space_();
}

/* ---------------------------------------- shell word helper macros ----- */
static char * pf_word_comma(void)
{
    char *p = pf_word(' ');
    DP += *((p4char*)(p)) + 1;   // add word length + 1
    pf_align_();
    return p;
}

#ifdef S_IRUSR
# ifdef S_IWGRP
# define RWALL	(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
# else
# define RWALL  (S_IRUSR | S_IWUSR)
# endif
# ifdef S_IXGRP
# define RWXALL	(RWALL | S_IXUSR | S_IXGRP | S_IXOTH)
# else
# define RWXALL (RWALL | S_IXUSR)
# endif
#else
# define RWALL	0666
# define RWXALL	0777
#endif

/* ----------------------------------------------------------------------- */
FCode (pf_remove_execution)
{
    char *p = (char *)IP;
    strncpy( buffer1, p+1, (int)(*p) );
    if (remove(buffer1)==-1)
        p4_throwstr (FX_IOR, buffer1);
    FX_SKIP_STRING;
}

FCode (pf_remove)
{
    if (STATE)
    {
        p4_Semant pf_remove_Semant;
        FX_ZCOMMA(&pf_remove_Semant.exec[0]);
        pf_word_comma ();
    }
    else
    {
        char *p = pf_word (' ');
        if (remove(p+1)==-1)
             p4_throwstr (FX_IOR, p);
    }
}

P4COMPILES (pf_remove, pf_remove_execution, P4_SKIPS_CELL, P4_DEFAULT_STYLE);

/* ----------------------------------------------------------------------- */
static int touch (const char *s)
{
    int result;
    if (access (s, F_OK) == 0)
        return utime (s, NULL);
    result = open (s, O_WRONLY | O_CREAT, RWALL);
    if (result < 0)
        return result;
    close (result);
    return 0;
}

FCode (pf_touch_execution)
{
    char *p = (char *)IP;
    strncpy( buffer1, p+1, (int)(*p) );
    if (touch(buffer1)==-1)
        p4_throwstr (FX_IOR, buffer1);
    FX_SKIP_STRING;
}

FCode (pf_touch)
{
    if (STATE)
    {
        p4_Semant pf_touch_Semant;
        FX_ZCOMMA(&pf_touch_Semant.exec[0]);
        pf_word_comma ();
    }
    else
    {
        char *p = pf_word (' ');
        if (touch(p+1)==-1)
             p4_throwstr (FX_IOR, p);
    }
}

P4COMPILES (pf_touch, pf_touch_execution, P4_SKIPS_CELL, P4_DEFAULT_STYLE);

/* ----------------------------------------------------------------------- */
FCode (pf_chdir_execution)
{
    char *p = (char *)IP;
    strncpy( buffer1, p+1, (int)(*p) );
    if (chdir(buffer1)==-1)
        p4_throwstr (FX_IOR, buffer1);
    FX_SKIP_STRING;
}

FCode (pf_chdir)
{
    if (STATE)
    {
        p4_Semant pf_chdir_Semant;
        FX_ZCOMMA(&pf_chdir_Semant.exec[0]);
        pf_word_comma ();
    }
    else
    {
        char *p = pf_word (' ');
        if (chdir(p+1)==-1)
             p4_throwstr (FX_IOR, p);
    }
}

P4COMPILES (pf_chdir, pf_chdir_execution, P4_SKIPS_CELL, P4_DEFAULT_STYLE);

/* ----------------------------------------------------------------------- */
FCode (pf_mkdir_execution)
{
    char *p = (char *)IP;
    strncpy( buffer1, p+1, (int)(*p) );
    if (mkdir(buffer1, RWXALL)==-1)
        p4_throwstr (FX_IOR, buffer1);
    FX_SKIP_STRING;
}

FCode (pf_mkdir)
{
    if (STATE)
    {
        p4_Semant pf_mkdir_Semant;
        FX_ZCOMMA(&pf_mkdir_Semant.exec[0]);
        pf_word_comma ();
    }
    else
    {
        char *p = pf_word (' ');
        if (mkdir(p+1, RWXALL)==-1)
             p4_throwstr (FX_IOR, p);
    }
}

P4COMPILES (pf_mkdir, pf_mkdir_execution, P4_SKIPS_CELL, P4_DEFAULT_STYLE);

/* ----------------------------------------------------------------------- */
static int ls (const char* p)
{
    DIR* dir;
    struct dirent* dirent;
    if (*p=='\0') {
       p = getcwd (current_dir, PATH_LENGTH);
    }
    pf_cr_();
    dir = opendir (p);
    if (!dir) return -1;
  
    while ((dirent=readdir(dir)))
    {
        if (dirent->d_name[0] == '.') continue;
        pf_type(dirent->d_name, strlen(dirent->d_name));
        pf_space_();
    }
    return closedir (dir);
}

FCode (pf_ls_execution)
{
    char *p = (char *)IP;
    strncpy( buffer1, p+1, (int)(*p) );
    if (ls(buffer1)==-1)
        p4_throwstr (FX_IOR, buffer1);
    FX_SKIP_STRING;
}

FCode (pf_ls)
{
    if (STATE)
    {
        p4_Semant pf_ls_Semant;
        FX_ZCOMMA(&pf_ls_Semant.exec[0]);
        pf_word_comma ();
    }
    else
    {
        char *p = pf_word (' ');
        if (ls(p+1)==-1)
             p4_throwstr (FX_IOR, p);
    }
}

P4COMPILES (pf_ls, pf_ls_execution, P4_SKIPS_CELL, P4_DEFAULT_STYLE);

/* ----------------------------------------------------------------------- */
static int ll (const char* p)
{
    DIR* dir;
    struct dirent* dirent;
    struct stat st;
    struct tm tm;
    char buf[514];
    
    if (*p=='\0') {
       p = getcwd (current_dir, PATH_LENGTH);
    }
    pf_cr_();
    pf_more_();


    dir = opendir (p);
    if (!dir) return -1;
    
    while ((dirent=readdir(dir)))
    {
        strncpy (buf, p, 255);
        strcat (buf, "/");
        strncat (buf, dirent->d_name, 255);
        stat (buf, &st);
        memcpy (&tm, localtime (&st.st_mtime), sizeof(struct tm));
        
        if (S_ISREG (st.st_mode))
        {
            pf_outf ("%8i  ", st.st_size );
        } else if (S_ISDIR (st.st_mode))
        {
            pf_outf ("DIRECTORY ");
        } else {
            pf_outf ("SPECIAL   ");
        } 
        pf_outf ("%2i-%02i-%04i %2i:%02i:%02i  %s",  
              tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900,
              tm.tm_hour, tm.tm_min, tm.tm_sec,
              dirent->d_name);
        
        if (pf_more_Q()) 
            break;
    }
    return closedir (dir);
}

FCode (pf_ll_execution)
{
    char *p = (char *)IP;
    strncpy( buffer1, p+1, (int)(*p) );
    if (ll(buffer1)==-1)
        p4_throwstr (FX_IOR, buffer1);
    FX_SKIP_STRING;
}

FCode (pf_ll)
{
    if (STATE)
    {
        p4_Semant pf_ll_Semant;
        FX_ZCOMMA(&pf_ll_Semant.exec[0]);
        pf_word_comma ();
    }
    else
    {
        char *p = pf_word (' ');
        if (ll(p+1)==-1)
             p4_throwstr (FX_IOR, p);
    }
}

P4COMPILES (pf_ll, pf_ll_execution, P4_SKIPS_CELL, P4_DEFAULT_STYLE);
/* ----------------------------------------------------------------------- */


/* ----------------------------------------------------------------------- */

/**
 * issue a system() call, after formatting
 */
int p4_systemf (const char *s,...)
{
    char buf[P4_ARG_MAX+1];
    va_list p;
    int r;

    va_start (p, s);
    vsprintf (buf, s, p);
    va_end (p);
//    p4_system_terminal ();
//    p4_swap_signals ();
    r = system (buf);
//    p4_swap_signals ();
//    p4_interactive_terminal ();
//    p4_dot_normal ();
    return r;
}

/* ----------------------------------------------------------------------- */
/** ARGC ( -- arg-count ) [FTH]
FCode (pf_argc)
{
    *--SP = (p4cell)(P4_opt.argc);
}
 */

/** ARGV ( arg-n# -- arg-ptr arg-len ) [FTH]
FCode (pf_argv)
{
    p4ucell n = *SP++;

    if (n < (p4ucell) P4_opt.argc)
        p4_strpush (P4_opt.argv [n]);
    else
        p4_strpush (NULL);
}
 */

/** SYSTEM ( command-ptr command-len -- command-exitcode# ) [FTH]
 * run a shell command  (note: embedded systems have no shell)
 */
FCode (pf_system)
{
    SP[1] = p4_systemf ("%.*s", (int) SP[0], (char *) SP[1]);
    SP++;
}

/* ----------------------------------------------------------------------- */
P4_LISTWORDS (shell) =
{
//    P4_INTO ("FORTH", 0),
    /** ( -- fid ) - the standard file-handles of the task */
//    P4_DVaL ("STDIN",		stdIn),
//    P4_DVaL ("STDOUT",		stdOut),
//    P4_DVaL ("STDERR",		stdErr),

    P4_FXco ("$PID",		p4_getpid),
    P4_FXco ("$UID",		p4_getuid),
    P4_FXco ("$EUID",		p4_geteuid),
    P4_FXco ("$GID",		p4_getgid),
    P4_FXco ("UMASK",		p4_umask),
    P4_FXco ("$HOME",		p4_home),
    P4_FXco ("$USER",		p4_user),
    P4_FXco ("$CWD",		p4_cwd),
    P4_FXco ("PWD",		p4_pwd),
    /** mimics a unix'ish shell-command - =>'PARSE's one filename/dirname */
    P4_FXco ("RM",		pf_remove),
    P4_FXco ("TOUCH",		pf_touch),
    P4_FXco ("CD",		pf_chdir),
    P4_FXco ("MKDIR",		pf_mkdir),
    /** mimics a unix'ish shell-command - =>'PARSE's two filenames/dirnames */
//    P4_FXco ("LN",		p4_link),
    /** mimics a unix'ish shell-command - =>'PARSE's one filename/dirname */
    P4_FXco ("LS",		pf_ls),
    P4_FXco ("LL",		pf_ll),
    /** will invoke a shell-command with the command and a two filenames */
//    P4_SXco ("MV",		p4_mv),
    /** mimics a unix'ish shell-command - =>'PARSE's two filenames/dirname */
//    P4_SXco ("CP",		p4_cp),
    /** task system hooks */
//    P4_FXco ("ARGC",		pf_argc),
//    P4_FXco ("ARGV",		pf_argv),
    P4_FXco ("SYSTEM",		pf_system),
};
P4_COUNTWORDS (shell, "Shell like words");

