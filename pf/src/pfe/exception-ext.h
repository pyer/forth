#ifndef _PFE_EXCEPTION_EXT_H
#define _PFE_EXCEPTION_EXT_H 1209930552
/* generated 2008-0504-2149 ../../pfe/../mk/Make-H.pl ../../pfe/exception-ext.c */

#include <pfe/pfe-ext.h>

/** 
 * --    The Optional Exception Word Set
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.7 $
 *     (modified $Date: 2008-05-04 19:53:05 $)
 *
 *  @description
 *      These words implement an exception system in the
 *      widely known => THROW &amp; => CATCH concept.
 *
 *      see the PFE-SIG wordset for catching OS traps.
 */

#ifdef __cplusplus
extern "C" {
#endif




/** CATCH ( catch-xt* -- 0 | throw#! ) [ANS]
 * execute the given execution-token and catch
 * any exception that can be caught therein.
 * software can arbitrarily raise an exception
 * using => THROW - the value 0 means there
 * was no exception, other denote implementation
 * dependent exception-codes.
 */
extern P4_CODE (p4_catch);

/** THROW ( throw#! -- [THROW] | throw# -- ) [ANS]
 * raise an exception - it will adjust the depth
 * of all stacks and start interpreting at the point 
 * of the latest => CATCH <br>
 * if n is null nothing happens, the -1 (ie. => FALSE )
 * is the raise-code of => ABORT - the other codes
 * are implementation dependent and will result in
 * something quite like => ABORT
 */
extern P4_CODE (p4_throw);

/** ABORT ( -- [THROW] ) [ANS]
 * throw - cleanup some things and go back to the QUIT routine
 : ABORT -1 THROW ;
 */
extern P4_CODE (p4_abort);

/** ((ABORT")) ( -- ) [HIDDEN]
 * compiled by => ABORT" what"
 */
extern P4_CODE (p4_abort_quote_execution);

/** 'ABORT"' ( [string<">] -- [THROW] ) [ANS]
 * throw like => ABORT but print an additional error-message
 * to stdout telling what has happened.
 */
extern P4_CODE (p4_abort_quote);

extern P4_CODE (p4_exception_string_RT);

/** (EXCEPTION-STRING: ( exception# [description<closeparen>] -- )
 * append a node with the given id and a pointer to an 
 * extern zstring to the => NEXT-EXCEPTION chain-list.
 */
extern P4_CODE (p4_exception_string);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
