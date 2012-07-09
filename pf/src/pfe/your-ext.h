#ifndef _VOL_8_SRC_CVS_PFE_33_PFE_YOUR_EXT_H
#define _VOL_8_SRC_CVS_PFE_33_PFE_YOUR_EXT_H 1209868837
/* generated 2008-0504-0440 /vol/8/src/cvs/pfe-33/pfe/../mk/Make-H.pl /vol/8/src/cvs/pfe-33/pfe/your-ext.c */

#include <pfe/pfe-ext.h>

/** 
 * -- user-supplied additional primitives
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) Guido U. Draheim, 2005 - 2008
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.5 $
 *     (modified $Date: 2008-05-04 02:57:30 $)
 *
 *  @description
 *              This wordset is the place to add any additional primitives
 *		you might wish. A set of words do already live here that
 *              must be bound statically into the main pfe-object to
 *              work out smart and nicely.
 */

#ifdef __cplusplus
extern "C" {
#endif




extern P4_CODE (p4_name);

/** "'>" ( [name] -- xt )
 * get the execution-token, ie the CFA, of the word following.
 * This word is fully state-smart while the ANSI standard words
 * namely => ' and => ['] are not.
 */
extern P4_CODE (p4_tick_from);

/** @> ( [name] -- value )
 * does fetch the value from the PFA of the named item, which
 * may be about everything, including a => VARIABLE , => VALUE
 * => LVALUE , => LOCALS| , => VAR , => DEFER , => DOER , => DOES>
 * and more.
 */
extern P4_CODE (p4_fetch_from);

/** ((INTO))
 * execution compiled by => INTO
 */
extern P4_CODE (p4_into_execution);

/** ((INTO-)) ( -- value ) 
 * execution compiled by => INTO
 */
extern P4_CODE (p4_into_local_execution);

/** INTO ( [name] -- pfa )
 * will return the parameter-field address of the following word.
 * Unlike others, this word will also return the address of
 * => LOCALS| and local => LVALUE - so in fact a <c>TO A</c> and 
 * <c>INTO A !</c> are the same. This word is most useful when calling
 * C-exported function with a temporary local-VAR as a return-place
 * argument - so the address of a local has to be given as an arg.
 * Beware that you should not try to save the address anywhere else,
 * since a local's address does always depend of the RP-depth -
 * EXIT from a colon-word and the value may soon get overwritten.
 * (see also => TO )
 */
extern P4_CODE (p4_into);

/** .H2 ( value -- )
 * print hexadecimal, but with per-byte 0-padding
   0x0     -> 00
   0xf     -> 0f
   0x12    -> 12
   0x123   -> 0123
   0x1234  -> 1234
   0x12345 -> 012345
 */
extern P4_CODE (p4_dot_h2);

/** HERE-WORD ( char "name<char>" -- )
 * a FIG-compatible WORD. Where ANSI says "skip leading delimiters"
 * this one acts as "skip leading whitespace". And it will not return
 * anything and have the string parsed to => HERE
 */
extern P4_CODE (p4_here_word);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
