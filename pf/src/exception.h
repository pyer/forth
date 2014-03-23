#ifndef _PF_EXCEPTION_H
#define _PF_EXCEPTION_H

/** 
 * --  Exception-oriented Subroutines.
 * 
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.6 $
 *     (modified $Date: 2008-05-04 02:57:30 $)
 */

#define p4_jmp_buf jmp_buf
#define p4_setjmp(_buf) setjmp((_buf), 1)
#define p4_longjmp(_buf,_val) longjmp((_buf), (_val))

#define p4_longjmp_abort()	(p4_longjmp_loop('A'))
#define p4_longjmp_exit()	(p4_longjmp_loop('X'))
#define p4_longjmp_quit()	(p4_longjmp_loop('Q'))
#define p4_longjmp_yield()	(p4_longjmp_loop('S'))


FCode (p4_cr_show_input);

/**
 * just call longjmp on PFE.loop
 */
void p4_longjmp_loop(int arg) ; /*{*/

/**
 * the CATCH impl
 */
int p4_catch (p4xt xt) ; /*{*/

void p4_throw (int id) ; /*{*/

void p4_throwstr (int id, const char* description) ; /*{*/

/**
 * the THROW impl
 */
void p4_throws (int id, const p4_char_t* description, int len) ; /*{*/

#endif
