#ifndef _PFE_HEADER_EXT_H
#define _PFE_HEADER_EXT_H 1209930552
/* generated 2008-0504-2149 ../../pfe/../mk/Make-H.pl ../../pfe/header-ext.c */

#include <pfe/pfe-ext.h>

/** 
 *  Implements header creation and navigation and defer/synonym words.
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.7 $
 *     (modified $Date: 2008-09-11 01:27:20 $)
 *
 *  @description
 *    Implements header creation and navigation words including the
 *    ones from the forth-83 "experimental annex" targetting definition
 *    field access. Also it has the defer and synonym words that are
 *    almost standard - It is said that the missing DEFER in the ANS Forth
 *    standard of 1994 was just a mistake. 
 */

#ifdef __cplusplus
extern "C" {
#endif




/** >NAME ( cfa -- nfa )
 * converts a pointer to the code-field (CFA) to point
 * then to the corresponding name-field (NFA)
 implementation-specific simulation:
   : >NAME  >LINK L>NAME ;
 */
extern P4_CODE (p4_to_name);

/** >LINK ( cfa -- lfa )
 * converts a pointer to the code-field (CFA) to point
 * then to the corresponding link-field (LFA) - in some configurations
 * this can be a very slow operation since the system might need to walk
 * through all header-words in the system, looking for a =>">NAME" that
 * has the cfa and *then* returning the "N>LINK" result here - which might
 * be none at all if the word is a =>":NONAME". Use always =>">NAME" and
 * treat this word as non-portable just like any assumption about the
 * contents of the =>">LINK"-field.
 * Only in fig-mode and for traditional fig-mode programs, this word may
 * possibly have enough extra assertions to be somewhat reliable.
 * (and fig-mode did not know about =>"SYNONYM"s - see note at =>"LINK>").
 */
extern P4_CODE (p4_to_link);

/** BODY> ( pfa -- cfa )
 * trying to convert a pointer to the parameter-field (PFA) to point
 * then to the corresponding code-field (CFA) - note that this is not
 * necessarily the inverse of =>">BODY" instead it is a fast implementation
 * assuming a =>"VARIABLE" thing had been used. Every use of "BODY>" is
 * warned in the logfile.
 implementation-specific simulation:
   : BODY> CELL - ;
 * 
 */
extern P4_CODE (p4_body_from);

/** NAME> ( nfa -- cfa )
 * converts a pointer to the name-field (NFA) to point
 * then to the corresponding code-field (CFA)
 * 
 * In all cases but a => SYNONYM the pfe will behave not unlike the
 * original fig-forth did - being identical to => N>LINK => LINK> .
 */
extern P4_CODE (p4_name_from);

/** LINK> ( lfa -- cfa )
 * converts a pointer to the link-field (LFA) to point
 * then to the corresponding code-field (CFA)
 *
 * BEWARE: this one does not care about =>"SYNONYM"s and it is the
 * only way to get at the data of a =>"SYNONYM". Therefore, if you have
 * a synonym called A for an old word B then there is a different
 * result using "NAME>" on an A-nfa or using "N>LINK LINK>" since the
 * first "NAME>" will return the xt of B while the latter will return
 * the xt of A - but executing an xt of A is an error and it will => THROW
 *
 * this difference is intentional to allow knowledgable persons to
 * do weird things looking around in the dictionary. The forth standard
 * words will not give you much of a chance to get hold of the nfa of
 * a => SYNONYM word anyway - asking => FIND for a word A will return
 * the execution token of B immediatly and "NAME>" on that one lead to
 * the nfa of B and not that of A.
 */
extern P4_CODE (p4_link_from);

/** L>NAME ( lfa -- nfa )
 * converts a pointer to the link-field (LFA) to point
 * then to the corresponding name-field (CFA) - this one is one of
 * the slowest operation available. One should always use the inverse
 * operation =>"N>LINK" and cache an older value if that is needed.
 * Some words might be linked but they do not have a name-field (just
 * the other fields) but this word can not detect that and will try to look
 * into the bits of the dictionary anway in the assumption that there is 
 * something - and if done in the wrong place it might even segfault.
 * Only in fig-mode and for traditional fig-mode programs, this word may
 * possibly have enough extra assertions to be somewhat reliable.
 * (and fig-mode did not know about =>"SYNONYM"s - see note at =>"LINK>").

 implementation-specific configure-dependent fig-only simulation:
 : L>NAME BEGIN DUP C@ 128 AND 0= WHILE 1- REPEAT ;
 */
extern P4_CODE (p4_l_to_name);

/** N>LINK ( nfa -- lfa )
 * converts a pointer to the name-field (NFA) to point
 * then to the corresponding link-field (LFA) - this operation
 * is quicker than the inverse =>"L>NAME". This word is a specific
 * implementation detail and should not be used by normal users - instead
 * use always =>"NAME>" which is much more portable. Many systems may
 * possibly not even have a =>">LINK"-field in the sense that a => @ on
 * this adress will lead to another =>">NAME". Any operation on the 
 * resulting =>">LINK"-adress is even dependent on the current configuration
 * of PFE - only in fig-mode you are asserted to have the classic detail.
 * (and fig-mode did not know about =>"SYNONYM"s - see note at =>"LINK>").

 implementation-specific configure-dependent fig-only simulation:
   : N>LINK  C@ + ;
 */
extern P4_CODE (p4_n_to_link);

/** >FFA ( nfa -- ffa ) obsolete
 * converts a pointer to the name-field (NFA) to point
 * then to the corresponding flag-field (FFA) - in traditinal
 * Forth this is the same address. pfe _can_ do different.
 implementation-specific configure-dependent simulation:
   : FFA  1- ;
 */
extern P4_CODE (p4_to_ffa);

/** FFA> ( ffa -- nfa ) obsolete
 * converts a pointer to the flag-field (FFA) to point
 * then to the corresponding name-field (NFA) - in traditinal
 * Forth this is the same address. pfe _can_ do different.
 implementation-specific configure-dependent simulation:
   : FFA  1+ ;
 */
extern P4_CODE (p4_ffa_from);

/** NAME>STRING        ( name-token -- str-ptr str-len )
 * convert a name-token into a string-span, used to detect the
 * name for a word and print it. The word =>"ID." can be defined as
 : ID. NAME>STRING TYPE ;
 * the implementation of => NAME>STRING depends on the header layout
 * that is defined during the configuration of the forth system.
 : NAME>STRING COUNT 31 AND ; ( for fig-like names )
 : NAME>STRING COUNT ;        ( default, name is a simple counted string )
 : NAME>STRING @ ZCOUNT ;     ( name-token is a pointer to a C-level string )
 : NAME>STRING COUNT 31 AND   ( hybrid of fig-like and zero-terminated )
      DUP 31 = IF DROP 1+ ZCOUNT THEN
 ;
 : NAME>STRING HEAD:: COUNT CODE:: PAD PLACE PAD ; ( different i86 segments )
*/
extern P4_CODE (p4_name_to_string);

/** HEADER, ( str-ptr str-len -- )
 * => CREATE a new header in the dictionary from the given string, without CFA
 usage: : VARIABLE  BL WORD COUNT HEADER, DOVAR , ;
 */
extern P4_CODE (p4_header_comma);

/** $HEADER ( bstring -- )
 * => CREATE a new header in the dictionary from the given string
 * with the variable runtime (see =>"HEADER," and =>"CREATE:")
 usage: : VARIABLE  BL WORD $HEADER ;
 */
extern P4_CODE (p4_str_header);

/** LATEST ( -- nfa )
 * return the NFA of the lateset definition in the
 * => CURRENT vocabulary
 */
extern P4_CODE (p4_latest);

/** SMUGDE ( -- )
 * the FIG definition toggles the => SMUDGE bit, and not all systems have
 * a smudge bit - instead one should use => REVEAL or => HIDE
 : SMUDGE LAST @ >FFA SMUDGE-MASK TOGGLE ;
 : SMUDGE LAST @ NAME-FLAGS@ SMUDGE-MASK XOR LAST @ NAME-FLAGS! ;
 : HIDE   LAST @ NAME-FLAGS@ SMUDGE-MASK  OR LAST @ NAME-FLAGS! ;
 */
extern P4_CODE (p4_smudge);

/** HIDE ( -- )
 * the FIG definition toggles the => SMUDGE bit, and not all systems have
 * a smudge bit - instead one should use => REVEAL or => HIDE
 : HIDE LAST @ FLAGS@ SMUDGE-MASK XOR LAST @ FLAGS! ;
 */
extern P4_CODE (p4_hide);

/** REVEAL ( -- ) 
 * the FIG definition toggles the => SMUDGE bit, and not all systems have
 * a smudge bit - instead one should use => REVEAL or => HIDE
 : REVEAL LAST @ FLAGS@ SMUDGE-MASK INVERT AND LAST @ FLAGS! ;
 : REVEAL LAST @ CHAIN-INTO-CURRENT ;
 */
extern P4_CODE (p4_reveal);

/** NAME-FLAGS@ ( nfa -- nfa-flags )
 * get the nfa-flags that corresponds to the nfa given. Note that
 * in the fig-style would include the nfa-count in the lower bits.
 * (see =>"NAME-FLAGS!")
 */
extern P4_CODE (p4_name_flags_fetch);

/** NAME-FLAGS! ( nfa-flags nfa -- )
 * set the nfa-flags of nfa given. Note that in the fig-style the nfa-flags
 * would include the nfa-count in the lower bits - therefore this should only
 * set bits that had been previously retrieved with => NAME-FLAGS@
 : IMMEDIATE LAST @ NAME-FLAGS@ IMMEDIATE-MASK OR LAST @ NAME-FLAGS! ;
 */
extern P4_CODE (p4_name_flags_store);

/** ((DEFER)) ( -- )
 * runtime of => DEFER words
 */
extern P4_CODE (p4_defer_RT);

/** DEFER ( 'word' -- )
 * create a new word with ((DEFER))-semantics
 simulate:
   : DEFER  CREATE 0, DOES> ( the ((DEFER)) runtime ) 
      @ ?DUP IF EXECUTE THEN ;
   : DEFER  DEFER-RT HEADER 0 , ;
 *
 * declare as <c>"DEFER deferword"</c>  <br>
 * and set as <c>"['] executionword IS deferword"</c>
 * (in pfe, you can also use <c>TO deferword</c> to set the execution)
 */
extern P4_CODE (p4_defer);

extern P4_CODE (p4_is_execution);

/** IS ( xt-value [word] -- )
 * set a => DEFER word
 * (in pfe: set the DOES-field - which is the BODY-field in ans-mode
 *  and therefore the same as => TO /  in fig-mode the DOES-field is
 *  one cell higher up than for a => CREATE: => VARIABLE 
 *  Use => IS freely on each DOES-words in both modes).
 : IS ' 
   STATE @ IF LITERAL, POSTPONE >DOES-BODY POSTPONE ! 
   ELSE >DOES-BODY ! THEN 
 ; IMMEDIATE
 */
extern P4_CODE (p4_is);

/** BEHAVIOR ( xt1 -- xt2 )
 * get the execution token xt2 that would be executed by the => DEFER
 * identified by xt1.
 *
 * This command is used to obtain the execution contents of a deferred
 * word. A typical use would be to retrieve and save the execution
 * behavior of the deferred word, set the deferred word to a new behavior,
 * and then later restore the old behavior.
 *
 * If the deferred word identified by _xt1_ is associated with some
 * other deferred word, _xt2_ is the execution token of that other
 * deferred word. To retrieve the execution token of the word currently
 * associated with that other deferred word, use the phrase BEHAVIOR BEHAVIOR .
 *
 * Experience:
 *      Many years of use in OpenBoot and OpenFirmware systems.
 * (Proposed for ANS Forth 2001)
 *
 * In PFE it is the inverse of an => IS operation and it will never fail
 * if applied to a word with atleast a body. That's just like => IS can
 * be applied to almost every =>"DOES>" word where => BEHAVIOR will get
 * the value back.
 */
extern P4_CODE (p4_behavior);

extern P4_CODE (p4_synonym_RT);

extern P4_CODE (p4_obsoleted_RT);

/** SYNONYM ( "newname" "oldname" -- )
 * make an name-alias for a word - this is very different from a => DEFER
 * since a => DEFER will resolve at runtime. Changing the target of a
 * => DEFER via => IS will result in changing the => BEHAVIOR of all
 * words defined earlier and containing the name of the => DEFER.
 *
 * A => SYNONYM however does not have any data field (theoretically not
 * even an execution token), instead it gets resolved at compile time.
 * In theory, you can try to => FIND the name of the => SYNONYM but as
 * soon as you apply =>"NAME>" the execution token of the end-point is
 * returned. This has also the effect that using the inverse =>">NAME"
 * operation will result in the name-token of the other name. 

   SYNONYM CREATE <BUILDS ( like it is in ANS Forth )
   : FOO CREATE DOES> @ ;
   SEE FOO
   : foo <builds
     does> @ ;
   SYNONYM CREATE CREATE:
   : BAR CREATE 10 ALLOT ;
   SEE BAR
   : bar create: 10 allot ;
 *                      (only =>"LINK>" does not care about =>"SYNONYM"s)
 */
extern P4_CODE (p4_synonym);

/** "SYNONYM-OBSOLETED ( "newname" "oldname" -- )
 * same as => SYNONYM but on the first use an error message will be
 * displayed on both the screen and the sys-log.
 */
extern P4_CODE (p4_obsoleted);

extern P4_CODE (p4_deprecated_RT);

/** (DEPRECATED: ( "newname" [message<closeparen>] -- )
 * add a message for the following word "newname" that should
 * be shown once upon using the following word. Use it like
   (DEPRECATED: myword is obsoleted in Forth200X)
   : myword ." hello world" ;
 */
extern P4_CODE (p4_deprecated);

/** EXTERN,-DEPRECATED: ( "newname" zstring* -- )
 * compile a pointer to an extern (loader) z-string
 * to the dictionary and on execution show a deprecation
 * message once. Note: the new name is smudged+immediate,
 * so it you can not => FIND it right after compilation.
 * 
 * see also =>"(DEPRECATED:" name message) for the real thing
 */
extern P4_CODE (p4_extern_deprecated);

/** (CHECK-DEPRECATED) ( xt* -- xt* )
 * an internal function that will check an execution token
 * to have any deprecation attribution - some words have
 * a (one time) message to be shown to the user, while
 * => OBSOLETED-SYNONYM will show a message and rebuild
 * itself as a normal SYNONYM. - Note that most deprecations
 * are only shown once and that they are not emitted when
 * having REDEFINED-MSG OFF.
 */
extern P4_CODE (p4_check_deprecated);


extern P4_CODE (p4_logemssage_RT);

/** (LOGMESSAGE: ( "newname" [message<closeparen>] -- )
 */
extern P4_CODE (p4_logmessage);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
