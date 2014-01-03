#ifndef __PFE_DEF_PATHS_H 
#define __PFE_DEF_PATHS_H
/** 
 * -- path style defines
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
 *             System location on disk and file name conventions,
 *             detect from PFE_PATH_STYLE ; used in pfe sources...
 */
/*@{*/

#include <pfe/def-config.h>

#ifndef PFE_VARIANT
#define PFE_VARIANT ""
#endif
#ifndef PFE_PACKAGE
#define PFE_PACKAGE "pf"
#endif
#define PFE_PKGVARIANT PFE_PACKAGE PFE_VARIANT


#if defined ARM
#define PFE_BOOT_FILE "/etc/pf.4th"
#else
#define PFE_BOOT_FILE "/usr/local/etc/pf.4th"
#endif

#if PFE_PATH_STYLE & 16	
/* MS-DOS like file and path names */

#if defined HOST_OS_WATCOM
# define _STR_(X) #X
#else
# define _STR_(X) P4STRING(X)
#endif

# define PFE_PATH_DELIMITER	';'
# define PFE_DIR_DELIMITER	'\\'
# define PFE_DIR_DELIMSTR       "\\"
# define PFE_LLCMD		"DIR"
# define PFE_LSCMD		"DIR /W"

#elif PFE_PATH_STYLE & 32	
/* UNIX-like file and path names */

# define PFE_PATH_DELIMITER	':'
# define PFE_DIR_DELIMITER	'/'
# define PFE_DIR_DELIMSTR       "/"
# define PFE_LLCMD		"ls -alF"
# define PFE_LSCMD		"ls -C"

#elif PFE_PATH_STYLE & 64	
/* WEB-like file and path names */

# define PFE_PATH_DELIMITER	';'
# define PFE_DIR_DELIMITER	'/'
# define PFE_DIR_DELIMSTR       "/"
# define PFE_LLCMD		"ls -alF"
# define PFE_LSCMD		"ls -C"

#else
/* UNKNOWN file and path names */

#error "Don't know what kind of file names your system uses, check pfe/_config.h"

#endif

#ifndef PFE_DEF_PATH_MAX
#define PFE_DEF_PATH_MAX        256
#endif

/*@}*/
#endif 
