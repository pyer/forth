#ifndef _VOL_8_SRC_CVS_PFE_33_PFE_FILE_MIX_H
#define _VOL_8_SRC_CVS_PFE_33_PFE_FILE_MIX_H 1209868837
/* generated 2008-0504-0440 /vol/8/src/cvs/pfe-33/pfe/../mk/Make-H.pl /vol/8/src/cvs/pfe-33/pfe/file-mix.c */

#include <pfe/pfe-mix.h>

/** 
 * -- miscellaneous useful extra words for FILE-EXT
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
 *      Compatiblity with former standards, miscellaneous useful words.
 *      ... for FILE-EXT
 */

#ifdef __cplusplus
extern "C" {
#endif




/** INCLUDE ( "filename" -- ??? ) [FTH]
 * load the specified file, see also => LOAD" filename"
 */
extern P4_CODE (p4_include);

/** COPY-FILE ( src-ptr src-len dst-ptr dst-len -- copy-errno# ) [FTH]
 * like =>'RENAME-FILE', copies the file from src-name to dst-name
 * and returns an error-code or null
 */
extern P4_CODE (p4_copy_file);

/** MOVE-FILE ( src-ptr src-len dst-ptr dst-len -- move-errno# ) [FTH]
 * like =>'RENAME-FILE', but also across-volumes <br>
 * moves the file from src-name to dst-name and returns an
 * error-code or null
 */
extern P4_CODE (p4_move_file);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
