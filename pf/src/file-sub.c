/** 
 *  Subroutines for file access
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.3 $
 *     (modified $Date: 2008-04-20 04:46:30 $)
 */
/*@{*/
#if defined(__version_control__) && defined(__GNUC__)
static char* id __attribute__((unused)) = 
"@(#) $Id: file-sub.c,v 1.3 2008-04-20 04:46:30 guidod Exp $";
#endif

#define _P4_SOURCE
//#define _P4_NO_REGS_SOURCE 1

#include <pfe/pfe-base.h>
#include <pfe/def-cell.h>
#include <pfe/def-limits.h>

#include <errno.h>
//#include <pfe/os-string.h>
#include <stdio.h>
#include <stdlib.h>

#include <pfe/logging.h>
#include <pfe/_nonansi.h>
#include <pfe/_missing.h>

#ifdef PFE_HAVE_TRUNCATE
#ifdef PFE_HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif

_export _p4_off_t
p4_file_size (FILE * f)		/* Result: file length, -1 on error */
{
# if defined PFE_HAVE_FSTAT && defined PFE_HAVE_FILENO

    struct stat st;		/* version using fstat() */
    int fh = fileno (f);
    
    if (fh < 0 || fstat (fh, &st) < 0)
        return -1;
    return st.st_size;

# else

    _p4_off_t pos, len;		/* ANSI-C version using fseek()/ftell() */
    
    clearerr (f);
    pos = _p4_ftello (f);
    if (pos == -1)
        return -1;
    if (_p4_fseeko (f, 0, SEEK_END) != 0)
        return -1;
    len = _p4_ftello (f);
    if (_p4_fseeko (f, pos, SEEK_SET) != 0)
    { P4_fatal1 ("could not reset to start position %li", pos); }
    else
    { P4_leave1 ("success after call at file offset %li", pos); }
    return len;
    
# endif
}

static _p4_off_t
file_size (const char *fn)		/* Result: file length, -1 on error */
{
# if defined PFE_HAVE_STAT
    struct stat st;

    if (stat (fn, &st) != 0)
        return -1;
    return st.st_size;
# else
    FILE *f;
    _p4_off_t len;
    
    f = fopen (fn, "r");
    if (f == NULL)
        return -1;
    len = p4_file_size (f);
    fclose (f);
    return len;
# endif
}

_export _p4_off_t
p4_file_copy (const char *src, const char *dst, _p4_off_t limit)
/*
 * Copies file, but at most limit characters.
 * Returns destination file length if successful, -1 otherwise.
 */
{
    FILE *f, *g;
    char buf[BUFSIZ];
    size_t n;
    _p4_off_t m;

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
_export int
p4_file_move (const char *src, const char *dst)
{
    if (_P4_rename (src, dst) == 0)
        return 0;
    if (p4_file_copy (src, dst, LONG_MAX) != -1)
    {
        return _pfe_remove (src);
    }else{
        _pfe_remove (dst);
        return -1;
    }
}

/** make file longer */
static int
_fextend (FILE * f, _p4_off_t size)	
{
    _p4_off_t n;
    
    if (_p4_fseeko (f, 0, SEEK_END) != 0)
        return -1;
    for (n = _p4_ftello (f); n < size; n++)
        if (putc (0, f) == EOF)
            return -1;
    return 0;
}

static int
file_extend (const char *fn, _p4_off_t size)
{
    FILE *f;
    int result;
    
    f = fopen (fn, "ab");
    if (f == NULL)
        return -1;
    result = _fextend (f, size);
    fclose (f);
    return result;
}

#ifndef PFE_HAVE_TRUNCATE
static int
_p4_truncate (const char *path, _p4_off_t length)
{
    char tfn[L_tmpnam];
    _p4_off_t len;
    
    tmpnam (tfn);
    len = p4_file_copy (path, tfn, length);
    if (len == length && _pfe_remove (path) == 0)
    {
        return p4_file_move (tfn, path);
    }else{
        _pfe_remove (tfn);
        return 0;
    }
}
#endif

/*
 * Truncates or extends file.
 * Returns 0 if successful, -1 otherwise.
 */
_export int
p4_file_resize (const char *fn, _p4_off_t new_size)
{
    _p4_off_t old_size;
    
    old_size = file_size (fn);
    if (old_size == -1)
        return -1;
    if (old_size <= new_size)
        return file_extend (fn, new_size);
    else
        return _P4_truncate (fn, new_size);
}

/* ********************************************************************** 
 * file interface
 */

/**
 */
static p4_File *
p4_free_file_slot (void)
{
    p4_File *f;

    for (f = PFE.files; f < PFE.files_top; f++)
        if (f->f == NULL)
        {
            p4_memset (f, 0, sizeof *f);
            return f;
        }
    P4_warn ("not enough file slots in pfe io subsystem");
    return NULL;
}

/**
 * Return best possible access method,
 * 0 if no access but file exists, -1 if file doesn't exist.
 */
_export int
p4_file_access (const p4_char_t *fn, int len)
{
    char* buf = p4_pocket_filename (fn, len);
    if (_P4_access (buf, F_OK) != 0)
        return -1;
    if (_P4_access (buf, R_OK | W_OK) == 0)
        return FMODE_RW;
    if (_P4_access (buf, R_OK) == 0)
        return FMODE_RO;
    if (_P4_access (buf, W_OK) == 0)
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
_export p4_File *
p4_open_file (const p4_char_t *name, int len, int mode)
{
    p4_File *fid;
    mode &= 7;

    fid = p4_free_file_slot ();
    if (fid == NULL) 
        return NULL;
    p4_store_filename (name, len, fid->name, sizeof fid->name);
    fid->mode = mode;
    fid->last_op = 0;
    p4_strcpy (fid->mdstr, open_mode[mode - FMODE_RO]);
    if ((fid->f = fopen (fid->name, fid->mdstr)) == NULL)
        return NULL;
    fid->size = (p4ucell) (p4_file_size (fid->f) / BPBUF);
    fid->n = (p4ucelll)(p4celll) (-1); /* before first line */
    return fid;
}

/**
 * create file 
 */
_export p4_File *
p4_create_file (const p4_char_t *name, int len, int mode)
{
#   define null_AT_fclose(X) { FILE* f = (X); if (!f) goto _null; fclose(f); }

    char* fn;
    p4_File *fid;

    fn = p4_pocket_filename (name, len);
    null_AT_fclose (fopen (fn, "wb"));
    fid = p4_open_file (name, len, mode);
    if (fid)
    {
        return fid;
    }else{
        _pfe_remove (fn);
        return NULL;
    }
#   undef null_AT_fclose
 _null:
    
    if (mode > 256) /* updec! */
    {   P4_fail2 ("%s : %s", fn, strerror(PFE_io_errno));   } 
    return NULL; 
}

/**
 * close file
 */
_export int
p4_close_file (p4_File *fid)
{
    int res = 0;
    
    if (fid->f)
    {
        res = fclose (fid->f);
        p4_memset (fid, 0, sizeof *fid);
    }
    return res;
}

/**
 * seek file
 */
_export int
p4_reposition_file (p4_File *fid, _p4_off_t pos)
{
    fid->last_op = 0;
    return _p4_fseeko (fid->f, pos, SEEK_SET) ? PFE_io_errno : 0;
}

/*
 * Called before trying to read from a file.
 * Checks if you may, maybe fseeks() so you can.
 */
static int
p4_can_read (p4_File *fid)
{
    switch (fid->mode)		/* check permission */
    {
     case FMODE_WO:
     case FMODE_WOB:
         return 0;
    }
    if (fid->last_op < 0)		/* last operation was write? */
        _p4_fseeko (fid->f, 0, SEEK_CUR); /* then seek to this position */
    fid->last_op = 1;
    return 1;
}

/*
 * Called before trying to write to a file.
 * Checks if you may, maybe fseeks() so you can.
 */
static int
p4_can_write (p4_File *fid)
{
    switch (fid->mode)		/* check permission */
    {
     case FMODE_RO:
     case FMODE_ROB:
         return 0;
    }
    if (fid->last_op > 0)		/* last operation was read? */
        _p4_fseeko (fid->f, 0, SEEK_CUR); /* then seek to this position */
    fid->last_op = -1;
    return 1;
}

/**
 * read file
 */
_export int
p4_read_file (void *p, p4ucell *n, p4_File *fid)
{
    int m;

    if (!p4_can_read (fid))
        return EPERM;
    errno = 0;
    m = fread (p, 1, *n, fid->f);
    if (m != (int) *n)
    {
        *n = m;
        return PFE_io_errno;
    }
    else
        return 0;
}

/**
 * write file
 */
_export int
p4_write_file (void *p, p4ucell n, p4_File *fid)
{
    if (!p4_can_write (fid))
        return EPERM;
    errno = 0;
    return (p4ucell) fwrite (p, 1, n, fid->f) != n ? PFE_io_errno : 0;
}

/**
 * resize file
 */
_export int
p4_resize_file (p4_File *fid, _p4_off_t size)
{
    _p4_off_t pos;
    int r;

    if (fid == NULL || fid->f == NULL)
        p4_throw (P4_ON_FILE_NEX);

    pos = _p4_ftello (fid->f);
    if (pos == -1)
        return -1;
    
    fclose (fid->f);
    r = p4_file_resize (fid->name, size);
    fid->f = fopen (fid->name, fid->mdstr);
    
    if (pos < size)
        _p4_fseeko (fid->f, pos, SEEK_SET);
    else
        _p4_fseeko (fid->f, 0, SEEK_END);
    return r;
}

/**
 * read line
 */
_export int
p4_read_line (void* buf, p4ucell *u, p4_File *fid, p4cell *ior)
{
    int c, n; char* p = buf;
    
    if (!p4_can_read (fid))
        return EPERM;
    fid->line.pos = _p4_ftello (fid->f); /* fixme: the only reference to it!*/
    for (n = 0; (p4ucell) n < *u; n++)
    {
        switch (c = getc (fid->f))
        {
         default:
             *p++ = c;
             continue;
         case EOF:
             *u = n;
             if (ferror (fid->f))
                 *ior = PFE_io_errno;
             else
                 *ior = 0;
             return P4_FLAG (n > 0);
         case '\r':
             c = getc (fid->f);
             if (c != '\n')
                 ungetc (c, fid->f);
         case '\n':
             goto happy;
        }
    }
 happy:
    *u = n;
    *ior = 0;
    fid->n++;
    return P4_TRUE;
}

/*@}*/
