#ifndef _VOL_8_SRC_CVS_PFE_33_PFE_SMART_GO_EXT_H
#define _VOL_8_SRC_CVS_PFE_33_PFE_SMART_GO_EXT_H 1209868838
/* generated 2008-0504-0440 /vol/8/src/cvs/pfe-33/pfe/../mk/Make-H.pl /vol/8/src/cvs/pfe-33/pfe/smart-go-ext.c */

#include <pfe/pfe-ext.h>

/** 
 * -- smart outer interpreter
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
 *      Smart Outer Interpreter allows to register executions tokens
 *      that get tied to a single char - if the outer interpreter 
 *      Compatiblity with former standards, miscellaneous useful words.
 *      ... for TOOLS-EXT
 */

#ifdef __cplusplus
extern "C" {
#endif




/** SMART-INTERPRET-INIT ( -- )
 * creates a set of interpret-words that are used in the inner
 * interpreter, so if a word is unknown to the interpreter-loop
 * it will use the first char of that word, attach it to an 
 * "interpret-" prefix, and tries to use that =>'IMMEDIATE'-=>'DEFER'-word 
 * on the rest of the word. This => SMART-INTERPRET-INIT will set up
 * words like interpret-" so you can write 
 * <c>"hello"</c>  instead of   <c>" hello"</c>
 * and it creates interpret-\ so that words like <c>\if-unix</c> are
 * ignoring the line if the word <c>\if-unknown</c> is unknown in itself.
 * This is usually <i>not</i> activated on startup.
 */
extern P4_CODE (p4_smart_interpret_init);

/** SMART-INTERPRET! ( -- )
 * enables/disables the SMART-INTERPRET extension in => INTERPRET ,
 * (actually stores an XT in => DEFER inside the mainloop interpreter)
 */
extern P4_CODE (p4_smart_interpret_store);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
