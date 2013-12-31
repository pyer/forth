/** 
 * -- Stackhelp for Optional File-Access Word Set
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
 *       The Optional File-Access Word Set and
 *       File-Access Extension Words.
 *
 */
/*@{*/
#if defined(__version_control__) && defined(__GNUC__)
static char* id __attribute__((unused)) = 
"@(#) $Id: file-stk.c,v 1.3 2008-04-20 04:46:30 guidod Exp $";
#endif

#define _P4_SOURCE 1

#include <pfe/pfe-base.h>
#include <pfe/stackhelp-ext.h>

P4_LISTWORDS (file_check) =
{
    P4_STKi ("BIN",		"access-mode# -- access-mode#'"),
    P4_STKi ("CLOSE-FILE",	"file* -- errno#"),
    P4_STKi ("CREATE-FILE",	"str-ptr str-len open-mode# -- file* errno#"),
    P4_STKi ("DELETE-FILE",	"str-ptr -str-len -- errno#"),
    P4_STKi ("FILE-POSITION",	"file* -- p,pos# errno#"),
    P4_STKi ("FILE-SIZE",	"file* -- s,size# errno#"),
    P4_STKi ("INCLUDE-FILE",	"file* --"),
    P4_STKi ("INCLUDED",	"str-ptr str-len"),
    P4_STKi ("OPEN-FILE",	"str-ptr str-len open-mode# -- file* errno#"),
    P4_STKi ("R/O",		"-- readonly-mode#"),
    P4_STKi ("R/W",		"-- readwrite-mode#"),
    P4_STKi ("READ-FILE",    "str-ptr str-len file* -- count errno#"),
    P4_STKi ("READ-LINE",    "str-ptr str-len file* -- count flag? errno#"),
    P4_STKi ("REPOSITION-FILE",	"o,offset# file* -- errno#"),
    P4_STKi ("RESIZE-FILE",	"s,size file* -- errno#"),
    P4_STKi ("W/O",		"-- writeonly-mode#"),
    P4_STKi ("WRITE-FILE",	"str-ptr str-len file* -- errno#"),
    P4_STKi ("WRITE-LINE",	"str-ptr str-len file* -- errno#"),
    P4_STKi ("FILE-STATUS",	"str-ptr str-len -- code# errno#"),
    P4_STKi ("FLUSH-FILE",	"file* -- errno#"),
    P4_STKi ("RENAME-FILE",  "str1-ptr str1-len str2-ptr str2-len -- errno#"),
    
    /* file-mix */
    P4_STKi ("INCLUDE",		"<name> -- ???"),

    P4_STKi ("COPY-FILE",	"src-ptr src-len dst-ptr dst-len -- errno#"),
    P4_STKi ("MOVE-FILE",	"src-ptr str-len dst-ptr dst-len -- errno#"),

    P4_INTO ("ENVIRONMENT", 0),
    P4_STKi ("FILE-EXT",        "-- year# true! | 0"),
    P4_STKi ("MAX-FILES",       "-- filesmax#"),
};
P4_COUNTWORDS (file_check, "Check-File-access + extensions");

/*@}*/

