/**
 *  CORE -- The standard CORE wordset
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.18 $
 *     (modified $Date: 2008-12-21 11:01:54 $)
 *
 *  @description
 *      The Core Wordset contains the most of the essential words
 *      for ANS Forth.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <setjmp.h>
#include <errno.h>

#include "config.h"
#include "types.h"
#include "const.h"
#include "macro.h"
#include "listwords.h"
#include "session.h"

#include "compiler.h"
#include "dictionary.h"
#include "exception.h"
#include "interpret.h"

#define	P4_FLAG(X)	((X) ? P4_TRUE : P4_FALSE)

/************************************************************************/
/* Core Words                                                           */
/************************************************************************/

/** ! ( value some-cell* -- | value addr* -- [?] ) [ANS]
 * store value at addr (sizeof =>"CELL")
 */
FCode (p4_store)
{
    *(p4cell *) SP[0] = SP[1];
    SP += 2;
}


/** "*" ( a# b# -- mul-a#' | a b -- mul-a' [??] ) [ANS]
 * return the multiply of the two args
 */
FCode (p4_star)
{
    SP[1] = SP[0] * SP[1];
    SP++;
}

/** + ( a* b# -- a*' | a# b* -- b*' | a# b# -- a#' | a b -- a' [??] ) [ANS]
 * return the sum of the two args
 */
FCode (p4_plus)
{
    SP[1] += SP[0];
    SP++;
}

/** +! ( value# some-cell* -- | value some* -- [?] ) [ANS]
 * add val to the value found in addr
 simulate:
   : +! TUCK @ + SWAP ! ;
 */
FCode (p4_plus_store)
{
    *(p4cell *) SP[0] += SP[1];
    SP += 2;
}

/** "-" ( a* b# -- a*' | a# b* -- b*' | a# b# -- a#' | a* b* -- diff-b#' | a b -- a' [??] ) [ANS]
 * return the difference of the two arguments
 */
FCode (p4_minus)
{
    SP[1] -= SP[0];
    SP++;
}

/** _/_
 * floored divide procedure, single prec
 */
fdiv_t p4_fdiv (p4cell num, p4cell denom)
{
    fdiv_t res;

    res.quot = num / denom;
    res.rem = num % denom;
    if (res.rem && (num ^ denom) < 0)
    {
        res.quot--;
        res.rem += denom;
    }
    return res;
}

/** "/" ( a# b#  -- a#' | a b -- a' [???] ) [ANS]
 * return the quotient of the two arguments
 */
FCode (p4_slash)
{
    fdiv_t res = p4_fdiv (SP[1], SP[0]);
    *++SP = res.quot;
}

/** "/MOD" ( a# b# -- div-a#' mod-a#' | a b -- div-a' mod-a' [??] ) [ANS]
 * divide a and b and return both
 * quotient n and remainder m
 */
FCode (p4_slash_mod)
{
    *(fdiv_t *) SP = p4_fdiv (SP[1], SP[0]);
}

/** 0< ( value -- test-flag ) [ANS]
 * return a flag that is true if val is lower than zero
 simulate:
  : 0< 0 < ;
 */
FCode (p4_zero_less)
{
    *SP = P4_FLAG (*SP < 0);
}

/** 0= ( 0 -- test-flag! | value! -- 0 | value -- test-flag ) [ANS]
 * return a flag that is true if val is just zero
 simulate:
  : 0= 0 = ;
 */
FCode (p4_zero_equal)
{
    *SP = P4_FLAG (*SP == 0);
}

/** "0<>" ( 0 -- 0 | value! -- value-flag! | value -- value-flag ) [ANS]
 * returns a logical-value saying if the value was not-zero.
 * This is most useful in turning a numerical value into a
 * boolean value that can be fed into bitwise words like
 * => AND and => XOR - a simple => IF or => WHILE doesn't
 * need it actually.
 */
FCode (p4_zero_not_equals)
{
    *SP = P4_FLAG (*SP != 0);
}

/** 0> ( 0 -- 0 | value! -- value-flag! | value -- value-flag ) [ANS]
 * return value greater than zero
 simulate:    : 0> 0 > ;
 */
FCode (p4_zero_greater)
{
    *SP = P4_FLAG (*SP > 0);
}

/** 1+ ( value -- value' ) [ANS]
 * return the value incremented by one
 simulate:
  : 1+ 1 + ;
 */
FCode (p4_one_plus)
{
    ++*SP;
}

/** 1- ( value -- value' ) [ANS]
 * return the value decremented by one
 simulate:
   : 1- 1 - ;
 */
FCode (p4_one_minus)
{
    --*SP;
}

/** 2+ ( value -- value' ) [ANS]
 * return the value incremented by one
 simulate:
  : 2+ 2 + ;
 */
FCode (p4_two_plus)
{
    *SP += 2;
}

/** 2- ( value -- value' ) [ANS]
 * return the value decremented by one
 simulate:
   : 2- 2 - ;
 */
FCode (p4_two_minus)
{
    *SP -= 2;
}

/** 2* ( a# -- a#' | a -- a' [??] ) [ANS]
 * multiplies the value with two - but it
 * does actually use a shift1 to be faster
 simulate:
  : 2* 2 * ; ( canonic) : 2* 1 LSHIFT ; ( usual)
 */
FCode (p4_two_star)
{
    *SP <<= 1;
}

/** 2/ ( a# -- a#' | a -- a' [??] ) [ANS]
 * divides the value by two - but it
 * does actually use a shift1 to be faster
 simulate:
  : 2/ 2 / ; ( canonic) : 2/ 1 RSHIFT ; ( usual)
 */
FCode (p4_two_slash)
{
    *SP >>= 1;
}

/** < ( a* b* -- test-flag | a# b# -- test-flag | a b -- test-flag [?] ) [ANS]
 * return a flag telling if a is lower than b
 */
FCode (p4_less_than)
{
    SP[1] = P4_FLAG (SP[1] < SP[0]);
    SP++;
}

/** = ( a* b* -- test-flag | a# b# -- test-flag | a b -- test-flag [?] ) [ANS]
 * return a flag telling if a is equal to b
 */
FCode (p4_equals)
{
    SP[1] = P4_FLAG (SP[1] == SP[0]);
    SP++;
}

/** "<>" ( a b -- a-flag ) [ANS]
 * return true if a and b are not equal, see => =
 */
FCode (p4_not_equals)
{
    SP[1] = P4_FLAG (SP[0] != SP[1]);
    SP++;
}

/** > ( a* b* -- test-flag | a# b# -- test-flag | a b -- test-flag [?] ) [ANS]
 * return a flag telling if a is greater than b
 */
FCode (p4_greater_than)
{
    SP[1] = P4_FLAG (SP[1] > SP[0]);
    SP++;
}

/** >R ( value -- R: value ) [ANS]
 * save the value onto the return stack. The return
 * stack must be returned back to clean state before
 * an exit and you should note that the return-stack
 * is also touched by the => DO ... => WHILE loop.
 * Use => R> to clean the stack and => R@ to get the
 * last value put by => >R
 */
FCode (p4_to_r)
{
    FX (pf_Q_comp);
    FX_COMPILE (p4_to_r);
}
FCode_XE (p4_to_r_execution)
{
//#define P4_PUSH(X,P)    (*P4_DEC (P, p4cell) = (X))
//#define P4_DEC(P,T)	(--(*(T **)&(P)))
//    P4_PUSH(*SP++, RP);
//    *P4_DEC (RP, p4cell) = *SP++;
    *(--(*(p4cell **)&(RP))) = *SP++;
}
P4COMPILES (p4_to_r, p4_to_r_execution,
	    P4_SKIPS_NOTHING, P4_DEFAULT_STYLE);


/** ?DUP ( 0 -- 0 | value! -- value! value! | value -- 0 | value! value! ) [ANS]
 * one of the rare words whose stack-change is
 * condition-dependet. This word will duplicate
 * the value only if it is not zero. The usual
 * place to use it is directly before a control-word
 * that can go to different places where we can
 * spare an extra => DROP on the is-null-part.
 * This makes the code faster and often a little
 * easier to read.
 example:
   : XX BEGIN ?DUP WHILE DUP . 2/ REPEAT ; instead of
   : XX BEGIN DUP WHILE DUP . 2/ REPEAT DROP ;
 */
FCode (p4_Q_dup)
{
    if (*SP)
        --SP, SP[0] = SP[1];
}

/** @ ( value* -- value ) [ANS]
 * fetch the value from the variables address
 */
FCode (p4_fetch)
{
    *SP = *(p4cell *) *SP;
}

/** ABS ( value# -- value#' ) [ANS]
 * return the absolute value
 */
FCode (p4_abs)
{
    if (*SP < 0)
        *SP = -*SP;
}

/** AND ( value mask -- value' ) [ANS]
 * mask with a bitwise and - be careful when applying
 * it to logical values.
 */
FCode (p4_and)
{
    SP[1] &= SP[0];
    SP++;
}

/** C! ( value# variable#* -- | value# variable* [?] ) [ANS]
 * store the byte-value at address, see => !
 */
FCode (p4_c_store)
{
    *(char *) SP[0] = SP[1];
    SP += 2;
}

/** C@ ( value#* -- value# | value* -- value# [?] ) [ANS]
 * fetch a byte-value from the address, see => @
 */
FCode (p4_c_fetch)
{
    *SP = *(p4char *) *SP;
}

/** COUNT ( string-bstr* -- string-ptr' string-len | some* -- some*' some-len [?] ) [ANS]
 * usually before calling => TYPE
 *
 * (as an unwarranted extension, this word does try to be idempotent).
 */
FCode (p4_count)
{
    /* can not unpack twice - this trick prevents from many common errors */
    if (256 > (p4ucell)(SP[0])) goto possibly_idempotent;
    --SP;
//#define P4_INCC(P,T)	((*(T **)&(P))++)
//    SP[0] = *P4_INCC(SP[1], p4char); /* SP[0] = *((p4char*) SP[1] )++; */
    SP[0] = *((*(p4char **)&(SP[1]))++);
    return;

    /* an idempotent COUNT allows to ease the transition from counted-strings
     * to string-spans:
     c" hello world" count type ( is identical with...)
     s" hello world" count type
     * however: it makes some NULL argument or just illegal argument to be
     * silently accepted that can make debugging programs a pain. Therefore
     * this function has been given some intelligence, with the counter effect
     * of being somewhat undetermined which part gets triggered at runtime.
     */
 possibly_idempotent:
    if (((p4char**)SP)[1][-1] == (p4char)(SP[0])) /* idempotent ? */
    { if ((p4char)(SP[0])) return; } /* only if not null-count ! */
    *--PFE.sp = (p4cell)(0); /* makes later functions to copy nothing at all */
}

/** DEPTH ( -- depth# ) [ANS]
 * return the depth of the parameter stack before
 * the call, see => SP@ - the return-value is in => CELLS
 */
FCode (p4_depth)
{
    register size_t n;

    n = p4_S0 - SP;
    *--SP = n;
}

/** DROP ( a -- ) [ANS]
 * just drop the word on the top of stack, see => DUP
 */
FCode (p4_drop)
{
    SP++;
}

/** DUP ( a -- a a ) [ANS]
 * duplicate the cell on top of the stack - so the
 * two topmost cells have the same value (they are
 * equal w.r.t => = ) , see => DROP for the inverse
 */
FCode (p4_dup)
{
    SP--;
    SP[0] = SP[1];
}

/** EXECUTE ( some-xt* -- ??? ) [ANS]
 * run the execution-token on stack - this will usually
 * trap if it was null for some reason, see => >EXECUTE
 simulate:
  : EXECUTE >R EXIT ;
 */
FCode (p4_execute)
{
    PFE.execute ((p4xt) *SP++);
}

/** FILL ( mem-ptr mem-len char# -- ) [ANS]
 * fill a memory area with the given char, does now
 * simply call memset()
 */
FCode (p4_fill)
{
    memset ((void *) SP[2], SP[0], SP[1]);
    SP += 3;
}

/** INVERT ( value# -- value#' ) [ANS]
 * make a bitwise negation of the value on stack.
 * see also => NEGATE
 */
FCode (p4_invert)
{
    *SP = ~*SP;
}

/** LSHIFT ( value# shift-count -- value#' ) [ANS]
 * does a bitwise left-shift on value
 */
FCode (p4_l_shift)
{
    SP[1] <<= SP[0];
    SP++;
}

/** MAX ( a# b# -- a#|b# | a* b* -- a*|b* | a b -- a|b [??] ) [ANS]
 * return the maximum of a and b
 */
FCode (p4_max)
{
    if (SP[0] > SP[1])
        SP[1] = SP[0];
    SP++;
}

/** MIN ( a# b# -- a#|b# | a* b* -- a*|b* | a b -- a|b [??] ) [ANS]
 * return the minimum of a and b
 */
FCode (p4_min)
{
    if (SP[0] < SP[1])
        SP[1] = SP[0];
    SP++;
}

/** MOD ( a# b# -- mod-a# | a b# -- mod-a# [??] ) [ANS]
 * return the module of "a mod b"
 */
FCode (p4_mod)
{
    fdiv_t res = p4_fdiv (SP[1], SP[0]);

    *++SP = res.rem;
}

/** NEGATE ( value# -- value#' ) [ANS]
 * return the arithmetic negative of the (signed) cell
 simulate:   : NEGATE -1 * ;
 */
FCode (p4_negate)
{
    *SP = -*SP;
}

/** OR ( a b# -- a' | a# b -- b' | a b -- a' [??] ) [ANS]
 * return the bitwise OR of a and b - unlike => AND this
 * is usually safe to use on logical values
 */
FCode (p4_or)
{
    SP[1] |= SP[0];
    SP++;
}

/** OVER ( a b -- a b a ) [ANS]
 * get the value from under the top of stack. The inverse
 * operation would be => TUCK
 */
FCode (p4_over)
{
    --SP;
    SP[0] = SP[2];
}

/** PICK ( value ...[n-1] n -- value ...[n-1] value ) [ANS]
 * pick the nth value from under the top of stack and push it
 * note that
   0 PICK -> DUP         1 PICK -> OVER
 */
FCode (p4_pick)
{
    *SP = SP[*SP + 1];
}

/** R> ( R: a -- a R: ) [ANS]
 * get back a value from the return-stack that had been saved
 * there using => >R . This is the traditional form of a local
 * var space that could be accessed with => R@ later. If you
 * need more local variables you should have a look at => LOCALS|
 * which does grab some space from the return-stack too, but names
 * them the way you like.
 */
FCode (p4_r_from)
{
    FX (pf_Q_comp);
    FX_COMPILE (p4_r_from);
}

FCode_XE (p4_r_from_execution)
{
    *--SP = (p4cell) *RP++;
}
P4COMPILES (p4_r_from, p4_r_from_execution,
	    P4_SKIPS_NOTHING, P4_DEFAULT_STYLE);

/** R@ ( R: a -- a R: a ) [ANS]
 * fetch the (upper-most) value from the return-stack that had
 * been saved there using =>">R" - This is the traditional form of a local
 * var space. If you need more local variables you should have a
 * look at => LOCALS| , see also =>">R" and =>"R>" . Without LOCALS-EXT
 * there are useful words like =>"2R@" =>"R'@" =>'R"@' =>'R!'
 */
FCode (p4_r_fetch)
{
    FX (pf_Q_comp);
    FX_COMPILE (p4_r_fetch);
}
FCode_XE (p4_r_fetch_execution)
{
    *--SP = *((p4cell*)RP);
}
P4COMPILES (p4_r_fetch, p4_r_fetch_execution,
	    P4_SKIPS_NOTHING, P4_DEFAULT_STYLE);

/** ROLL ( value ...[n-1] n -- ...[n-1] value ) [ANS]
 * the extended form of => ROT
    2 ROLL -> ROT
 */
FCode (p4_roll)
{
    p4cell i = *SP++;
    p4cell h = SP[i];

    for (; i > 0; i--)
        SP[i] = SP[i - 1];
    SP[0] = h;
}

/** ROT ( a b c -- b c a ) [ANS]
 * rotates the three uppermost values on the stack,
 * the other direction would be with => -ROT - please
 * have a look at => LOCALS| and => VAR that can avoid
 * its use.
 */
FCode (p4_rot)
{
    p4cell h = SP[2];

    SP[2] = SP[1];
    SP[1] = SP[0];
    SP[0] = h;
}

/** RSHIFT ( value# shift-count# -- value#' ) [ANS]
 * does a bitwise logical right-shift on value
 * (ie. the value is considered to be unsigned)
 */
FCode (p4_r_shift)
{
    *(p4ucell *) &SP[1] >>= SP[0];
    SP++;
}

/** SWAP ( a b -- b a ) [ANS]
 * exchanges the value on top of the stack with the value beneath it
 */
FCode (p4_swap)
{
    p4cell h = SP[1];

    SP[1] = SP[0];
    SP[0] = h;
}

/** XOR ( a# b# -- a#' ) [ANS]
 * return the bitwise-or of the two arguments - it may be unsafe
 * use it on logical values. beware.
 */
FCode (p4_xor)
{
    SP[1] ^= SP[0];
    SP++;
}

/************************************************************************/
/* Core Extension Words                                                 */
/************************************************************************/

/** ERASE ( buffer-ptr buffer-len -- ) [ANS]
 * fill an area will zeros.
 2000 CREATE DUP ALLOT ERASE
 */
FCode (p4_erase)
{
    memset ((void *) SP[1], 0, SP[0]);
    SP += 2;
}

/** FALSE ( -- 0 ) [ANS]
 * places zero on the stack to express the code want to do a boolean
 * evaluation with this value. (=>"IF")
 */

/** TRUE ( -- true! ) [ANS]
 * places => FALSE => INVERT on the stack to express the code want to
 * do a boolean evaluation with this value. (=>"IF") - we chose the
 * FIG79/FTH83 value of allbitsset to make it easier to do logical
 * computations using bitwise operands like => AND => OR => NOT ,
 * however ANS-Forth programs shall not assume a specific representation
 * of the truth value and handle just the cases =>"=0" and =>"<>0"
 */

/** NIP ( a b -- b ) [ANS]
 * drop the value under the top of stack, inverse of => TUCK
 simulate:        : NIP SWAP DROP ;
 */
FCode (p4_nip)
{
    SP[1] = SP[0];
    SP++;
}

/** TUCK ( a b -- b a b ) [ANS]
 * shove the top-value under the value beneath. See => OVER
 * and => NIP
 simulate:    : TUCK  SWAP OVER ;
 */
FCode (p4_tuck)
{
    --SP;
    SP[0] = SP[1];
    SP[1] = SP[2];
    SP[2] = SP[0];
}

/** WITHIN ( a# b# c# -- a-flag | a* b* c* -- a-flag ) [ANS]
 * a widely used word, returns ( b <= a && a < c ) so
 * that is very useful to check an index a of an array
 * to be within range b to c
 */
FCode (p4_within)
{
    SP[2] = P4_FLAG
        ( (p4ucell) (SP[2] - SP[1]) <
          (p4ucell) (SP[0] - SP[1]) );
    SP += 2;
}

/* -------------------------------------------------------------- */
/*
 * a few notes: there are some synonyms in the table below that might
 * not look natural - these are synyms where the behaviour of a word
 * has changed from fig-forth to ans-forth. In these case I prefer to
 * "invent" a new word that is not ambiguous in its name between the
 * system. The names in the ans-forth sources do not need to change
 * but people should expect to see a different name being decompiled.
 * This is merely intended since I have to maintain quite a few programs
 * that are ported from fig-forth to ans-forth and where some parts are
 * in ans-forth speak while others are in fig-forth speak. Decompiling
 * the result will show to me whether there was an error during load
 * time of the program, e.g. the "tick" used will turn out to be either
 * a "cfa'" (the ans-forth result) or "pfa'" (the fig-forth result).
 * Likewise I let "create" decompile as either "<builds" or "create:"
 * because the latter is not supposed to be extended with "does>".
 */
/*
Nucleus layer 
!  *  +  +!  -  /  /MOD  0<  0=  0>  1+  1-  2+  
2-  2! 2* 2/  <  =  >  >R  ?DUP  @  ABS  AND  C!  C@
COUNT  DEPTH  DROP  DUP
EXIT  FILL  I  J  MAX  MIN  MOD  NEGATE  NOT  OR  OVER  PICK  
R>  R@  ROLL  ROT  SWAP  U<  UM*  UM/MOD  XOR 

not implemented
CMOVE CMOVE>

not here
EXECUTE  
EXIT
*/

P4_LISTWORDS (core) =
{
//    P4_INTO ("FORTH", 0),
    /** quick constants - implemented as code */
    P4_OCoN ("0",		0),
    P4_OCoN ("1",		1),
    P4_OCoN ("2",		2),
    P4_OCoN ("3",		3),
    /* core words */
    P4_FXco ("!",            p4_store),
    P4_FXco ("*",            p4_star),
    P4_FXco ("+",            p4_plus),
    P4_FXco ("+!",           p4_plus_store),
    P4_FXco ("-",            p4_minus),
    P4_FXco ("/",            p4_slash),
    P4_FXco ("/MOD",         p4_slash_mod),
    P4_FXco ("0<",           p4_zero_less),
    P4_FXco ("0=",           p4_zero_equal),
    P4_FXco ("0<>",          p4_zero_not_equals),
    P4_FXco ("0>",           p4_zero_greater),
    P4_FXco ("1+",           p4_one_plus),
    P4_FXco ("1-",           p4_one_minus),
    P4_FXco ("2+",           p4_two_plus),
    P4_FXco ("2-",           p4_two_minus),
    P4_FXco ("2*",           p4_two_star),
    P4_FXco ("2/",           p4_two_slash),
    P4_FXco ("<",            p4_less_than),
    P4_FXco ("=",            p4_equals),
    P4_FXco ("<>",           p4_not_equals),
    P4_FXco (">",            p4_greater_than),
    P4_SXco (">R",           p4_to_r),
    P4_FXco ("?DUP",         p4_Q_dup),
    P4_FXco ("@",            p4_fetch),
    P4_FXco ("ABS",          p4_abs),
    P4_FXco ("AND",          p4_and),
    P4_FXco ("C!",           p4_c_store),
    P4_FXco ("C@",           p4_c_fetch),
    P4_FXco ("COUNT",        p4_count),
    P4_FXco ("DEPTH",        p4_depth),
    P4_FXco ("DROP",         p4_drop),
    P4_FXco ("DUP",          p4_dup),
    P4_FXco ("EXECUTE",      p4_execute),
    P4_FXco ("FILL",         p4_fill),
    P4_FXco ("INVERT",       p4_invert),
    P4_FXco ("MAX",          p4_max),
    P4_FXco ("MIN",          p4_min),
    P4_FXco ("MOD",          p4_mod),
    P4_FXco ("NEGATE",       p4_negate),
    P4_FXco ("NOT",          p4_invert), // "INVERT" synonym
    P4_FXco ("OR",           p4_or),
    P4_FXco ("OVER",         p4_over),
    P4_FXco ("PICK",         p4_pick),
    P4_SXco ("R>",           p4_r_from),
    P4_SXco ("R@",           p4_r_fetch),
    P4_FXco ("ROLL",         p4_roll),
    P4_FXco ("ROT",          p4_rot),
    P4_FXco ("SWAP",         p4_swap),
    P4_FXco ("XOR",          p4_xor),

    /* core extension words */
    P4_FXco ("ERASE",        p4_erase),
    P4_OCoN ("FALSE",        P4_FALSE),
    P4_OCoN ("TRUE",         P4_TRUE),
    P4_FXco ("NIP",          p4_nip),
    P4_FXco ("TUCK",         p4_tuck),
    P4_FXco ("WITHIN",       p4_within),
};
P4_COUNTWORDS (core, "Core words");

