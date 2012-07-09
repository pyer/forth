#ifndef _VOL_8_SRC_CVS_PFE_33_PFE_FORTH_USUAL_EXT_H
#define _VOL_8_SRC_CVS_PFE_33_PFE_FORTH_USUAL_EXT_H 1209868837
/* generated 2008-0504-0440 /vol/8/src/cvs/pfe-33/pfe/../mk/Make-H.pl /vol/8/src/cvs/pfe-33/pfe/forth-usual-ext.c */

#include <pfe/pfe-ext.h>

/** 
 * -- usually implemented words.
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
 *              There are lots of useful words that do not appear
 *              in any standard. This wordset defines some of them.
 */

#ifdef __cplusplus
extern "C" {
#endif




/** C+!  ( n addr -- )
 *  Add the low-order byte of _n_ to the byte at _addr_,
 *  removing both from the stack.
 */
extern P4_CODE (p4_c_plus_store);

/** ">WORDLIST" ( xt -- wordl* )
 * convert a =>"VOCABULARY"-xt into its wordlist reference
 * (as in win32forth)
 */
extern P4_CODE (p4_to_wordlist);

/** BOUNDS                ( str len -- str+len str )
 *  Convert _str len_ to range for DO-loop.
 : BOUNDS  ( str len -- str+len str )  OVER + SWAP ;
 */
extern P4_CODE (p4_bounds);

/** OFF                   ( addr  -- )
 *  Store 0 at _addr_. Defined in f84 as =>"OFF". See antonym =>"ON!".
  : OFF  ( addr -- )  0 SWAP ! ;
 */
extern P4_CODE (p4_off_store);

/** ON!                    ( addr -- )
 *  Store -1 at _addr_. Defined in f83 as =>"ON". See antonym =>"OFF!".
  : ON!  ( addr -- )  -1 SWAP ! ;
 */
extern P4_CODE (p4_on_store);

/** +PLACE                ( str len add2 -- )
 *  Append string _str len_ to the counted string at _addr_.
 *  a.k.a. =>"APPEND" (being a => SYNONYM now)
 : +PLACE   2DUP 2>R  COUNT +  SWAP MOVE ( ) 2R> C+! ;
 */
extern P4_CODE (p4_append);

/** C+PLACE           ( char addr -- )
 *  Append _char_ to the counted string at _addr_.
 *  a.k.a. =>"APPEND-CHAR" (being a => SYNONYM now)
 : C+PLACE   DUP >R  COUNT  DUP 1+ R> C!  +  C! ;
 */
extern P4_CODE (p4_append_char);

/** PLACE                 ( str len addr -- )
 *  Place the string _str len_ at _addr_, formatting it as a
 *  counted string.
 : PLACE  2DUP 2>R  1+ SWAP  MOVE  2R> C! ;
 : PLACE  2DUP C!   1+ SWAP CMOVE ;
 */
extern P4_CODE (p4_place);

/** ?LEAVE ( cond -- )
 * leave a (innermost) loop if condition is true
 */
extern P4_CODE (p4_question_leave);

extern P4_CODE (p4_question_leave_execution);

/** RP@ ( -- addr ) 
 * returns the return stack pointer 
 example:
   : R@ RP@ @ ;
 */
extern P4_CODE (p4_r_p_fetch);

extern P4_CODE (p4_r_p_fetch_execution);

/** RP! ( addr -- ) 
 * sets the return stack pointer, reverse of => RP@
 */
extern P4_CODE (p4_r_p_store);

/** SP! ( ... addr -- ) 
 * sets the stack pointer, reverse of => SP@
 */
extern P4_CODE (p4_s_p_store);

/** -ROT ( a b c -- c a b )
 * inverse of => ROT
 */
extern P4_CODE (p4_dash_rot);

/** CSET ( n addr -- ) 
 * set bits in byte at given address 
 simulate:
   : CSET  TUCK @ SWAP OR SWAP ! ;
 */
extern P4_CODE (p4_c_set);

/** CRESET ( n addr -- ) 
 *  reset bits in byte at given address 
 simulate:
   : CRESET  TUCK @ SWAP NOT AND SWAP ! ;
 */
extern P4_CODE (p4_c_reset);

/** CTOGGLE ( n addr -- ) 
 * toggle bits in byte at given address 
 simulate:
   : CTOGGLE  TUCK @ SWAP XOR SWAP ! ;
 */
extern P4_CODE (p4_c_toggle);

/** TOGGLE ( c-addr charmask -- ) 
 * toggle the bits given in charmask, see also => SMUDGE and = UNSMUDGE
 example: the fig-style SMUDGE had been defined such
   : FIG-SMUDGE LATEST >FFA (SMUDGE#) TOGGLE ;
 */
extern P4_CODE (p4_toggle);

/** 3DUP                ( x y z -- x y z x y z )
 *  Copy top three elements on the stack onto top of stack.
 : 3DUP   THIRD THIRD THIRD ;
 *
 * or
 : 3DUP  3 PICK 3 PICK 3 PICK ;
 */
extern P4_CODE (p4_three_dup);

/** 3DROP               ( x y z -- )
 *  Drop the top three elements from the stack.
 : 3DROP   DROP 2DROP ;
 */
extern P4_CODE (p4_three_drop);

/** 4DUP ( a b c d -- a b c d a b c d )
 simulate:
  : 4DUP  4 PICK 4 PICK 4 PICK 4 PICK ;
 */
extern P4_CODE (p4_four_dup);

/** 4DROP               ( x y z -- )
 *  Drop the top three elements from the stack.
 : 4DROP   2DROP 2DROP ;
 */
extern P4_CODE (p4_four_drop);

/** TOUPPER ( c1 -- c2 ) 
 * convert a single character to upper case 
   : TOUPPER  >R _toupper ;
 */
extern P4_CODE (p4_toupper);

/** UPPER ( addr cnt -- ) 
 * convert string to upper case 
 simulate:
   : UPPER  0 DO  DUP I +  DUP C@ UPC SWAP C!  LOOP  DROP ;
 */
extern P4_CODE (p4_upper);

/** LOWER ( addr cnt -- ) 
 * convert string to lower case
 * This is not in L&P's F83 but provided for symmetry 
 simulate:
   : LOWER  0 DO  DUP I +  DUP C@ >R _tolower SWAP C!  LOOP  DROP ;
 */
extern P4_CODE (p4_lower);

/** ASCII ( [word] -- val )
 * state smart version of => CHAR or => [CHAR] resp.
 simulate:
   : ASCII  [COMPILE] [CHAR] 
            STATE @ IF [COMPILE] LITERAL THEN ;
 */
extern P4_CODE (p4_ascii);

/** CONTROL ( [word] -- val )
 * see =>'ASCII', but returns char - '@' 
 simulate:
   : CONTROL  [COMPILE] [CHAR]  [CHAR] @ -  
              STATE @ IF [COMPILE] LITERAL THEN ;
 */
extern P4_CODE (p4_control);

/** NUMBER? ( addr -- d flag ) 
 * convert counted string to number - used in inner interpreter 
 * ( => INTERPRET ), flags if conversion was successful
 example:
   BL WORD  HERE NUMBER? 0= IF ." not a number " THEN . 
 */
extern P4_CODE (p4_number_question);

/** VOCS ( -- )
 * list all vocabularies in the system
 simulate:
   : VOCS VOC-LINK @ BEGIN DUP WHILE
                           DUP ->WORDLIST.NAME @ ID.
                           ->WORDLIST.LINK @
                     REPEAT DROP ; 
 */
extern P4_CODE (p4_vocs);

/** @EXECUTE ( xt -- ? )
 * same as => @ => EXECUTE , but checks for null as xt and
 * silently ignores it. Same as in most forths where defined.
 simulate:
   : @EXECUTE  @ ?DUP IF EXECUTE THEN ;
 */
extern P4_CODE (p4_fetch_execute);

/** EMITS           ( n char -- )
 *  Emit _char_ _n_ times.
 : EMITS             ( n char -- )
    SWAP 0 ?DO  DUP EMIT  LOOP DROP ;
 * also compare
 : SPACES BL EMITS ;
 : SPACE BL EMIT ;
 */
extern P4_CODE(p4_emits);

/** FILE-CHECK        ( n -- )
 *  Check for file access error.
 \ : FILE-CHECK    ( n -- )  THROW ;
 : FILE-CHECK      ( n -- )  ABORT" File Access Error " ;
 */
extern P4_CODE (p4_file_check);

/** MEMORY-CHECK      ( n -- )
 *  Check for memory allocation error.
 \ : MEMORY-CHECK  ( n -- )  THROW ;
 : MEMORY-CHECK    ( n -- )  ABORT" Memory Allocation Error " ;
 */
extern P4_CODE (p4_memory_check);

/** ++                  ( addr -- )
 *  Increment the value at _addr_.
 : ++  ( addr -- )  1 SWAP +! ;
 */
extern P4_CODE (p4_plus_plus);

/** @++                  ( addr -- addr' x )
 *  Fetch the value _x_ from _addr_, and increment the address
 *  by one cell.
 : @++  ( addr -- addr' x )  DUP CELL+ SWAP  @ ;
 */
extern P4_CODE (p4_fetch_plus_plus);

/** !++                  ( addr x -- addr' )
 *  Store the value _x_ into _addr_, and increment the address
 *  by one cell.
 : !++  ( addr x -- addr' )  OVER !  CELL+ ;
 */
extern P4_CODE (p4_store_plus_plus);

_extern  p4_Wordl* p4_to_wordlist (p4xt xt) ; /*{*/

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
