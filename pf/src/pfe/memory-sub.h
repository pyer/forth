#ifndef _VOL_8_SRC_CVS_PFE_33_PFE_MEMORY_SUB_H
#define _VOL_8_SRC_CVS_PFE_33_PFE_MEMORY_SUB_H 1209868837
/* generated 2008-0504-0440 /vol/8/src/cvs/pfe-33/pfe/../mk/Make-H.pl /vol/8/src/cvs/pfe-33/pfe/memory-sub.c */

#include <pfe/pfe-sub.h>

/** 
 * -- Memory Allocation Words
 * 
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.5 $
 *     (modified $Date: 2008-05-04 02:57:30 $)
 *
 *  @description
 *     memory allocation interfaces to the surrounding OS.
 */

#ifdef __cplusplus
extern "C" {
#endif




_extern  void * p4_xcalloc (int n_elem, size_t size) /* allocate memory, die when failed */ ; /*{*/

_extern  void * p4_calloc (int n_elem, size_t size) /* allocate memory, with debug info */ ; /*{*/

_extern  void * p4_xalloc (size_t size) /* allocate memory, throw when failed */ ; /*{*/

_extern  void p4_xfree (void* p) ; /*{*/

/**
 * helper routine to allocate a portion of the dictionary
 * especially for some stack-areas of the forth system
 * ... just decreases PFE.dictlimit, returns 0 if impossible.
 */
_extern  void* p4_dict_allocate (int items, int size, int align, void** lower, void** upper) ; /*{*/

_extern  int p4_mmap_creat(char* name, void* addr, long size) ; /*{*/

_extern  void p4_mmap_close(int fd, void* addr, long size) ; /*{*/

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
