/**
 *  -- The Optional Floating-Point Word Set
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *  Copyright (C) Pierre Bazonnard 2013 - 2026.
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.13 $
 *     (modified $Date: 2008-09-11 01:27:20 $)
 *
 *  @description
 *         The Optional Floating-Point Wordset is not usually
 *         used on embedded platforms. The PFE can be configured
 *         to even not allocate the separate floating-point stack
 *         that most of the floating-point words refer to.
 *
 *  @author  Pierre Bazonnard
 *  @description
 *         Reduced wordset for my PF
 *  
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <errno.h>

#include "config.h"
#include "types.h"
#include "const.h"
#include "macro.h"

#include "compiler.h"
#include "interpret.h"
#include "terminal.h"

#if defined PF_WITH_FLOATING

/* ------------------------------------------------------------------ */
p4fcell* fstack;   /* floating point stack */
p4fcell* f0;
p4fcell* fp;       /* the floating point stack pointer */

/* ------------------------------------------------------------------ */
#define P4_DFALIGNED(P)	(((size_t)(P) & (SIZEOF_FCELL - 1)) == 0)
/* ------------------------------------------------------------------ */

/**
 * precision is a Forth variable
 * floating point output precision
 */
p4cell precision;

/**
 * return the address of precision on the stack
 */
FCode (pf_precision)            
{
    *--SP = (p4cell)&precision;
}

/**
 * return float-aligned address
 */
p4cell p4_dfaligned (p4cell n)
{
    while (!P4_DFALIGNED (n))
        n++;
    return n;
}

FCode (p4_d_f_align)
{
    while (!P4_DFALIGNED (DP))
        *DP++ = 0;
}

FCode (p4_d_f_aligned)
{
    SP[0] = p4_dfaligned (SP[0]);
}

/**
 *  used in engine
 *  return 0 if error
 */
int pf_to_float (const char *p, p4cell n, p4fcell *r)
{
/* most systems have good strtod */
    char *endptr;
    *r = 0.0;
    if (!*p)
        return 1;

    errno = 0;    /* To distinguish success/failure after call */
    *r = strtod (p, &endptr);
    if ( errno != 0 )
        return 0;
    /* strtod() successfully parsed a number if next char is NULL or SPACE */
    if ( *endptr=='\0' || *endptr==' ' )
        return 1;
    return 0;
}

/*
 * >FLOAT ( c-addr u -- true | false ) ( F: -- r | )
 *
 * This is a working solution on most machines.
 * Unfortunately it relies on pretty obscure features of sscanf()
 * which are not truly implemented everywhere.
 */
FCode (pf_to_float)
{
    const char *p;
    p4cell n;
    p4fcell r;

    n = (int)*SP++;
    p = (char*) *SP;
    
    if ( pf_to_float( p, n, &r ) == 0 ) {
        *SP = P4_FALSE;
    } else {
        *--fp = r;
        *SP = P4_TRUE;
    }
}

/** ( n -- f: r )
 */
FCode (pf_s_to_f)
{
    *--fp = (p4fcell)(*SP++);
}

/** ( f: r -- n )
 * n is the integer representation of r
 * we use truncation towards zero.
 */
FCode (pf_f_to_s)
{
    *--SP = rint (*fp++);
}

FCode (p4_f_store)
{
    *(p4fcell *) *SP++ = *fp++;
}

FCode (p4_f_star)
{
    fp[1] *= fp[0];
    fp++;
}

FCode (p4_f_plus)
{
    fp[1] += fp[0];
    fp++;
}

FCode (p4_f_minus)
{
    fp[1] -= fp[0];
    fp++;
}

FCode (p4_f_slash)
{
    fp[1] /= fp[0];
    fp++;
}

FCode (p4_f_zero_less)
{
    *--SP = P4_FLAG (*fp++ < 0);
}

FCode (p4_f_zero_equal)
{
    *--SP = P4_FLAG (*fp++ == 0);
}

FCode (p4_f_less_than)
{
    *--SP = P4_FLAG (fp[1] < fp[0]);
    fp += 2;
}

FCode (p4_f_fetch)
{
    *--fp = *(p4fcell *) *SP++;
}

FCode (p4_f_constant_RT)
{
    *--fp = *(p4fcell *)WP_PFA;
}

FCode (p4_f_constant)
{
    p4_header_in();
    P4_NAMEFLAGS(LATEST) |= P4xISxRUNTIME;
    FX_RUNTIME1 (p4_f_constant);
    FX_FCOMMA (*fp++);
}
P4RUNTIME1(p4_f_constant, p4_f_constant_RT);

FCode (p4_f_depth)
{
    *--SP = f0 - fp;
}

FCode (p4_f_drop)
{
    fp++;
}

FCode (p4_f_dup)
{
    fp--;
    fp[0] = fp[1];
}

FCode (p4_f_literal_execution)
{
    *--fp = *((*(p4fcell **)&(IP))++);
}

FCode (p4_f_literal)
{
    if (STATE) {
# if SIZEOF_FCELL > SIZEOF_CELL
        if (P4_DFALIGNED (DP))
            FX_COMPILE (p4_f_literal);
#endif
        FX_COMPILE (p4_f_literal);
        FX_FCOMMA (*fp++);
    }
}
P4COMPILE (p4_f_literal, p4_f_literal_execution, P4_SKIPS_FLOAT);

FCode (p4_floor)
{
  *fp = floor (*fp);
}

FCode (p4_f_max)
{
    if (fp[0] > fp[1])
        fp[1] = fp[0];
    fp++;
}

FCode (p4_f_min)
{
    if (fp[0] < fp[1])
        fp[1] = fp[0];
    fp++;
}

FCode (p4_f_negate)
{
    *fp = -*fp;
}

FCode (p4_f_over)
{
    fp--;
    fp[0] = fp[2];
}

FCode (p4_f_rot)
{
    p4fcell h = fp[2];

    fp[2] = fp[1];
    fp[1] = fp[0];
    fp[0] = h;
}

FCode (p4_f_round)
{
    /* correct and fast */
    *fp = rint (*fp);
}

FCode (p4_f_swap)
{
    p4fcell h = fp[1];

    fp[1] = fp[0];
    fp[0] = h;
}

FCode (p4_f_variable_RT)
{
    *--SP = (p4cell) WP_PFA;
}

FCode (p4_f_variable)
{
    p4_header_in();
    P4_NAMEFLAGS(LATEST) |= P4xISxRUNTIME;
    FX_RUNTIME1 (p4_f_variable);
    FX_FCOMMA (0.);
}
P4RUNTIME1(p4_f_variable, p4_f_variable_RT);

FCode (p4_represent)		/* with help from Lennart Benshop */
{
    char *p, buf[0x80];
    int u, log, sign;
    p4fcell f;

    f = *fp++;
    p = (char *) SP[1];
    u = SP[0];
    SP--;

    sign = (f < 0.0);
    f = fabs(f);
    if (u > 1) {
        /* one digit is always present before the decimal point */
        sprintf (buf, "%.*e", u-1, f);
        *p = buf[0];
        memcpy(p+1, buf+2, u-1);
        log = atoi(buf+u+2) + 1;
    } else if (u > 0) {
        /* sprintf(2) says "if precision is zero then no decimal
         * point will be present in the output string" */
        sprintf (buf, "%.*e", 0, f);
        *p = buf[0];
        log = atoi(buf+2) + 1;
    } else {
        log = 0;
    }
    if (f == 0) log = 0;

    SP[2] = log;
    SP[1] = P4_FLAG(sign);
    SP[0] = P4_TRUE;
}

/* ********************************************************************** */
/* Floating point extension words:                                        */
/* ********************************************************************** */

FCode (pf_f_dot)
{
    pf_outf ("%.*f ", (int)precision, *fp++);
}

FCode (pf_f_e_dot)
{
    pf_outf ("%.*e ", (int)precision, *fp++);
}

FCode (p4_float_plus)
{
    *SP += SIZEOF_FCELL;
}

FCode (p4_floats)
{
    *SP += SIZEOF_FCELL;
}

FCode (p4_f_star_star)
{
    fp[1] = pow (fp[1], fp[0]);
    fp++;
}

FCode (p4_f_abs)
{
  *fp = fabs (*fp);
}

FCode (p4_s_f_store)
{
    *(float *) *SP++ = *fp++;
}

FCode (p4_s_f_fetch)
{
    *--fp = *(float *) *SP++;
}

/*
FCode (p4_s_float_plus)
{
    *SP += sizeof (float);
}

FCode (p4_s_floats)
{
    *SP *= sizeof (float);
}
*/

/*-- simple mappings to the ANSI-C library  --*/
/*
FCode (p4_f_acos)	{ *fp = acos (*fp); }
FCode (p4_f_acosh)	{ *fp = acosh (*fp); }
FCode (p4_f_alog)	{ *fp = pow10 (*fp); }
FCode (p4_f_asin)	{ *fp = asin (*fp); }
FCode (p4_f_asinh)	{ *fp = asinh (*fp); }
FCode (p4_f_atan)	{ *fp = atan (*fp); }
FCode (p4_f_atan2)	{ fp [1] = atan2 (fp [1], fp [0]); fp++; }
FCode (p4_f_atanh)	{ *fp = atanh (*fp); }
FCode (p4_f_cos)	{ *fp = cos (*fp); }
FCode (p4_f_cosh)	{ *fp = cosh (*fp); }
FCode (p4_f_exp)	{ *fp = exp (*fp); }
FCode (p4_f_expm1)	{ *fp = expm1 (*fp); }
FCode (p4_f_ln)		{ *fp = log (*fp); }
FCode (p4_f_lnp1)	{ *fp = log1p (*fp); }
FCode (p4_f_log)	{ *fp = log10 (*fp); }
FCode (p4_f_sin)	{ *fp = sin (*fp); }
FCode (p4_f_sincos)	{ --fp; fp [0] = cos (fp [1]); fp [1] = sin (fp [1]); }
FCode (p4_f_sinh)	{ *fp = sinh (*fp); }
FCode (p4_f_sqrt)	{ *fp = sqrt (*fp); }
FCode (p4_f_tan)	{ *fp = tan (*fp); }
FCode (p4_f_tanh)	{ *fp = tanh (*fp); }
*/

/* ************************************************** */

P4_LISTWORDS (floating) =
{
    P4_FXco ("PRECISION",  pf_precision),
    P4_FXco ("FALIGN",		 p4_d_f_align),
    P4_FXco ("FALIGNED",	 p4_d_f_aligned),
    P4_FXco (">FLOAT",		 pf_to_float),
    P4_FXco ("S>F",		 pf_s_to_f),
    P4_FXco ("F>S",		 pf_f_to_s),

    P4_FXco ("F!",		 p4_f_store),
    P4_FXco ("F*",		 p4_f_star),
    P4_FXco ("F+",		 p4_f_plus),
    P4_FXco ("F-",		 p4_f_minus),
    P4_FXco ("F/",		 p4_f_slash),
    P4_FXco ("F0<",		 p4_f_zero_less),
    P4_FXco ("F0=",		 p4_f_zero_equal),
    P4_FXco ("F<",		 p4_f_less_than),

    P4_FXco ("F@",		 p4_f_fetch),
    P4_RTco ("FCONSTANT",	 p4_f_constant),
    P4_FXco ("FDEPTH",		 p4_f_depth),
    P4_FXco ("FDROP",		 p4_f_drop),
    P4_FXco ("FDUP",		 p4_f_dup),
    P4_SXco ("FLITERAL",	 p4_f_literal),
    P4_FXco ("FLOAT+",		 p4_float_plus),
    P4_FXco ("FLOATS",		 p4_floats),
    P4_FXco ("FLOOR",		 p4_floor),
    P4_FXco ("FMAX",		 p4_f_max),
    P4_FXco ("FMIN",		 p4_f_min),
    P4_FXco ("FNEGATE",		 p4_f_negate),
    P4_FXco ("FOVER",		 p4_f_over),
    P4_FXco ("FROT",		 p4_f_rot),
    P4_FXco ("FROUND",		 p4_f_round),
    P4_FXco ("FSWAP",		 p4_f_swap),
    P4_RTco ("FVARIABLE",	 p4_f_variable),
    P4_FXco ("REPRESENT",	 p4_represent),

    /* floating point extension words */
    P4_FXco ("F.",		 pf_f_dot),
    P4_FXco ("FE.",		 pf_f_e_dot),
    P4_FXco ("F**",		 p4_f_star_star),
    P4_FXco ("FABS",		 p4_f_abs),
/*
    P4_FXco ("DF!",		 p4_f_store),
    P4_FXco ("DF@",		 p4_f_fetch),
    P4_FXco ("DFALIGN",		 p4_d_f_align),
    P4_FXco ("DFALIGNED",	 p4_d_f_aligned),
    P4_FXco ("DFLOAT+",		 p4_d_float_plus),
    P4_FXco ("DFLOATS",		 p4_d_floats),

    P4_FXco ("FACOS",		 p4_f_acos),
    P4_FXco ("FACOSH",		 p4_f_acosh),
    P4_FXco ("FALOG",		 p4_f_alog),
    P4_FXco ("FASIN",		 p4_f_asin),
    P4_FXco ("FASINH",		 p4_f_asinh),
    P4_FXco ("FATAN",		 p4_f_atan),
    P4_FXco ("FATAN2",		 p4_f_atan2),
    P4_FXco ("FATANH",		 p4_f_atanh),
    P4_FXco ("FCOS",		 p4_f_cos),
    P4_FXco ("FCOSH",		 p4_f_cosh),
    P4_FXco ("FEXP",		 p4_f_exp),
    P4_FXco ("FEXPM1",		 p4_f_expm1),
    P4_FXco ("FLN",		 p4_f_ln),
    P4_FXco ("FLNP1",		 p4_f_lnp1),
    P4_FXco ("FLOG",		 p4_f_log),
    P4_FXco ("FSIN",		 p4_f_sin),
    P4_FXco ("FSINCOS",		 p4_f_sincos),
    P4_FXco ("FSINH",		 p4_f_sinh),
    P4_FXco ("FSQRT",		 p4_f_sqrt),
    P4_FXco ("FTAN",		 p4_f_tan),
    P4_FXco ("FTANH",		 p4_f_tanh),

    P4_FXco ("F~",		 p4_f_proximate),
    P4_FXco ("SF!",		 p4_s_f_store),
    P4_FXco ("SF@",		 p4_s_f_fetch),
    P4_FXco ("SFALIGN",		 p4_align),
    P4_FXco ("SFALIGNED",	 p4_aligned),
    P4_FXco ("SFLOAT+",		 p4_s_float_plus),
    P4_FXco ("SFLOATS",		 p4_s_floats),
*/
};
P4_COUNTWORDS (floating, "Floating point");

#endif
