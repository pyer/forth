#ifndef _VOL_8_SRC_CVS_PFE_33_PFE_CORE_EXT_H
#define _VOL_8_SRC_CVS_PFE_33_PFE_CORE_EXT_H 1209868836
/* generated 2008-0504-0440 /vol/8/src/cvs/pfe-33/pfe/../mk/Make-H.pl /vol/8/src/cvs/pfe-33/pfe/core-ext.c */

#include <pfe/pfe-ext.h>

/**
 *  CORE-EXT -- The standard CORE and CORE-EXT wordset
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.6 $
 *     (modified $Date: 2008-05-04 02:57:30 $)
 *
 *  @description
 *      The Core Wordset contains the most of the essential words
 *      for ANS Forth.
 */

#ifdef __cplusplus
extern "C" {
#endif




/** ! ( value some-cell* -- | value addr* -- [?] ) [ANS]
 * store value at addr (sizeof =>"CELL")
 */
extern P4_CODE (p4_store);

/** # ( n,n -- n,n' ) [ANS]
 * see also => HOLD for old-style forth-formatting words
 * and => PRINTF of the C-style formatting - this word
 * divides the argument by => BASE and add it to the
 * picture space - it should be used inside of => <#
 * and => #>
 */
extern P4_CODE (p4_sh);

/** #> ( n,n -- hold-str-ptr hold-str-len ) [ANS]
 * see also => HOLD for old-style forth-formatting words
 * and => PRINTF of the C-style formatting - this word
 * drops the argument and returns the picture space
 * buffer
 */
extern P4_CODE (p4_sh_greater);

/** #S ( n,n -- 0,0 ) [ANS]
 * see also => HOLD for old-style forth-formatting words
 * and => PRINTF of the C-style formatting - this word
 * does repeat the word => # for a number of times, until
 * the argument becomes zero. Hence the result is always
 * null - it should be used inside of => <# and => #>
 */
extern P4_CODE (p4_sh_s);

/** "'" ( 'name' -- name-xt* ) [ANS]
 * return the execution token of the following name. This word
 * is _not_ immediate and may not do what you expect in
 * compile-mode. See => ['] and => '> - note that in FIG-forth
 * the word of the same name had returned the PFA (not the CFA)
 * and was immediate/smart, so beware when porting forth-code
 * from FIG-forth to ANSI-forth.
 */
extern P4_CODE (p4_tick);

/** "("  ( 'comment<closeparen>' -- ) [ANS]
 * eat everything up to the next closing paren - treat it
 * as a comment.
 */
extern P4_CODE (p4_paren);

/** "*" ( a# b# -- mul-a#' | a b -- mul-a' [??] ) [ANS]
 * return the multiply of the two args
 */
extern P4_CODE (p4_star);

/** "*\/" ( a# b# c# -- scale-a#' | a b c -- scale-a' [??] ) [ANS]
 * regard the b/c as element Q - this word
 * has an advantage over the sequence of => *
 * and => / by using an intermediate double-cell
 * value
 */
extern P4_CODE (p4_star_slash);

/** "*\/MOD" ( a# b# c# -- div-a# mod-a# | a b c -- div-a mod-a [??] ) [ANS]
 * has an adavantage over the sequence of => *
 * and => /MOD by using an intermediate double-cell
 * value.
 */
extern P4_CODE (p4_star_slash_mod);

/** + ( a* b# -- a*' | a# b* -- b*' | a# b# -- a#' | a b -- a' [??] ) [ANS]
 * return the sum of the two args
 */
extern P4_CODE (p4_plus);

/** +! ( value# some-cell* -- | value some* -- [?] ) [ANS]
 * add val to the value found in addr
 simulate:
   : +! TUCK @ + SWAP ! ;
 */
extern P4_CODE (p4_plus_store);

/** "((+LOOP))" ( increment# -- ) [HIDDEN]
 * compiled by => +LOOP
 */
extern P4_CODE (p4_plus_loop_execution);

/** +LOOP ( increment# R: some,loop -- ) [ANS]
 * compile => ((+LOOP)) which will use the increment
 * as the loop-offset instead of just 1. See the
 * => DO and => LOOP construct.
 */
extern P4_CODE (p4_plus_loop);

/** "," ( value* -- | value# -- | value -- [?] ) [ANS]
 * store the value in the dictionary
 simulate:
   : , DP  1 CELLS DP +!  ! ;
 */
extern P4_CODE (p4_comma);

/** "-" ( a* b# -- a*' | a# b* -- b*' | a# b# -- a#' | a* b* -- diff-b#' | a b -- a' [??] ) [ANS]
 * return the difference of the two arguments
 */
extern P4_CODE (p4_minus);

/** "." ( value# -- | value* -- [?] | value -- [??] ) [ANS]
 * print the numerical value to stdout - uses => BASE
 */
extern P4_CODE (p4_dot);

/** '((.\"))' ( -- ) [HIDDEN] skipstring
 * compiled by => ." string"
 */
extern P4_CODE (p4_dot_quote_execution);

/** '.\"' ( [string<">] -- ) [ANS]
 * print the string to stdout
 */
extern P4_CODE (p4_dot_quote);

/** "/" ( a# b#  -- a#' | a b -- a' [???] ) [ANS]
 * return the quotient of the two arguments
 */
extern P4_CODE (p4_slash);

/** "/MOD" ( a# b# -- div-a#' mod-a#' | a b -- div-a' mod-a' [??] ) [ANS]
 * divide a and b and return both
 * quotient n and remainder m
 */
extern P4_CODE (p4_slash_mod);

/** 0< ( value -- test-flag ) [ANS]
 * return a flag that is true if val is lower than zero
 simulate:
  : 0< 0 < ;
 */
extern P4_CODE (p4_zero_less);

/** 0= ( 0 -- test-flag! | value! -- 0 | value -- test-flag ) [ANS]
 * return a flag that is true if val is just zero
 simulate:
  : 0= 0 = ;
 */
extern P4_CODE (p4_zero_equal);

/** 1+ ( value -- value' ) [ANS]
 * return the value incremented by one
 simulate:
  : 1+ 1 + ;
 */
extern P4_CODE (p4_one_plus);

/** 1- ( value -- value' ) [ANS]
 * return the value decremented by one
 simulate:
   : 1- 1 - ;
 */
extern P4_CODE (p4_one_minus);

/** 2! ( x,x variable* -- ) [ANS]
 * double-cell store 
 */
extern P4_CODE (p4_two_store);

/** 2* ( a# -- a#' | a -- a' [??] ) [ANS]
 * multiplies the value with two - but it
 * does actually use a shift1 to be faster
 simulate:
  : 2* 2 * ; ( canonic) : 2* 1 LSHIFT ; ( usual)
 */
extern P4_CODE (p4_two_star);

/** 2/ ( a# -- a#' | a -- a' [??] ) [ANS]
 * divides the value by two - but it
 * does actually use a shift1 to be faster
 simulate:
  : 2/ 2 / ; ( canonic) : 2/ 1 RSHIFT ; ( usual)
 */
extern P4_CODE (p4_two_slash);

/** 2@ ( variable* -- x,x ) [ANS]
 * double-cell fetch
 */
extern P4_CODE (p4_two_fetch);

/** 2DROP ( a b -- ) [ANS]
 * double-cell drop, also used to drop two items
 */
extern P4_CODE (p4_two_drop);

/** 2DUP ( a,a -- a,a a,a ) [ANS]
 * double-cell duplication, also used to duplicate
 * two items
 simulate:
   : 2DUP OVER OVER ; ( wrong would be : 2DUP DUP DUP ; !!) 
 */
extern P4_CODE (p4_two_dup);

/** 2OVER ( a,a b,b -- a,a b,b a,a ) [ANS]
 * double-cell over, see => OVER and => 2DUP
 simulate:
   : 2OVER SP@ 2 CELLS + 2@ ;
 */
extern P4_CODE (p4_two_over);

/** 2SWAP ( a,a b,b -- b,b a,a ) [ANS]
 * double-cell swap, see => SWAP and => 2DUP
 simulate:
   : 2SWAP LOCALS| B1 B2 A1 A2 | B2 B1 A2 A1 ;
 */
extern P4_CODE (p4_two_swap);

/** "(NEST)" ( -- ) [HIDDEN]
 * compiled by => :
 * (see also => (NONAME) compiled by => :NONAME )
 */
extern P4_CODE (p4_colon_RT);

extern P4_CODE (p4_colon_EXIT);

/** ":" ( 'name' -- ) [ANS] [NEW]
 * create a header for a nesting word and go to compiling
 * mode then. This word is usually ended with => ; but
 * the execution of the resulting colon-word can also 
 * return with => EXIT
 */
extern P4_CODE (p4_colon);

/** "((;))" ( -- ) [HIDDEN] [EXIT]
 * compiled by => ; and maybe => ;AND --
 * it will perform an => EXIT
 */
extern P4_CODE (p4_semicolon_execution);

/** ";" ( -- ) [ANS] [EXIT] [END]
 * compiles => ((;)) which does => EXIT the current
 * colon-definition. It does then end compile-mode
 * and returns to execute-mode. See => : and => :NONAME
 */
extern P4_CODE (p4_semicolon);

/** < ( a* b* -- test-flag | a# b# -- test-flag | a b -- test-flag [?] ) [ANS]
 * return a flag telling if a is lower than b
 */
extern P4_CODE (p4_less_than);

/** <# ( -- ) [ANS]
 * see also => HOLD for old-style forth-formatting words
 * and => PRINTF of the C-style formatting - this word
 * does initialize the pictured numeric output space.
 */
extern P4_CODE (p4_less_sh);

/** = ( a* b* -- test-flag | a# b# -- test-flag | a b -- test-flag [?] ) [ANS]
 * return a flag telling if a is equal to b
 */
extern P4_CODE (p4_equals);

/** > ( a* b* -- test-flag | a# b# -- test-flag | a b -- test-flag [?] ) [ANS]
 * return a flag telling if a is greater than b
 */
extern P4_CODE (p4_greater_than);

/** >BODY ( some-xt* -- some-body* ) [ANS]
 * adjust the execution-token (ie. the CFA) to point
 * to the parameter field (ie. the PFA) of a word.
 * this is not a constant operation - most words have their
 * parameters at "1 CELLS +" but CREATE/DOES-words have the
 * parameters at "2 CELLS +" and ROM/USER words go indirect
 * with a rom'ed offset i.e. "CELL + @ UP +"
 */
extern P4_CODE (p4_to_body);

/** >NUMBER ( a,a str-ptr str-len -- a,a' str-ptr' str-len) [ANS]
 * try to convert a string into a number, and place
 * that number at a,a respeciting => BASE
 */
extern P4_CODE (p4_to_number);

/** >R ( value -- R: value ) [ANS]
 * save the value onto the return stack. The return
 * stack must be returned back to clean state before
 * an exit and you should note that the return-stack
 * is also touched by the => DO ... => WHILE loop.
 * Use => R> to clean the stack and => R@ to get the 
 * last value put by => >R
 */
extern P4_CODE (p4_to_r);

extern P4_CODE (p4_to_r_execution);

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
extern P4_CODE (p4_Q_dup);

/** @ ( value* -- value ) [ANS]
 * fetch the value from the variables address
 */
extern P4_CODE (p4_fetch);

/** ABS ( value# -- value#' ) [ANS]
 * return the absolute value
 */
extern P4_CODE (p4_abs);

/** ACCEPT ( buffer-ptr buffer-max -- buffer-len ) [ANS]
 * get a string from terminal into the named input 
 * buffer, returns the number of bytes being stored
 * in the buffer. May provide line-editing functions.
 */
extern P4_CODE (p4_accept);

/** ALIGN ( -- ) [ANS]
 * will make the dictionary aligned, usually to a
 * cell-boundary, see => ALIGNED
 */
extern P4_CODE (p4_align);

/** ALIGNED ( addr -- addr' ) [ANS]
 * uses the value (being usually a dictionary-address)
 * and increment it to the required alignment for the
 * dictionary which is usually in => CELLS - see also
 * => ALIGN
 */
extern P4_CODE (p4_aligned);

/** ALLOT ( allot-count -- ) [ANS]
 * make room in the dictionary - usually called after
 * a => CREATE word like => VARIABLE or => VALUE
 * to make for an array of variables. Does not 
 * initialize the space allocated from the dictionary-heap.
 * The count is in bytes - use => CELLS ALLOT to allocate 
 * a field of cells.
 */
extern P4_CODE (p4_allot);

/** AND ( value mask -- value' ) [ANS]
 * mask with a bitwise and - be careful when applying
 * it to logical values.
 */
extern P4_CODE (p4_and);

/** BEGIN ( -- ) [ANS] [LOOP]
 * start a control-loop, see => WHILE and => REPEAT
 */
extern P4_CODE (p4_begin);

/** C! ( value# variable#* -- | value# variable* [?] ) [ANS]
 * store the byte-value at address, see => !
 */
extern P4_CODE (p4_c_store);

/** C, ( value# -- ) [ANS]
 * store a new byte-value in the dictionary, implicit 1 ALLOT,
 * see => ,
 */
extern P4_CODE (p4_c_comma);

/** C@ ( value#* -- value# | value* -- value# [?] ) [ANS]
 * fetch a byte-value from the address, see => @
 */
extern P4_CODE (p4_c_fetch);

/** CELL+ ( value -- value' ) [ANS]
 * adjust the value by adding a single Cell's width
 * - the value is often an address or offset, see => CELLS
 */
extern P4_CODE (p4_cell_plus);

/** CELLS ( value# -- value#' ) [ANS]
 * scale the value by the sizeof a Cell
 * the value is then often applied to an address or
 * fed into => ALLOT
 */
extern P4_CODE (p4_cells);

/** CHAR ( 'word' -- char# ) [ANS]
 * return the (ascii-)value of the following word's
 * first character. 
 */
extern P4_CODE (p4_char);

/** CHAR+ ( value -- value' ) [ANS]
 * increment the value by the sizeof one char
 * - the value is often a pointer or an offset,
 * see => CHARS
 */
extern P4_CODE (p4_char_plus);

/** CHARS ( value# -- value#' ) [ANS]
 * scale the value by the sizeof a char
 * - the value is then often applied to an address or
 * fed into => ALLOT (did you expect that sizeof(p4char)
 * may actually yield 2 bytes?)
 */
extern P4_CODE (p4_chars);

/** "((CONSTANT))" ( -- ) [HIDDEN]
 * runtime compiled by => CONSTANT
 */
extern P4_CODE (p4_constant_RT);

/** CONSTANT ( value 'name' -- ) [ANS] [DOES: -- value ]
 * => CREATE a new word with runtime => ((CONSTANT))
 * so that the value placed here is returned everytime
 * the constant's name is used in code. See => VALUE
 * for constant-like names that are expected to change
 * during execution of the program. In a ROM-able
 * forth the CONSTANT-value may get into a shared
 * ROM-area and is never copied to a RAM-address.
 */
extern P4_CODE (p4_constant);

/** COUNT ( string-bstr* -- string-ptr' string-len | some* -- some*' some-len [?] ) [ANS]
 * usually before calling => TYPE
 *
 * (as an unwarranted extension, this word does try to be idempotent).
 */
extern P4_CODE (p4_count);

/** CR ( -- ) [ANS]
 * print a carriage-return/new-line on stdout
 */
extern P4_CODE (p4_cr);

/** DECIMAL ( -- ) [ANS]
 * set the => BASE to 10
 simulate:
   : DECIMAL 10 BASE ! ;
 */
extern P4_CODE (p4_decimal);

/** DEPTH ( -- depth# ) [ANS]
 * return the depth of the parameter stack before
 * the call, see => SP@ - the return-value is in => CELLS
 */
extern P4_CODE (p4_depth);

/** "((DO))" ( end# start# -- ) [HIDDEN]
 * compiled by => DO
 */
extern P4_CODE (p4_do_execution);

/** DO ( end# start# | end* start* -- R: some,loop ) [ANS] [LOOP]
 *  pushes $end and $start onto the return-stack ( => >R )
 *  and starts a control-loop that ends with => LOOP or
 *  => +LOOP and may get a break-out with => LEAVE . The
 *  loop-variable can be accessed with => I
 */
extern P4_CODE (p4_do);

/** "((VAR))" ( -- pfa ) [HIDDEN]
 * the runtime compiled by => VARIABLE
 */
extern P4_CODE (p4_variable_RT);

/** "((BUILDS))" ( -- pfa ) [HIDDEN]
 * the runtime compiled by => CREATE which
 * is not much unlike a => VARIABLE 
 * (in ANS Forth Mode we reserve an additional DOES-field)
 */
extern P4_CODE (p4_builds_RT);

/** "((DOES>))" ( -- pfa ) [HIDDEN]
 * runtime compiled by DOES>
 */
extern P4_CODE (p4_does_RT);

/** "(DOES>)" ( -- pfa ) [HIDDEN]
 * execution compiled by => DOES>
 */
extern P4_CODE (p4_does_execution);

/** "DOES>" ( -- does* ) [ANS] [END] [NEW]
 * does twist the last => CREATE word to carry
 * the => (DOES>) runtime. That way, using the
 * word will execute the code-piece following => DOES>
 * where the pfa of the word is already on stack.
 * (note: FIG option will leave pfa+cell since does-rt is stored in pfa)
 */
extern P4_CODE (p4_does);

/** <BUILDS ( 'name' -- ) [FTH]
 *  make a => HEADER whose runtime will be changed later
 *  using => DOES>  <br />
 *  note that ans'forth does not define => <BUILDS and
 *  it suggests to use => CREATE directly. <br />
 *  ... if you want to write FIG-programs in pure pfe then you have
 *  to use => CREATE: to get the FIG-like meaning of => CREATE whereas
 *  the ans-forth => CREATE is the same as =>"<BUILDS"
 : <BUILDS BL WORD HEADER DOCREATE A, 0 A, ;
 */
extern P4_CODE (p4_builds);

/** DROP ( a -- ) [ANS]
 * just drop the word on the top of stack, see => DUP
 */
extern P4_CODE (p4_drop);

/** DUP ( a -- a a ) [ANS]
 * duplicate the cell on top of the stack - so the
 * two topmost cells have the same value (they are
 * equal w.r.t => = ) , see => DROP for the inverse
 */
extern P4_CODE (p4_dup);

/** "(BRANCH)" ( -- ) [HIDDEN]
 * execution compiled by => ELSE - just a simple => BRANCH
 */
extern P4_CODE (p4_branch_execution);

/** "((ELSE))" ( -- ) OBSOLETE (FIXME: to be removed in pfe-34)
 */
extern P4_CODE (p4_else_execution);

/** ELSE ( -- ) [HIDDEN]
 * will compile an => ((ELSE)) => BRANCH that performs an 
 * unconditional jump to the next => THEN - and it resolves 
 * an => IF for the non-true case
 */
extern P4_CODE (p4_else);

/** EMIT ( char# -- ) [ANS]
 * print the char-value on stack to stdout
 */
extern P4_CODE (p4_emit);

/** ENVIRONMENT? ( name-ptr name-len -- 0 | ?? name-flag! ) [ANS]
 * check the environment for a property, usually
 * a condition like questioning the existance of 
 * specified wordset, but it can also return some
 * implementation properties like "WORDLISTS"
 * (the length of the search-order) or "#LOCALS"
 * (the maximum number of locals) 

 * Here it implements the environment queries as a => SEARCH-WORDLIST 
 * in a user-visible vocabulary called => ENVIRONMENT
 : ENVIRONMENT?
   ['] ENVIRONMENT >WORDLIST SEARCH-WORDLIST
   IF  EXECUTE TRUE ELSE  FALSE THEN ;
 */
extern P4_CODE (p4_environment_Q_core);

/** EVALUATE ( str-ptr str-len -- ) [ANS]
 * => INTERPRET the given string, => SOURCE id
 * is -1 during that time.
 */
extern P4_CODE (p4_evaluate);

/** EXECUTE ( some-xt* -- ??? ) [ANS]
 * run the execution-token on stack - this will usually
 * trap if it was null for some reason, see => >EXECUTE
 simulate:
  : EXECUTE >R EXIT ;
 */
extern P4_CODE (p4_execute);

/** EXIT ( -- ) [ANS] [EXIT]
 * will unnest the current colon-word so it will actually
 * return the word calling it. This can be found in the
 * middle of a colon-sequence between => : and => ;
 */
extern P4_CODE (p4_exit);

/** FILL ( mem-ptr mem-len char# -- ) [ANS]
 * fill a memory area with the given char, does now
 * simply call p4_memset()
 */
extern P4_CODE (p4_fill);

/** FIND ( name-bstr* -- name-bstr* 0 | name-xt* -1|1 ) [ANS]
 * looks into the current search-order and tries to find
 * the name string as the name of a word. Returns its
 * execution-token or the original-bstring if not found,
 * along with a flag-like value that is zero if nothing
 * could be found. Otherwise it will be 1 (a positive value)
 * if the word had been immediate, -1 otherwise (a negative
 * value).
 */
extern P4_CODE (p4_find);

/** "FM/MOD" ( n1,n1# n2# -- div-n1# mod-n1# ) [ANS]
 * divide the double-cell value n1 by n2 and return
 * both (floored) quotient n and remainder m 
 */
extern P4_CODE (p4_f_m_slash_mod);

/** HERE ( -- here* ) [ANS]
 * used with => WORD and many compiling words
 simulate:   : HERE DP @ ;
 */
extern P4_CODE (p4_here);

/** HOLD ( char# -- ) [ANS]
 * the old-style forth-formatting system -- this
 * word adds a char to the picutred output string.
 */
extern P4_CODE (p4_hold);

/** I ( R: some,loop -- S: i# ) [ANS]
 * returns the index-value of the innermost => DO .. => LOOP
 */
extern P4_CODE (p4_i);

extern P4_CODE (p4_i_execution);

/** "(?BRANCH)" ( -- ) [HIDDEN]
 * execution word compiled by => IF - just some simple => ?BRANCH
 */
extern P4_CODE (p4_q_branch_execution);

/** "((IF))" ( -- ) OBSOLETE (FIXME: to be removed in pfe-34)
 * use =>"(?BRANCH)"
 */
extern P4_CODE (p4_if_execution);

/** IF ( value -- ) [ANS]
 * checks the value on the stack (at run-time, not compile-time)
 * and if true executes the code-piece between => IF and the next
 * => ELSE or => THEN . Otherwise it has compiled a branch over
 * to be executed if the value on stack had been null at run-time.
 */
extern P4_CODE (p4_if);

/** IMMEDIATE ( -- ) [ANS]
 * make the => LATEST word immediate, see also => CREATE
 */
extern P4_CODE (p4_immediate);

/** INVERT ( value# -- value#' ) [ANS]
 * make a bitwise negation of the value on stack.
 * see also => NEGATE
 */
extern P4_CODE (p4_invert);

/** J ( R: some,loop -- S: j# ) [ANS]
 * get the current => DO ... => LOOP index-value being
 * the not-innnermost. (the second-innermost...)
 * see also for the other loop-index-values at
 * => I and => K
 */
extern P4_CODE (p4_j);

extern P4_CODE (p4_j_execution);

/** KEY ( -- char# ) [ANS]
 * return a single character from the keyboard - the
 * key is not echoed.
 */
extern P4_CODE (p4_key);

/** LEAVE ( R: some,loop -- R: some,loop ) [ANS]
 * quit the innermost => DO .. => LOOP  - it does even
 * clean the return-stack and branches to the place directly
 * after the next => LOOP
 */
extern P4_CODE (p4_leave);

extern P4_CODE (p4_leave_execution);

/** "((LIT))" ( -- value ) [HIDDEN]
 * execution compiled by => LITERAL
 */
extern P4_CODE (p4_literal_execution);

/** LITERAL ( C: value -- S: value ) [ANS]
 * if compiling this will take the value from the compiling-stack 
 * and puts in dictionary so that it will pop up again at the
 * run-time of the word currently in creation. This word is used
 * in compiling words but may also be useful in making a hard-constant
 * value in some code-piece like this:
 : DCELLS [ 2 CELLS ] LITERAL * ; ( will save a multiplication at runtime)
 * (in most configurations this word is statesmart and it will do nothing
 *  in interpret-mode. See =>"LITERAL," for a non-immediate variant)
 */
extern P4_CODE (p4_literal);

/** "((LOOP))" ( -- ) [HIDDEN]
 * execution compiled by => LOOP
 */
extern P4_CODE (p4_loop_execution);

/** LOOP ( R: some,loop -- ) [ANS] [REPEAT]
 * resolves a previous => DO thereby compiling => ((LOOP)) which
 * does increment/decrement the index-value and branch back if
 * the end-value of the loop has not been reached.
 */
extern P4_CODE (p4_loop);

/** LSHIFT ( value# shift-count -- value#' ) [ANS]
 * does a bitwise left-shift on value
 */
extern P4_CODE (p4_l_shift);

/** M* ( a# b# -- a,a#' ) [ANS]
 * multiply and return a double-cell result
 */
extern P4_CODE (p4_m_star);

/** MAX ( a# b# -- a#|b# | a* b* -- a*|b* | a b -- a|b [??] ) [ANS]
 * return the maximum of a and b
 */
extern P4_CODE (p4_max);

/** MIN ( a# b# -- a#|b# | a* b* -- a*|b* | a b -- a|b [??] ) [ANS]
 * return the minimum of a and b
 */
extern P4_CODE (p4_min);

/** MOD ( a# b# -- mod-a# | a b# -- mod-a# [??] ) [ANS]
 * return the module of "a mod b"
 */
extern P4_CODE (p4_mod);

/** MOVE ( from-ptr to-ptr move-len -- ) [ANS]
 * p4_memcpy an area
 */
extern P4_CODE (p4_move);

/** NEGATE ( value# -- value#' ) [ANS]
 * return the arithmetic negative of the (signed) cell
 simulate:   : NEGATE -1 * ;
 */
extern P4_CODE (p4_negate);

/** OR ( a b# -- a' | a# b -- b' | a b -- a' [??] ) [ANS]
 * return the bitwise OR of a and b - unlike => AND this
 * is usually safe to use on logical values
 */
extern P4_CODE (p4_or);

/** OVER ( a b -- a b a ) [ANS]
 * get the value from under the top of stack. The inverse
 * operation would be => TUCK
 */
extern P4_CODE (p4_over);

/** "((POSTPONE))" ( -- ) [HIDDEN]
 * execution compiled by => POSTPONE
 */
extern P4_CODE (p4_postpone_execution);

/** POSTPONE ( [word] -- ) [ANS]
 * will compile the following word at the run-time of the
 * current-word which is a compiling-word. The point is that
 * => POSTPONE takes care of the fact that word may be 
 * an IMMEDIATE-word that flags for a compiling word, so it
 * must be executed (and not pushed directly) to compile
 * sth. later. Choose this word in favour of => COMPILE
 * (for non-immediate words) and => [COMPILE] (for immediate
 * words)
 */
extern P4_CODE (p4_postpone);

/** QUIT ( -- ) [EXIT]
 * this will throw and lead back to the outer-interpreter.
 * traditionally, the outer-interpreter is called QUIT
 * in forth itself where the first part of the QUIT-word
 * had been to clean the stacks (and some other variables)
 * and then turn to an endless loop containing => QUERY 
 * and => EVALUATE (otherwise known as => INTERPRET )
 * - in pfe it is defined as a => THROW ,
 : QUIT -56 THROW ;
 */
extern P4_CODE (p4_quit);

/** R> ( R: a -- a R: ) [ANS]
 * get back a value from the return-stack that had been saved
 * there using => >R . This is the traditional form of a local
 * var space that could be accessed with => R@ later. If you
 * need more local variables you should have a look at => LOCALS|
 * which does grab some space from the return-stack too, but names
 * them the way you like.
 */
extern P4_CODE (p4_r_from);

extern P4_CODE (p4_r_from_execution);

/** R@ ( R: a -- a R: a ) [ANS]
 * fetch the (upper-most) value from the return-stack that had
 * been saved there using =>">R" - This is the traditional form of a local
 * var space. If you need more local variables you should have a 
 * look at => LOCALS| , see also =>">R" and =>"R>" . Without LOCALS-EXT
 * there are useful words like =>"2R@" =>"R'@" =>'R"@' =>'R!' 
 */
extern P4_CODE (p4_r_fetch);

extern P4_CODE (p4_r_fetch_execution);

/** RECURSE ( ? -- ? ) [ANS]
 * when creating a colon word the name of the currently-created
 * word is smudged, so that you can redefine a previous word
 * of the same name simply by using its name. Sometimes however
 * one wants to recurse into the current definition instead of
 * calling the older defintion. The => RECURSE word does it 
 * exactly this.
   traditionally the following code had been in use:
   : GREAT-WORD [ UNSMUDGE ] DUP . 1- ?DUP IF GREAT-WORD THEN ;
   now use
   : GREAT-WORD DUP . 1- ?DUP IF RECURSE THEN ;
 */
extern P4_CODE (p4_recurse);

/** REPEAT ( -- ) [ANS] [REPEAT]
 * ends an unconditional loop, see => BEGIN
 */
extern P4_CODE (p4_repeat);

/** ROT ( a b c -- b c a ) [ANS]
 * rotates the three uppermost values on the stack,
 * the other direction would be with => -ROT - please
 * have a look at => LOCALS| and => VAR that can avoid 
 * its use.
 */
extern P4_CODE (p4_rot);

/** RSHIFT ( value# shift-count# -- value#' ) [ANS]
 * does a bitwise logical right-shift on value
 * (ie. the value is considered to be unsigned)
 */
extern P4_CODE (p4_r_shift);

/** '((S"))' ( -- string-ptr string-len ) [HIDDEN]
 * execution compiled by => S"
 */
extern P4_CODE (p4_s_quote_execution);

/** 'S"' ( [string<">] -- string-ptr string-len) [ANS]
 * if compiling then place the string into the currently
 * compiled word and on execution the string pops up
 * again as a double-cell value yielding the string's address
 * and length. To be most portable this is the word to be
 * best being used. Compare with =>'C"' and non-portable => "
 */
extern P4_CODE (p4_s_quote);

/** S>D ( a# -- a,a#' | a -- a,a#' [??] ) [ANS]
 * signed extension of a single-cell value to a double-cell value
 */
extern P4_CODE (p4_s_to_d);

/** SIGN ( a# -- ) [ANS]
 * put the sign of the value into the hold-space, this is
 * the forth-style output formatting, see => HOLD
 */
extern P4_CODE (p4_sign);

/** SM/REM ( a,a# b# -- div-a# rem-a# ) [ANS]
 * see => /MOD or => FM/MOD or => UM/MOD or => SM/REM
 */
extern P4_CODE (p4_s_m_slash_rem);

/** SOURCE ( -- buffer* IN-offset# ) [ANS]
 *  the current point of interpret can be gotten through SOURCE.
 *  The buffer may flag out TIB or BLK or a FILE and IN gives
 *  you the offset therein. Traditionally, if the current SOURCE
 *  buffer is used up, => REFILL is called that asks for another
 *  input-line or input-block. This scheme would have made it
 *  impossible to stretch an [IF] ... [THEN] over different blocks,
 *  unless [IF] does call => REFILL
 */
extern P4_CODE (p4_source);

/** SPACE ( -- ) [ANS]
 * print a single space to stdout, see => SPACES
 simulate:    : SPACE  BL EMIT ;
 */
extern P4_CODE (p4_space);

/** SPACES ( space-count -- ) [ANS]
 * print n space to stdout, actually a loop over n calling => SPACE ,
 * but the implemenation may take advantage of printing chunks of
 * spaces to speed up the operation.
 */
extern P4_CODE (p4_spaces);

/** SWAP ( a b -- b a ) [ANS]
 * exchanges the value on top of the stack with the value beneath it
 */
extern P4_CODE (p4_swap);

/** THEN ( -- ) [ANS]
 * does resolve a branch coming from either => IF or => ELSE
 */
extern P4_CODE (p4_then);

/** TYPE ( string-ptr string-len -- ) [ANS]
 * prints the string-buffer to stdout, see => COUNT and => EMIT
 */
extern P4_CODE (p4_type);

/** U. ( value# -- | value -- [?] ) [ANS]
 * print unsigned number to stdout
 */
extern P4_CODE (p4_u_dot);

/** U< ( a b -- test-flag ) [ANS]
 * unsigned comparison, see => <
 */
extern P4_CODE (p4_u_less_than);

/** UM* ( a# b# -- a,a#' ) [ANS]
 * unsigned multiply returning double-cell value
 */
extern P4_CODE (p4_u_m_star);

/** "UM/MOD" ( a,a# b# -- div-a#' mod-a#' ) [ANS]
 * see => /MOD and => SM/REM
 */
extern P4_CODE (p4_u_m_slash_mod);

/** UNLOOP ( R: some,loop -- ) [ANS]
 * drop the => DO .. => LOOP runtime variables from the return-stack,
 * usually used just in before an => EXIT call. Using this multiple
 * times can unnest multiple nested loops.
 */
extern P4_CODE (p4_unloop);

extern P4_CODE (p4_unloop_execution);

/** UNTIL ( test-flag -- ) [ANS] [REPEAT]
 * ends an control-loop, see => BEGIN and compare with => WHILE
 */
extern P4_CODE (p4_until);

/** VARIABLE ( 'name' -- ) [ANS] [DOES: -- name* ]
 * => CREATE a new variable, so that everytime the variable is
 * name, the address is returned for using with => @ and => !
 * - be aware that in FIG-forth VARIABLE did take an argument
 * being the initial value. ANSI-forth does different here.
 */
extern P4_CODE (p4_variable);

/** WHILE ( test-flag -- ) [ANS]
 * middle part of a => BEGIN .. => WHILE .. => REPEAT 
 * control-loop - if cond is true the code-piece up to => REPEAT
 * is executed which will then jump back to => BEGIN - and if
 * the cond is null then => WHILE will branch to right after
 * the => REPEAT
 * (compare with => UNTIL that forms a => BEGIN .. => UNTIL loop)
 */
extern P4_CODE (p4_while);

/** WORD ( delimiter-char# -- here* ) [ANS]
 * read the next => SOURCE section (thereby moving => >IN ) up
 * to the point reaching $delimiter-char - the text is placed
 * at => HERE - where you will find a counted string. You may
 * want to use => PARSE instead.
 */
extern P4_CODE (p4_word);

/** XOR ( a# b# -- a#' ) [ANS]
 * return the bitwise-or of the two arguments - it may be unsafe
 * use it on logical values. beware.
 */
extern P4_CODE (p4_xor);

/** [ ( -- ) [ANS]
 * leave compiling mode - often used inside of a colon-definition
 * to make fetch some very constant value and place it into the
 * currently compiled colon-defintion with => , or => LITERAL
 * - the corresponding unleave word is => ]
 */
extern P4_CODE (p4_left_bracket);

/** ['] ( [name] -- name-xt* ) [ANS] 
 * will place the execution token of the following word into
 * the dictionary. See => ' for non-compiling variant.
 */
extern P4_CODE (p4_bracket_tick);

/** [CHAR] ( [word] -- char# ) [ANS]
 * in compile-mode, get the (ascii-)value of the first charachter
 * in the following word and compile it as a literal so that it
 * will pop up on execution again. See => CHAR and forth-83 => ASCII
 */
extern P4_CODE (p4_bracket_char);

/** ] ( -- ) [ANS]
 * enter compiling mode - often used inside of a colon-definition
 * to end a previous => [ - you may find a  => , or => LITERAL
 * nearby in example texts.
 */
extern P4_CODE (p4_right_bracket);

/** ".(" ( [message<closeparen>] -- ) [ANS]
 * print the message to the screen while reading a file. This works
 * too while compiling, so you can whatch the interpretation/compilation
 * to go on. Some Forth-implementations won't even accept a => ." message"
 * outside compile-mode while the (current) pfe does.
 */
extern P4_CODE (p4_dot_paren);

/** .R ( value# precision# -- | value precision# -- [??] ) [ANS]
 * print with precision - that is to fill
 * a field of the give prec-with with 
 * right-aligned number from the converted value
 */
extern P4_CODE (p4_dot_r);

/** "0<>" ( 0 -- 0 | value! -- value-flag! | value -- value-flag ) [ANS]
 * returns a logical-value saying if the value was not-zero.
 * This is most useful in turning a numerical value into a 
 * boolean value that can be fed into bitwise words like
 * => AND and => XOR - a simple => IF or => WHILE doesn't
 * need it actually.
 */
extern P4_CODE (p4_zero_not_equals);

/** 0> ( 0 -- 0 | value! -- value-flag! | value -- value-flag ) [ANS]
 * return value greater than zero
 simulate:    : 0> 0 > ;
 */
extern P4_CODE (p4_zero_greater);

/** 2>R ( a,a -- R: a,a ) [ANS]
 * save a double-cell value onto the return-stack, see => >R
 */
extern P4_CODE (p4_two_to_r);

extern P4_CODE (p4_two_to_r_execution);

/** 2R> ( R: a,a -- a,a R: ) [ANS]
 * pop back a double-cell value from the return-stack, see => R>
 * and the earlier used => 2>R
 */
extern P4_CODE (p4_two_r_from);

extern P4_CODE (p4_two_r_from_execution);

/** 2R@ ( R: a,a -- a,a R: a,a ) [ANS]
 * fetch a double-cell value from the return-stack, that had been
 * previously been put there with =>"2>R" - see =>"R@" for single value.
 * This can partly be a two-cell => LOCALS| value,  without LOCALS-EXT
 * there are alos other useful words like =>"2R!" =>"R'@" =>'R"@'
 */
extern P4_CODE (p4_two_r_fetch);

extern P4_CODE (p4_two_r_fetch_execution);

/** "(NONAME)" ( -- ) [HIDDEN]
 * compiled by => :NONAME
 * (see also => (NEST) compiled by => : (execution is identical))
 */
extern P4_CODE (p4_colon_noname_RT);

extern P4_CODE (p4_colon_noname_EXIT);

/** :NONAME ( -- C: noname,word ) [ANS]
 * start a colon nested-word but do not use => CREATE - so no name
 * is given to the colon-definition that follows. When the definition
 * is finished at the corresponding => ; the start-address (ie.
 * the execution token) can be found on the outer cs.stack that may
 * be stored used elsewhere then.
 */
extern P4_CODE (p4_colon_noname);

/** "<>" ( a b -- a-flag ) [ANS]
 * return true if a and b are not equal, see => = 
 */
extern P4_CODE (p4_not_equals);

/** "((?DO))" ( a b -- ) [HIDDEN]
 * execution compiled by => ?DO
 */
extern P4_CODE (p4_Q_do_execution);

/** ?DO ( end# start# | end* start* -- R: some,loop ) [ANS]
 * start a control-loop just like => DO - but don't execute
 * atleast once. Instead jump over the code-piece if the loop's
 * variables are not in a range to allow any loop.
 */
extern P4_CODE (p4_Q_do);

/** AGAIN ( -- ) [ANS] [REPEAT]
 * ends an infinite loop, see => BEGIN and compare with
 * => WHILE
 */
extern P4_CODE (p4_again);

/** '((C"))' ( -- string-bstr* ) [HIDDEN]
 * execution compiled by => C" string"
 */
extern P4_CODE (p4_c_quote_execution);

/** 'C"' ( [string<">] -- string-bstr* ) [ANS]
 * in compiling mode place the following string in the current
 * word and return the address of the counted string on execution.
 * (in exec-mode use a => POCKET and leave the bstring-address of it),
 * see => S" string" and the non-portable => " string"
 */
extern P4_CODE (p4_c_quote);

/** CASE ( value -- value ) [ANS]
 * start a CASE construct that ends at => ENDCASE
 * and compares the value on stack at each => OF place
 */
extern P4_CODE (p4_case);

/** "COMPILE," ( some-xt* -- ) [ANS]
 * place the execution-token on stack into the dictionary - in
 * traditional forth this is not even the least different than
 * a simple => , but in call-threaded code there's a big 
 * difference - so COMPILE, is the portable one. Unlike 
 * => COMPILE , => [COMPILE] and => POSTPONE this word does
 * not need the xt to have actually a name, see => :NONAME
 */
extern P4_CODE (p4_compile_comma);

/** CONVERT ( a,a# string-bstr* -- a,a# a-len# ) [ANS] [OLD]
 * digit conversion, obsolete, superseded by => >NUMBER
 */
extern P4_CODE (p4_convert);

/** ENDCASE ( value -- ) [ANS]
 * ends a => CASE construct that may surround multiple sections of
 * => OF ... => ENDOF code-portions. The => ENDCASE has to resolve the
 * branches that are necessary at each => ENDOF to point to right after
 * => ENDCASE
 */
extern P4_CODE (p4_endcase);

/** ENDOF ( -- ) [ANS]
 * resolve the branch need at the previous => OF to mark
 * a code-piece and leave with an unconditional branch
 * at the next => ENDCASE (opened by => CASE )
 */
extern P4_CODE (p4_endof);

/** ERASE ( buffer-ptr buffer-len -- ) [ANS]
 * fill an area will zeros.
 2000 CREATE DUP ALLOT ERASE
 */
extern P4_CODE (p4_erase);

/** EXPECT ( str-ptr str-len -- ) [ANS] [OLD]
 * input handling, see => WORD and => PARSE and => QUERY
 * the input string is placed at str-adr and its length
 in => SPAN - this word is superceded by => ACCEPT
 */
extern P4_CODE (p4_expect);

/** HEX ( -- ) [ANS]
 * set the input/output => BASE to hexadecimal
 simulate:        : HEX 16 BASE ! ;
 */
extern P4_CODE (p4_hex);

/** MARKER ( 'name' -- ) [ANS]
 * create a named marker that you can use to => FORGET ,
 * running the created word will reset the dict/order variables
 * to the state at the creation of this name.
 : MARKER PARSE-WORD (MARKER) ;
 * see also => ANEW which is not defined in ans-forth but which uses
 * the => MARKER functionality in the way it should have been defined.
 : MARKER PARSE-WORD (MARKER) ;
 */
extern P4_CODE (p4_marker);

/** NIP ( a b -- b ) [ANS]
 * drop the value under the top of stack, inverse of => TUCK
 simulate:        : NIP SWAP DROP ;
 */
extern P4_CODE (p4_nip);

/** "((OF))" ( check val -- check ) [HIDDEN]
 * execution compiled by => OF
 */
extern P4_CODE (p4_of_execution);

/** OF ( value test -- value ) [ANS]
 * compare the case-value placed lately with the comp-value 
 * being available since => CASE - if they are equal run the 
 * following code-portion up to => ENDOF after which the
 * case-construct ends at the next => ENDCASE
 */
extern P4_CODE (p4_of);

/** PAD ( -- pad* ) [ANS]
 * transient buffer region 
 */
extern P4_CODE (p4_pad);

/** PARSE ( delim-char# -- buffer-ptr buffer-len ) [ANS]
 * parse a piece of input (not much unlike WORD) and place
 * it into the given buffer. The difference with word is
 * also that => WORD would first skip any delim-char while
 * => PARSE does not and thus may yield that one. In a newer
 * version, => PARSE will not copy but just return the word-span
 * being seen in the input-buffer - therefore a transient space.
 */
extern P4_CODE (p4_parse);

/** PARSE-WORD ( "chars" -- buffer-ptr buffer-len ) [ANS]
 * the ANS'94 standard describes this word in a comment
 * under =>"PARSE", section A.6.2.2008 - quote:
 * 
 * Skip leading spaces and parse name delimited by a space. c-addr 
 * is the address within the input buffer and u is the length of the
 * selected string. If the parse area is empty, the resulting string 
 * has a zero length. 
 *
 * If both => PARSE and => PARSE-WORD are present, the need for => WORD is 
 * largely eliminated. 
 */
extern P4_CODE (p4_parse_word);

/** PICK ( value ...[n-1] n -- value ...[n-1] value ) [ANS]
 * pick the nth value from under the top of stack and push it
 * note that
   0 PICK -> DUP         1 PICK -> OVER
 */
extern P4_CODE (p4_pick);

/** REFILL ( -- refill-flag ) [ANS]
 * try to get a new input line from the => SOURCE and set
 * => >IN accordingly. Return a flag if sucessful, which is
 * always true if the current input comes from a 
 * terminal and which is always false if the current input
 * comes from => EVALUATE - and may be either if the 
 * input comes from a file
 */
extern P4_CODE (p4_refill);

/** RESTORE-INPUT ( input...[input-len] input-len -- ) [ANS]
 * inverse of => SAVE-INPUT
 */
extern P4_CODE (p4_restore_input);

/** ROLL ( value ...[n-1] n -- ...[n-1] value ) [ANS]
 * the extended form of => ROT
    2 ROLL -> ROT
 */
extern P4_CODE (p4_roll);

/** SAVE-INPUT ( -- input...[input-len] input-len ) [ANS]
 * fetch the current state of the input-channel which
 * may be restored with => RESTORE-INPUT later
 */
extern P4_CODE (p4_save_input);

/** "((TO))" ( value -- ) [HIDDEN]
 * execution compiled by => TO
 */
extern P4_CODE (p4_to_execution);

/** TO ( value [name] -- ) [ANS]
 * set the parameter field of name to the value, this is used
 * to change the value of a => VALUE and it can be also used
 * to change the value of => LOCALS|
 */
extern P4_CODE (p4_to);

/** TUCK ( a b -- b a b ) [ANS]
 * shove the top-value under the value beneath. See => OVER
 * and => NIP
 simulate:    : TUCK  SWAP OVER ;
 */
extern P4_CODE (p4_tuck);

/** U.R ( value# precision# -- ) [ANS]
 * print right-aligned in a prec-field, treat value to
 * be unsigned as opposed to => .R
 */
extern P4_CODE (p4_u_dot_r);

/** U> ( a b -- a-flag ) [ANS]
 * unsigned comparison of a and b, see => >
 */
extern P4_CODE (p4_u_greater_than);

/** UNUSED ( -- unused-len ) [ANS]
 * return the number of cells that are left to be used
 * above => HERE
 */
extern P4_CODE (p4_unused);

/** "((VALUE))" ( -- value ) [HIDDEN]
 * runtime compiled by => VALUE
 */
extern P4_CODE (p4_value_RT);

/** VALUE ( value 'name' -- ) [HIDDEN] [DOES: -- value ]
 * => CREATE a word and initialize it with value. Using it
 * later will push the value back onto the stack. Compare with
 * => VARIABLE and => CONSTANT - look also for => LOCALS| and
 * => VAR
 */
extern P4_CODE (p4_value);

/** WITHIN ( a# b# c# -- a-flag | a* b* c* -- a-flag ) [ANS]
 * a widely used word, returns ( b <= a && a < c ) so
 * that is very useful to check an index a of an array
 * to be within range b to c
 */
extern P4_CODE (p4_within);

/** [COMPILE] ( [word] -- ) [ANS]
 * while compiling the next word will be place in the currently
 * defined word no matter if that word is immediate (like => IF )
 * - compare with => COMPILE and => POSTPONE
 */
extern P4_CODE (p4_bracket_compile);

/** "\\" ( [comment<eol>] -- ) [ANS]
 * eat everything up to the next end-of-line so that it is
 * getting ignored by the interpreter. 
 */
extern P4_CODE (p4_backslash);

/** '"' ( [string<">] -- string-bstr* ) [FTH]
    or perhaps ( [string<">] -- string-ptr string-len )
 *  This is the non-portable word which is why the ANSI-committee
 *  on forth has created the two other words, namely => S" and => C" ,
 *  since each implementation (and in pfe configurable) uses another
 *  runtime behaviour. FIG-forth did return bstring which is the configure
 *  default for pfe.
 */
extern P4_CODE (p4_quote);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
