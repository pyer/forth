/** 
 * -- Memory Allocation Words
 * 
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.3 $
 *     (modified $Date: 2008-04-20 04:46:30 $)
 *
 *  @description
 *     memory allocation interfaces to the surrounding OS.
 */
/*@{*/
#if defined(__version_control__) && defined(__GNUC__)
static char* id __attribute__((unused)) = 
"@(#) $Id: memory-sub.c,v 1.3 2008-04-20 04:46:30 guidod Exp $";
#endif

#define _P4_SOURCE 1

#include <pfe/pfe-base.h>
#include <pfe/def-limits.h>

#include <stdlib.h>
#include <stdio.h> /* FIXME: has to be in logging.h */
#include <pfe/os-string.h>
#include <errno.h>
#include <fcntl.h>

#ifdef PFE_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <fcntl.h>

#ifdef PFE_HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <pfe/logging.h>

_export void *
p4_xcalloc (int n_elem, size_t size) /* allocate memory, die when failed */
{
    void *p = calloc (n_elem, size);

    P4_debug3 (13, "xcalloc 0x%p[%i*%lu]", p, n_elem, (unsigned long) size);
    
    if (p == NULL)
    {
        P4_fatal ("out of memory");
	PFE.exitcode = 6;
	p4_longjmp_exit ();
    }
  
    return p;
}

_export void *
p4_calloc (int n_elem, size_t size) /* allocate memory, with debug info */
{
    void *p = calloc (n_elem, size);

    if (p)
    {
	P4_debug3 (13, "calloc 0x%p[%i*%lu]", p, n_elem, (unsigned long)size);
    }else{
        P4_warn2 ("calloc is null[%i*%lu]", n_elem, (unsigned long)size);
    }

    return p;
}

_export void *
p4_xalloc (size_t size)	/* allocate memory, throw when failed */
{
    void *p = calloc (1, size);

    P4_debug2 (13, "xalloc 0x%p[%lu]", p, (unsigned long)size);

    if (p == NULL)
        p4_throw (P4_ON_OUT_OF_MEMORY);

    return p;
}

_export void
p4_xfree (void* p)
{
    P4_debug1 (13, "xfree 0x%p", p);
    free (p);
}

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

