/** 
 * -- miscellaneous useful extra words for FILE-EXT
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.3 $
 *     (modified $Date: 2008-04-20 04:46:31 $)
 *
 *  @description
 *      Compatiblity with former standards, miscellaneous useful words.
 *      ... for FILE-EXT
 */
/*@{*/
#if defined(__version_control__) && defined(__GNUC__)
static char* id __attribute__((unused)) = 
"@(#) $Id: file-mix.c,v 1.3 2008-04-20 04:46:31 guidod Exp $";
#endif

#define _P4_SOURCE 1

#include <pfe/pfe-base.h>
#include <pfe/file-sub.h>

#include <errno.h>

/** INCLUDE ( "filename" -- ??? ) [FTH]
 * load the specified file, see also => LOAD" filename"
 */
FCode (p4_include)
{
    p4_char_t *fn = p4_word (' ');

    p4_included (P4_CHARBUF_PTR(fn), P4_CHARBUF_LEN(fn));
}

/************************************************************************/
/* more file manipulation                                               */
/************************************************************************/

/** COPY-FILE ( src-ptr src-len dst-ptr dst-len -- copy-errno# ) [FTH]
 * like =>'RENAME-FILE', copies the file from src-name to dst-name
 * and returns an error-code or null
 */
FCode (p4_copy_file)
{
    char* src = p4_pocket_filename ((p4_char_t *) SP[3], SP[2]);
    char* dst = p4_pocket_filename ((p4_char_t *) SP[1], SP[0]);
    SP += 3;
    *SP = p4_file_copy (src, dst, P4_OFF_T_MAX) ? PFE_io_errno : 0;
}

/** MOVE-FILE ( src-ptr src-len dst-ptr dst-len -- move-errno# ) [FTH]
 * like =>'RENAME-FILE', but also across-volumes <br>
 * moves the file from src-name to dst-name and returns an
 * error-code or null
 */
FCode (p4_move_file)		
{
    char* src = p4_pocket_filename ((p4_char_t *) SP[3], SP[2]);
    char* dst = p4_pocket_filename ((p4_char_t *) SP[1], SP[0]);
    SP += 3;
    *SP = p4_file_move (src, dst) ? PFE_io_errno : 0;
}

P4_LISTWORDS (file_misc) =
{
    P4_INTO ("FORTH", "[ANS]"),
    P4_FXco ("INCLUDE",		p4_include),

    /* more file-manipulation */
    P4_FXco ("COPY-FILE",	p4_copy_file),
    P4_FXco ("MOVE-FILE",	p4_move_file),
};
P4_COUNTWORDS (file_misc, "FILE-Misc Compatibility words");

/*@}*/
/* 
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */


