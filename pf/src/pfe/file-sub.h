#ifndef _VOL_8_SRC_CVS_PFE_33_PFE_FILE_SUB_H
#define _VOL_8_SRC_CVS_PFE_33_PFE_FILE_SUB_H 1209868837
/* generated 2008-0504-0440 /vol/8/src/cvs/pfe-33/pfe/../mk/Make-H.pl /vol/8/src/cvs/pfe-33/pfe/file-sub.c */

#include <pfe/pfe-sub.h>

/** 
 *  Subroutines for file access
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.5 $
 *     (modified $Date: 2008-05-04 02:57:30 $)
 */

#ifdef __cplusplus
extern "C" {
#endif

int p4_file_access (const p4_char_t *fn, int len);
p4_File * p4_open_file (const p4_char_t *name, int len, int mode);
p4_File * p4_create_file (const p4_char_t *name, int len, int mode);
int p4_close_file (p4_File *fid);
int p4_reposition_file (p4_File *fid, _p4_off_t pos);
int p4_read_file (void *p, p4ucell *n, p4_File *fid);
int p4_write_file (void *p, p4ucell n, p4_File *fid);
int p4_resize_file (p4_File *fid, _p4_off_t size);
int p4_read_line (void* buf, p4ucell *u, p4_File *fid, p4cell *ior);





_extern  _p4_off_t p4_file_size (FILE * f) /* Result: file length, -1 on error */ ; /*{*/

_extern  _p4_off_t p4_file_copy (const char *src, const char *dst, _p4_off_t limit) /* * Copies file, but at most limit characters. * Returns destination file length if successful, -1 otherwise. */ ; /*{*/

_extern  int p4_file_move (const char *src, const char *dst) ; /*{*/

_extern  int p4_file_resize (const char *fn, _p4_off_t new_size) ; /*{*/

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
