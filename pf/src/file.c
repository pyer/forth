/** 
 * FILE ---  Optional File-Access Word Set
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.3 $
 *     (modified $Date: 2008-04-20 04:46:29 $)
 *
 *  @description
 *       The Optional File-Access Word Set and
 *       File-Access Extension Words.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <setjmp.h>
//#include <pwd.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "types.h"
#include "const.h"
#include "macro.h"
#include "listwords.h"
#include "thread.h"

#include "compiler.h"
#include "exception.h"
#include "interpret.h"
#include "terminal.h"

# define PFE_DIR_DELIMITER	'/'

#define CHECKFILE " (did some FILE-OPEN fail?)"

/* ================================================================= */
enum
{
    FMODE_RO = 1, FMODE_WO, FMODE_RW,
    FMODE_ROB, FMODE_WOB, FMODE_RWB
};

# define FMODE_BIN (FMODE_ROB - FMODE_RO)
/* ================================================================= */

long
p4_file_size (FILE * f)		/* Result: file length, -1 on error */
{
    struct stat st;		/* version using fstat() */
    int fh = fileno (f);
    if (fh < 0 || fstat (fh, &st) < 0)
        return -1;
    return st.st_size;
}

static long
file_size (const char *fn)		/* Result: file length, -1 on error */
{
    struct stat st;

    if (stat (fn, &st) != 0)
        return -1;
    return st.st_size;
}

long
p4_file_copy (const char *src, const char *dst, long limit)
/*
 * Copies file, but at most limit characters.
 * Returns destination file length if successful, -1 otherwise.
 */
{
    FILE *f, *g;
    char buf[BUFSIZ];
    size_t n;
    long m;

    if ((f = fopen (src, "rb")) == NULL)
        return -1;
    if ((g = fopen (dst, "wb")) == NULL)
    {
        fclose (f);
        return -1;
    }
    for (m = limit; m; m -= n)
    {
        n = (size_t) (BUFSIZ < m ? BUFSIZ : m);
        n = fread (buf, 1, n, f);
        if (n == 0 || n != fwrite (buf, 1, n, g))
            break;
    }
    n = ferror (f) || ferror (g);
    fclose (f);
    fclose (g);
    return n ? -1 : limit - m;
}

/*
 * Renames or moves file, returns 0 on success, -1 on error.
 */
/*
int
p4_file_move (const char *src, const char *dst)
{
    if (_P4_rename (src, dst) == 0)
        return 0;
    if (p4_file_copy (src, dst, LONG_MAX) != -1)
    {
        return remove (src);
    }else{
        remove (dst);
        return -1;
    }
}
*/

/** make file longer */
static int file_extend (const char *fn, long size)
{
    FILE *f;
    int result = 0;
    long n;
    
    f = fopen (fn, "ab");
    if (f == NULL)
        return -1;

    if (fseek (f, 0, SEEK_END) == 0) {
        for (n = ftell(f); n < size; n++) {
            if (putc (0, f) == EOF) {
                result = -1;
                n = size;
            }
        }
    }

    fclose (f);
    return result;
}

/*
 * Truncates or extends file.
 * Returns 0 if successful, -1 otherwise.
 */
int p4_file_resize (const char *fn, off_t new_size)
{
    off_t old_size;
    
    old_size = file_size (fn);
    if (old_size == -1)
        return -1;
    if (old_size <= new_size)
        return file_extend (fn, new_size);
    else
        return truncate (fn, new_size);
}

/* ********************************************************************** 
 * file interface
 */
//char file_name[PATH_LENGTH];
/* FILENAME_MAX is defined in stdio.h */
char file_name[FILENAME_MAX];

/** _zplaced_filename_ ( str* str# dst* max# -- dst* ) [alias] _store_filename_
 * copy stringbuffer into a field as a zero-terminated filename-string,
 * a shell-homedir like "~username" will be expanded, and the
 * platform-specific dir-delimiter is converted in on the fly ('/' vs. '\\')
 */
char* pf_store_filename (const char* str, int n)
{
    /* RENAME: p4_zplace_filename */
    int s = 0;
    int d;
    int max = PATH_LENGTH-1;
    char* p;
    char* dst = file_name;
    char* src = (char*) str;

    if (!src || !n) { *dst = '\0'; return dst; }

#  if PFE_DIR_DELIMITER == '\\'
#   define PFE_ANTI_DELIMITER '/'
#  else
#   define PFE_ANTI_DELIMITER '\\'
#  endif

# define PFE_HOMEDIR_CHAR '~'

    *dst = '\0';
    if (n && max > n && *src == PFE_HOMEDIR_CHAR)
    {
	s = d = 1;
	while (s < n && d < max && src[s] && src[s] != PFE_DIR_DELIMITER)
	{ dst[d++] = src[s++]; }
	dst[d] = '\0';

	if (s == 1)
	{
	    p = getenv("HOME");
	    if (p && max > strlen(p)) { strcpy (dst, p); }
	    /* else *dst = '\0'; */
	}
	*dst = PFE_DIR_DELIMITER; /* /user/restofpath */
    }
    d = strlen (dst);

    while (d < max && s < n && src[s])
    {
	if (src[s] != PFE_ANTI_DELIMITER)
	    dst[d++] = src[s];
	else
	    dst[d++] = PFE_DIR_DELIMITER;
	s++;
    }
    dst[d] = '\0';
    return dst;
}

/**
 * Return best possible access method,
 * 0 if no access but file exists, -1 if file doesn't exist.
 */
int p4_file_access (const p4_char_t *fn, int len)
{
    char* buf = pf_store_filename ((const char *)fn, len);
    if (access (buf, F_OK) != 0)
        return -1;
    if (access (buf, R_OK | W_OK) == 0)
        return FMODE_RW;
    if (access (buf, R_OK) == 0)
        return FMODE_RO;
    if (access (buf, W_OK) == 0)
        return FMODE_WO;
    return 0;
}

static char open_mode[][4] =	/* mode strings for fopen() */
{
    "r", "r+", "r+",		/* R/O W/O R/W */
    "rb", "r+b", "r+b",		/* after application of BIN */
};

/**
 * open file
 */
FILE * p4_open_file (const p4_char_t *name, int len, int mode)
{
    char mdstr[8];
    mode &= 7;

    pf_store_filename ((const char*)name, len);
    strcpy (mdstr, open_mode[mode - FMODE_RO]);
    return (fopen (file_name, mdstr));
}

/**
 * create file 
 */
FILE * p4_create_file (const p4_char_t *name, int len, int mode)
{
#   define null_AT_fclose(X) { FILE* f = (X); if (!f) goto _null; fclose(f); }

    char* fn;
    FILE *fid;

    fn = pf_store_filename ((const char *)name, len);
    null_AT_fclose (fopen (fn, "wb"));
    fid = p4_open_file (name, len, mode);
    if (fid)
    {
        return fid;
    }else{
        remove (fn);
        return NULL;
    }
#   undef null_AT_fclose
 _null:
    
    if (mode > 256) { /* updec! */
        //P4_fail2 ("%s : %s", fn, strerror(errno));
        pf_outf("\nERROR: file %s, errno %s", fn, strerror(errno));
    } 
    return NULL; 
}

/**
 * read file
 */
int p4_read_file (void *p, p4ucell *n, FILE *fid)
{
    int m;
    errno = 0;
    m = fread (p, 1, *n, fid);
    if (m != (int) *n)
    {
        *n = m;
        return errno;
    }
    else
        return 0;
}

/**
 * write file
 */
int p4_write_file (void *p, p4ucell n, FILE *fid)
{
    return (p4ucell) fwrite (p, 1, n, fid) != n ? errno : 0;
}

/**
 * resize file
 */
int p4_resize_file (FILE *fid, long size)
{
    long pos;

    if (fid == NULL )
        p4_throw (P4_ON_FILE_NEX);

    pos = ftell (fid);
    if (pos == -1)
        return -1;
    
//    fclose (fid->f);
//    r = p4_file_resize (fid->name, size);
//    fid->f = fopen (fid->name, fid->mdstr);
    
    if (pos < size)
        fseek (fid, pos, SEEK_SET);
    else
        fseek (fid, 0, SEEK_END);
    return 0;
}

/**
 * read line
 */
int p4_read_line (void* buf, p4ucell *u, FILE *fid, p4cell *ior)
{
    int c, n; char* p = buf;
    
//    fid->line.pos = ftell (fid->f); /* fixme: the only reference to it!*/
    for (n = 0; (p4ucell) n < *u; n++)
    {
        switch (c = getc (fid))
        {
         default:
             *p++ = c;
             continue;
         case EOF:
             *u = n;
             if (ferror (fid))
                 *ior = errno;
             else
                 *ior = 0;
             return((n>0) ? P4_TRUE : P4_FALSE);
         case '\r':
             c = getc (fid);
             if (c != '\n')
                 ungetc (c, fid);
         case '\n':
             goto happy;
        }
    }
 happy:
    *u = n;
    *ior = 0;
//    fid->n++;
    return P4_TRUE;
}

/* ================================================================= */

/** BIN ( access-mode# -- access-mode#' ) [ANS]
 * modify the give file access-mode to be a binary-mode
 */
FCode (p4_bin)
{
    *SP += FMODE_BIN;
}

/** CLOSE-FILE ( some-file* -- some-errno# ) [ANS]
 * close the file and return the status-code
 */
FCode (p4_close_file)
{
    register FILE *fid = (FILE *) SP[0];
    SP[0] = fclose(fid) ? errno : 0;
}

/** CREATE-FILE ( name-ptr name-len open-mode# -- name-file* name-errno# ) [ANS]
 * create the file with the given name and open
 * it - returns the file id and a status code.
 * A code of zero means success. An existing file
 * of the same name is truncated upon open.
 */
FCode (p4_create_file)
{
    register p4_char_t *fn = (p4_char_t *) SP[2]; /* c-addr, name */
    register p4ucell u = SP[1];	                  /* length of name */
    register p4cell fam = SP[0];                  /* file access mode */
    FILE *fid = p4_create_file (fn, u, fam);
    
    SP += 1;
    SP[1] = (p4cell) fid;
    SP[0] = fid ? 0 : errno;
}

/** DELETE-FILE ( name-ptr name-len -- name-errno# ) [ANS]
 * delete the named file and return a status code
 */
FCode (p4_delete_file)
{
    register char* filename = 
	pf_store_filename ((char*)SP[1], SP[0]) ; /* as asciiz */
    SP += 1;
    SP[0] = remove (filename) ? errno : 0;
}

/** FILE-POSITION ( some-file* -- p,pos# some-errno# ) [ANS]
 * return the current position in the file and
 * return a status code. A code of zero means success.
 */
FCode (p4_file_position)
{
    register FILE *fid = (FILE *) SP[0];	/* file-id */
    register long pos;

    SP -= 2;
    pos = ftell (fid);
    if (pos != -1)
    {
	SP[2] = (p4ucell)(pos);
	if (sizeof (*SP) >= sizeof(pos)) /* compile-time decision !*/
	    SP[1] = 0;
	else                            /* assume: 1x or 2x sizeof(*SP) */
	    SP[1] = (p4ucell)(pos >> 8*(sizeof(pos)-sizeof(*SP)));
        SP[0] = 0;		/* ior */
    }else{
	SP[2] = (p4ucell)-1;      /* set to -1 */
	SP[1] = (p4ucell)-1;
        SP[0] = errno;	/* ior */
    }
}

/** FILE-SIZE ( some-file* -- s,size# some-errno# ) [ANS]
 * return the current size of the file and
 * return a status code. A code of zero means success.
 */
FCode (p4_file_size)
{
    FILE *fid = (FILE *) SP[0];	/* fileid */
    long size;

    size = p4_file_size (fid);
    SP -= 2;
    if (size != -1)
    {
	SP[2] = (p4ucell)(size);
	if (sizeof (*SP) >= sizeof(size)) /* compile-time decision !*/
	    SP[1] = 0;
	else                            /* assume: 1x or 2x sizeof(*SP) */
	    SP[1] = (p4ucell)(size >> 8*(sizeof(size)-sizeof(*SP)));
        SP[0] = 0;		/* ior */
    }else{
	SP[2] = (p4ucell)-1;      /* set to -1 */
	SP[1] = (p4ucell)-1;
        SP[0] = errno;	/* ior */
    }
}


/** OPEN-FILE ( name-ptr name-len open-mode# -- name-file* name-errno# ) [ANS]
 * open the named file with mode. returns the
 * file id and a status code. A code of zero
 * means success.
 */
FCode (p4_open_file)
{
    register p4_char_t *fn = (p4_char_t *) SP[2]; /* c-addr, name */
    register p4ucell u = SP[1];	                  /* length of name */
    register p4cell fam = SP[0];                  /* file access mode */
    register FILE *fid = p4_open_file (fn, u, fam);

    SP += 1;
    SP[1] = (p4cell) fid;
    SP[0] = fid ? 0 : errno;
}

/** READ-FILE ( buf-ptr buf-len some-file* -- buf-count some-errno# ) [ANS]
 * fill the given string buffer with characters
 * from the buffer. A status code of zero means
 * success and the returned count gives the
 * number of bytes actually read. If an error
 * occurs the number of already transferred bytes 
 * is returned.
 */
FCode (p4_read_file)
{
    register p4_char_t *  buf = (p4_char_t *) SP[2];
    register p4ucell len = SP[1];
    register FILE *  fid = (FILE *) SP[0];
    SP += 1;
    SP[1] = len;
    SP[0] = p4_read_file (buf, ((p4ucell*)SP) + 1, fid);
}

/** READ-LINE ( buf-ptr buf-len some-file* -- buf-count buf-flag some-errno# ) [ANS]
 * fill the given string buffer with one line
 * from the file. A line termination character
 * (or character sequence under WIN/DOS) may
 * also be placed in the buffer but is not
 * included in the final count. In other respects
 * this function performs a => READ-FILE
 */
FCode (p4_read_line)
{
    register p4_char_t *  buf = (p4_char_t *) SP[2];
    register p4ucell len = SP[1];
    register FILE *  fid = (FILE *) SP[0];
    SP[2] = len;
    SP[1] = p4_read_line (buf, ((p4ucell*)SP) + 2, fid, & SP[0]);
}

/** REPOSITION-FILE ( o,offset# some-file* -- some-errno# ) [ANS]
 * reposition the file offset - the next => FILE-POSITION
 * would return o.offset then. returns a status code where zero means success.
 */
FCode (p4_reposition_file)
{
    register FILE *  fid = (FILE *) SP[0];
    register long pos;
    if (sizeof (*SP) >= sizeof(pos))  /* compile-time decision !*/
    {
	pos = SP[2];
    } else
    {
	pos = (p4ucell) SP[1];
	pos <<= 8*(sizeof(pos)-sizeof(*SP)); /* assume: 1x or 2x sizeof(*SP) */
	pos |=  (p4ucell)(SP[2]);
    }

    SP += 2;
    SP[0] = fseek (fid, pos, SEEK_SET) ? errno : 0;
}

/** RESIZE-FILE ( s,size# some-file* -- some-errno# ) [ANS]
 * resize the give file, returns a status code where zero means success.
 */
FCode (p4_resize_file)
{
    register FILE *  fid = (FILE *) SP[0];
    register long size;
    if (sizeof (*SP) >= sizeof(size))  /* compile-time decision !*/
    {
	size = SP[2];
    } else
    {
	size = (p4ucell) SP[1];
	size <<= 8*(sizeof(size)-sizeof(*SP)); /* assume: 1x or 2x size(*SP) */
	size |=  (p4ucell)(SP[2]);
    }

    SP += 2;
    if (p4_resize_file (fid, size) != 0)
        *SP = errno;
    else
        *SP = 0;
}

/** WRITE-FILE ( buf-ptr buf-len some-file* -- some-errno# ) [ANS]
 * write characters from the string buffer to a file,
 * returns a status code where zero means success.
 */ 
FCode (p4_write_file)
{
    register char *  buf = (char *) SP[2];
    register p4ucell len = SP[1];
    register FILE *  fid = (FILE *) SP[0];

    SP += 2;
    SP[0] = p4_write_file (buf, len, fid);
}

/** WRITE-LINE ( buf-ptr buf-len some-file* -- some-errno# ) [ANS]
 * write characters from the string buffer to a file,
 * and add the line-terminator to the end of it.
 * returns a status code.
 */
FCode (p4_write_line)
{
    register char *  buf = (char *) SP[2];
    register p4ucell len = SP[1];
    register FILE *  fid = (FILE *) SP[0];

    SP += 2;
    if (! (SP[0] = p4_write_file (buf, len, fid)))
        putc ('\n', fid);
}

/** FILE-STATUS ( file-ptr file-len -- file-subcode# file-errno# ) [ANS]
 * check the named file - if it exists
 * the status errno code is zero. The status subcode
 * is implementation-specific and usually matches the
 * file access permission bits of the filesystem.
 */
FCode (p4_file_status)
{
    register int mode = p4_file_access ((p4_char_t *) SP[1], SP[0]);

    if (mode == -1)
    {
        SP[1] = 0;
        SP[0] = errno;
    }else{
        SP[1] = mode;
        SP[0] = 0;
    }
}

/** FLUSH-FILE ( some-file* -- some-errno# ) [ANS]
 * flush all unsaved buffers of the file to disk.
 * A status code of zero means success.
 */
FCode (p4_flush_file)
{
    register FILE *fid = (FILE *) SP[0];

    if (fflush (fid))
	SP[0] = errno;
    else
	SP[0] = 0;
}

/** RENAME-FILE ( oldname-ptr oldname-len newname-ptr newname-len -- newname-errno# ) [ANS]
 * rename the file named by "oldname" to the name of "newname"
 * returns a status-code where zero means success.
 */
/*
FCode (p4_rename_file)
{
    register char* oldnm;
    register char* newnm;

    oldnm = p4_pocket_filename ((char*) SP[3], SP[2]);
    newnm = p4_pocket_filename ((char *) SP[1], SP[0]);
    SP += 3;
    *SP = _P4_rename (oldnm, newnm) ? errno : 0;
}
*/

/** R/O ( -- readonly-mode# ) [ANS]
 * a bitmask for => OPEN-FILE ( => R/O => R/W => W/O => BIN )
 */

/** W/O ( -- writeonly-mode# ) [ANS]
 * a bitmask for => OPEN-FILE or => CREATE-FILE ( => R/O => R/W => W/O => BIN )
 */

/** R/W ( -- readwrite-mode# ) [ANS]
 * a bitmask for => OPEN-FILE or => CREATE-FILE ( => R/O => R/W => W/O => BIN )
 */

/** "ENVIRONMENT MAX-FILES" ( -- files-max ) [ENVIRONMENT]
 * the number of opened file-ids allowed during compilation.
 * portable programs can check this with => ENVIRONMENT?
 */

P4_LISTWORDS (file) =
{
//    P4_INTO ("[ANS]", 0),
    P4_FXco ("BIN",		 p4_bin),
    P4_FXco ("CLOSE-FILE",	 p4_close_file),
    P4_FXco ("CREATE-FILE",	 p4_create_file),
    P4_FXco ("DELETE-FILE",	 p4_delete_file),
    P4_FXco ("FILE-POSITION",	 p4_file_position),
    P4_FXco ("FILE-SIZE",	 p4_file_size),
    P4_FXco ("OPEN-FILE",	 p4_open_file),
    P4_OCON ("R/O",		 FMODE_RO),
    P4_OCON ("R/W",		 FMODE_RW),
    P4_FXco ("READ-FILE",	 p4_read_file),
    P4_FXco ("READ-LINE",	 p4_read_line),
    P4_FXco ("REPOSITION-FILE",	 p4_reposition_file),
    P4_FXco ("RESIZE-FILE",	 p4_resize_file),
    P4_OCON ("W/O",		 FMODE_WO),
    P4_FXco ("WRITE-FILE",	 p4_write_file),
    P4_FXco ("WRITE-LINE",	 p4_write_line),
    P4_FXco ("FILE-STATUS",	 p4_file_status),
    P4_FXco ("FLUSH-FILE",	 p4_flush_file),
//    P4_FXco ("RENAME-FILE",	 p4_rename_file),

};
P4_COUNTWORDS (file, "File-access + extensions");

