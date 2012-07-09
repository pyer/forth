#ifndef _VOL_8_SRC_CVS_PFE_33_PFE_CORE_MIX_H
#define _VOL_8_SRC_CVS_PFE_33_PFE_CORE_MIX_H 1209868836
/* generated 2008-0504-0440 /vol/8/src/cvs/pfe-33/pfe/../mk/Make-H.pl /vol/8/src/cvs/pfe-33/pfe/core-mix.c */

#include <pfe/pfe-mix.h>

/** 
 * -- miscellaneous useful extra words for CORE-EXT
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
 *      ... for CORE-EXT
 */

#ifdef __cplusplus
extern "C" {
#endif




/** 0<= ( a -- flag )
 simulate    : 0<= 0> 0= ;
 */
extern P4_CODE (p4_zero_less_equal);

/** 0>= ( a -- flag )
 simulate    : 0>= 0< 0= ;
 */
extern P4_CODE (p4_zero_greater_equal);

/** <= ( a b -- flag )
 simulate    : <= > 0= ;
 */
extern P4_CODE (p4_less_equal);

/** >= ( a b -- flag )
 simulate    : >= < 0= ;
 */
extern P4_CODE (p4_greater_equal);

/** U<= ( a b -- flag )
 simulate    : U<= U> 0= ;
 */
extern P4_CODE (p4_u_less_equal);

/** U>= ( a b -- flag )
 simulate    : U>= U< 0= ;
 */
extern P4_CODE (p4_u_greater_equal);

/** UMAX ( a b -- max )
 * see => MAX
 */
extern P4_CODE (p4_u_max);

/** UMIN ( a b -- min )
 * see => MIN , => MAX and => UMAX
 */
extern P4_CODE (p4_u_min);

/** LICENSE ( -- )
 * show a lisence info - the basic PFE system is licensed under the terms
 * of the LGPL (Lesser GNU Public License) - binary modules loaded into
 * the system and hooking into the system may carry another => LICENSE
 : LICENSE [ ENVIRONMENT ] FORTH-LICENSE TYPE ;
 */
extern P4_CODE (p4_license);

/** WARRANTY ( -- )
 * show a warranty info - the basic PFE system is licensed under the terms
 * of the LGPL (Lesser GNU Public License) - which exludes almost any 
 * liabilities whatsoever - however loadable binary modules may hook into
 * the system and their functionality may have different WARRANTY infos.
 */
extern P4_CODE (p4_warranty);

/** .VERSION ( -- )
 * show the version of the current PFE system
 : .VERSION [ ENVIRONMENT ] FORTH-NAME TYPE FORTH-VERSION TYPE ;
 */
extern P4_CODE (p4_dot_version);

/** .CVERSION ( -- )
 * show the compile date of the current PFE system
 : .CVERSION [ ENVIRONMENT ] FORTH-NAME TYPE FORTH-DATE TYPE ;
 */
extern P4_CODE (p4_dot_date);

/** STRING,               ( str len -- )
 *  Store a string in data space as a counted string.
 : STRING, HERE  OVER 1+  ALLOT  PLACE ;
 */
extern P4_CODE (p4_string_comma);

/** PARSE,                    ( "chars<">" -- )
 *  Store a char-delimited string in data space as a counted
 *  string. As seen in Bawd's
 : ," [CHAR] " PARSE  STRING, ; IMMEDIATE
 *
 * this implementation is much different from Bawd's
 : PARSE, PARSE STRING, ;
 */
extern P4_CODE (p4_parse_comma);

/** "PARSE,\""  ( "chars<">" -- )
 *  Store a quote-delimited string in data space as a counted
 *  string.
 : ," [CHAR] " PARSE  STRING, ; IMMEDIATE
 *
 * implemented here as
 : PARSE," [CHAR] " PARSE, ; IMMEDIATE
 */
extern P4_CODE (p4_parse_comma_quote);

/** "(MARKER)" ( str-ptr str-len -- )
 * create a named marker that you can use to => FORGET ,
 * running the created word will reset the dict/order variables
 * to the state at the creation of this name.
 : (MARKER) (CREATE) HERE , 
         GET-ORDER DUP , 0 DO ?DUP IF , THEN LOOP 0 , 
         ...
   DOES> DUP @ (FORGET) 
         ...
 ; 
 */
extern P4_CODE (p4_paren_marker);

/** ANEW ( 'name' -- )
 * creates a => MARKER if it doesn't exist,
 * or forgets everything after it if it does. (it just gets executed).
 *
 * Note: in PFE the => ANEW will always work on the => ENVIRONMENT-WORDLIST
 * which has a reason: it is never quite sure whether the same
 * => DEFINITIONS wordlist is in the search => ORDER that the original
 * => ANEW => MARKER was defined in. Therefore, => ANEW would be only safe
 * on systems that do always stick to => FORTH => DEFINITIONS. Instead
 * we will => CREATE the => ANEW => MARKER in the => ENVIRONMENT and use a 
 * simple => SEARCH-WORDLIST on the => ENVIRONMENT-WORDLIST upon re-run.
 \ old
 : ANEW BL WORD   DUP FIND NIP IF EXECUTE THEN   (MARKER) ;
 \ new
 : ANEW 
   PARSE-WORD  2DUP ENVIRONMENT-WORDLIST SEARCH-WORDLIST IF  EXECUTE  THEN 
   GET-CURRENT >R ENVIRONMENT-WORDLIST SET-CURRENT  (MARKER)  R> SET-CURRENT ;
 */
extern P4_CODE (p4_anew);

/** "((MARKER))" ( -- )
 * runtime compiled by => MARKER
 */
extern P4_CODE (p4_marker_RT);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
