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

jmp_buf jump_loop;
void pf_longjmp_loop(int arg);

#define pf_longjmp_abort()	(pf_longjmp_loop('A'))
#define pf_longjmp_exit()	(pf_longjmp_loop('X'))
#define pf_longjmp_quit()	(pf_longjmp_loop('Q'))
#define pf_longjmp_yield()	(pf_longjmp_loop('S'))

typedef struct p4_Except p4_Except; /* an exception frame */

struct p4_Except
{
    p4cell magic;
    p4xt** rpp;              /* P4_REGRP_T */
    p4xt *ipp;               /* P4_REGIP_T */
    p4cell *spp;                /* P4_REGSP_T */
    p4cell *lpp;                /* P4_REGLP_T */
    double *fpp;                /* P4_REGFP_T */
    jmp_buf jmp;
    p4_Except *prev;
};

/**
 * the CATCH impl
 */
int p4_catch (p4xt xt) ; /*{*/

/**
 * the THROW impl
 */
void p4_throwstr (int id, const char* description);
void p4_throw (int id);

#endif
