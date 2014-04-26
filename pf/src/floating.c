/**
 *  -- The Optional Floating-Point Word Set
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *  Copyright (C) Pierre Bazonnard 2013 - 2014.
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
#include <float.h>
#include <math.h>
#include <errno.h>

#include "config.h"
#include "types.h"
#include "const.h"
#include "macro.h"
#include "listwords.h"
#include "thread.h"

//#include "compiler.h"
//#include "exception.h"
//#include "interpret.h"
#include "terminal.h"

#if defined PF_WITH_FLOATING

#define CELLBITS	BITSOF (p4cell)

#ifndef signbit
#define signbit(X) ((X) < 0)
#endif

/* ------------------------------------------------------------------
 * Exact comparison of raw floats.  The following is intended to
 * capture the sizes listed for IEEE 754 by William Kahan,
 * "Fclass: a Proposed Classification of Standard Floating-Point
 * Operands", March 2, 2002.  He lists 4, >=6, 8, 10, 12, or 16
 * 8-bit bytes.  We assume that 4, 8, 10, 12, or 16 might
 * correspond to a double, and probably 4 could be omitted.
 * --KM&DNW 2Mar03
 */
#if PFE_SIZEOF_DOUBLE == PFE_SIZEOF_INT
#  define EXACTLY_EQUAL(A,B)  ( *((int*) &(A)) == *((int*) &(B)) )
#elif PFE_SIZEOF_DOUBLE == 2 * PFE_SIZEOF_INT
#  define EXACTLY_EQUAL(A,B) \
        ( *((int*) &(A)) == *((int*) &(B)) \
       && *(((int*) &(A)) + 1) == *(((int*) &(B)) + 1) )
#elif PFE_SIZEOF_DOUBLE == 2 * PFE_SIZEOF_INT + PFE_SIZEOF_SHORT
#  define EXACTLY_EQUAL(A,B) \
        ( *((int*) &(A)) == *((int*) &(B)) \
       && *(((int*) &(A)) + 1) == *(((int*) &(B)) + 1) \
       && (short)*(((int*) &(A)) + 2) == (short)*(((int*) &(B)) + 2) )
#elif PFE_SIZEOF_DOUBLE == 3 * PFE_SIZEOF_INT
#  define EXACTLY_EQUAL(A,B) \
        ( *((int*) &(A)) == *((int*) &(B)) \
       && *(((int*) &(A)) + 1) == *(((int*) &(B)) + 1) \
       && *(((int*) &(A)) + 2) == *(((int*) &(B)) + 2) )
#elif PFE_SIZEOF_DOUBLE == 4 * PFE_SIZEOF_INT
#  define EXACTLY_EQUAL(A,B) \
        ( *((int*) &(A)) == *((int*) &(B)) \
       && *(((int*) &(A)) + 1) == *(((int*) &(B)) + 1) \
       && *(((int*) &(A)) + 2) == *(((int*) &(B)) + 2) \
       && *(((int*) &(A)) + 3) == *(((int*) &(B)) + 3) )
#else
#  define EXACTLY_EQUAL(A,B)  (p4_memcmp (&(A), &(B), sizeof (double)) == 0)
#  ifdef __GNUC__
#  warning using p4_memcmp() in p4_f_proximate()
#  elif !defined _PFE_FLOATING_USING_MEMCMP
#  error   using p4_memcmp() in p4_f_proximate()
#  endif
#endif

#define PFE_ALIGNOF_DFLOAT PFE_SIZEOF_DOUBLE
#define P4_DFALIGNED(P)	(((size_t)(P) & (PFE_ALIGNOF_DFLOAT - 1)) == 0)
/* ------------------------------------------------------------------ */
FCode (pf_set_precision)
{
    PRECISION = *SP++;
}

/**
 * return double float-aligned address
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
int pf_to_float (const char *p, p4cell n, double *r)
{
/* most systems have good strtod */
    char *endptr;
    *r = 0.0;
    if (!*p)
        return 1;

    errno = 0;    /* To distinguish success/failure after call */
    *r = strtod (p, &endptr);
    /* Check for possible error */
//    if ( errno == ERANGE )
    if ( errno != 0 )
        return 0;

    /* If we got here, strtod() successfully parsed a number */
    //if (*endptr != '\0')      /* Not necessarily an error... */
				/* and it is not in Forth */
    return 1;
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
    double r;

    n = (int)*SP++;
    p = (char*) *SP;
    
    if ( pf_to_float( p, n, &r ) == 0 ) {
        *SP = P4_FALSE;
    } else {
        *--FP = r;
        *SP = P4_TRUE;
    }
}

/** ( n -- f: r )
 */
FCode (pf_s_to_f)
{
    *--FP = (double)(*SP++);
}

/** ( f: r -- n )
 * n is the integer representation of r
 * we use truncation towards zero.
 */
FCode (pf_f_to_s)
{
    *--SP = rint (*FP++);
}

FCode (p4_f_store)
{
    *(double *) *SP++ = *FP++;
}

FCode (p4_f_star)
{
    FP[1] *= FP[0];
    FP++;
}

FCode (p4_f_plus)
{
    FP[1] += FP[0];
    FP++;
}

FCode (p4_f_minus)
{
    FP[1] -= FP[0];
    FP++;
}

FCode (p4_f_slash)
{
    FP[1] /= FP[0];
    FP++;
}

FCode (p4_f_zero_less)
{
    *--SP = P4_FLAG (*FP++ < 0);
}

FCode (p4_f_zero_equal)
{
    *--SP = P4_FLAG (*FP++ == 0);
}

FCode (p4_f_less_than)
{
    *--SP = P4_FLAG (FP[1] < FP[0]);
    FP += 2;
}

FCode (p4_f_fetch)
{
    *--FP = *(double *) *SP++;
}

#if defined PF_TMP
static p4xcode* p4_f_constant_RT_SEE (char* out, p4xt xt, p4char* nfa)
{
    /*  (*P4_TO_CODE(xt) == PFX (p4_f_constant_RT)) */
    sprintf (out, "%g FCONSTANT %.*s",
	     *(double *) p4_dfaligned ((p4cell) P4_TO_BODY (xt)),
	     NAMELEN(nfa), NAMEPTR(nfa));
    return 0; /* no colon */
}

FCode_RT (p4_f_constant_RT)
{
    FX_USE_BODY_ADDR;
    *--FP = *(double *) p4_dfaligned ((p4cell) FX_POP_BODY_ADDR);
}

FCode (p4_f_constant)
{
    FX_RUNTIME_HEADER;
    FX_RUNTIME1 (p4_f_constant);
    FX (p4_d_f_align);
    FX_FCOMMA (*FP++);
}
P4RUNTIMES1_(p4_f_constant, p4_f_constant_RT, 0,p4_f_constant_RT_SEE);
#endif

FCode (p4_f_depth)
{
    *--SP = F0 - FP;
}

FCode (p4_f_drop)
{
    FP++;
}

FCode (p4_f_dup)
{
    FP--;
    FP[0] = FP[1];
}

#if defined PF_TMP
/* originally P4_SKIPS_FLOAT */
p4xcode*
p4_lit_float_SEE (p4xcode* ip, char* p, p4_Semant* s)
{
# if PFE_ALIGNOF_DFLOAT > PFE_ALIGNOF_CELL
    if (!P4_DFALIGNED (ip))
        ip++;
# endif
    sprintf (p, "%e ", *(double *) ip);
    P4_INC (ip, double);

    return ip;
}

FCode_XE (p4_f_literal_execution)
{
    FX_USE_CODE_ADDR;
    *--FP= P4_POP_ (double, IP);
    FX_USE_CODE_EXIT;
}

FCode (p4_f_literal)
{
    _FX_STATESMART_Q_COMP;
    if (STATESMART)
    {
#if PFE_ALIGNOF_DFLOAT > PFE_ALIGNOF_CELL
        if (P4_DFALIGNED (DP))
            FX_COMPILE2 (p4_f_literal);
#endif
        FX_COMPILE1 (p4_f_literal);
        FX_FCOMMA (*FP++);
    }
}
P4COMPILES2 (p4_f_literal, p4_f_literal_execution, p4_noop,
	     p4_lit_float_SEE, P4_DEFAULT_STYLE);
#endif

FCode (p4_floor)
{
  *FP = floor (*FP);
}

FCode (p4_f_max)
{
    if (FP[0] > FP[1])
        FP[1] = FP[0];
    FP++;
}

FCode (p4_f_min)
{
    if (FP[0] < FP[1])
        FP[1] = FP[0];
    FP++;
}

FCode (p4_f_negate)
{
    *FP = -*FP;
}

FCode (p4_f_over)
{
    FP--;
    FP[0] = FP[2];
}

FCode (p4_f_rot)
{
    double h = FP[2];

    FP[2] = FP[1];
    FP[1] = FP[0];
    FP[0] = h;
}

FCode (p4_f_round)
{
    /* correct and fast */
    *FP = rint (*FP);
}

FCode (p4_f_swap)
{
    double h = FP[1];

    FP[1] = FP[0];
    FP[0] = h;
}

#if defined PF_TMP
FCode_RT (p4_f_variable_RT)
{
    FX_USE_BODY_ADDR;
    FX_PUSH_SP = p4_dfaligned ((p4cell) FX_POP_BODY_ADDR);
}

FCode (p4_f_variable)
{
    FX_RUNTIME_HEADER;
    FX_RUNTIME1 (p4_f_variable);
    FX (p4_d_f_align);
    FX_FCOMMA (0.);
}
P4RUNTIME1(p4_f_variable, p4_f_variable_RT);
#endif

FCode (p4_represent)		/* with help from Lennart Benshop */
{
    char *p, buf[0x80];
    int u, log, sign;
    double f;

    f = *FP++;
    p = (char *) SP[1];
    u = SP[0];
    SP--;

    sign = signbit(f);
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
    pf_outf ("%.*f ", (int) PRECISION, *FP++);
}

FCode (pf_f_e_dot)			/* with help from Lennart Benshop */
{
    double f = fabs (*FP);
    double h = 0.5 * pow10 (-PRECISION);
    int n;

    if (f == 0)
        n = 0;
    else if (f < 1)
    {
        h = 1 - h;
        for (n = 3; f * pow10 (n) < h; n += 3);
    }else{
        h = 1000 - h;
        for (n = 0; h <= f * pow10 (n); n -= 3);
    }
    pf_outf ("%+*.*fE%+03d ", (int) PRECISION + 5, (int) PRECISION,
      *FP++ * pow10 (n), -n);
}

FCode (pf_f_s_dot)
{
    pf_outf ("%.*E ", (int) PRECISION, *FP++);
}

FCode (p4_d_float_plus)
{
    *SP += sizeof (double);
}

FCode (p4_d_floats)
{
    *SP *= sizeof (double);
}

FCode (p4_f_star_star)
{
    FP[1] = pow (FP[1], FP[0]);
    FP++;
}

FCode (p4_f_abs)
{
  *FP = fabs (*FP);
}

#if defined PF_TMP
FCode (p4_f_proximate)
{
    double a, b, c;

    a = FP[2];
    b = FP[1];
    c = FP[0];
    FP += 3;
    *--SP = P4_FLAG
        (c > 0
          ? fabs (a - b) < c
          : c < 0
          ? fabs (a - b) < -c * (fabs (a) + fabs (b))
          : EXACTLY_EQUAL (a, b));
}

FCode (p4_s_f_store)
{
    *(float *) *SP++ = *FP++;
}

FCode (p4_s_f_fetch)
{
    *--FP = *(float *) *SP++;
}

FCode (p4_s_float_plus)
{
    *SP += sizeof (float);
}

FCode (p4_s_floats)
{
    *SP *= sizeof (float);
}

/*-- simple mappings to the ANSI-C library  --*/

FCode (p4_f_acos)	{ *FP = acos (*FP); }
FCode (p4_f_acosh)	{ *FP = acosh (*FP); }
FCode (p4_f_alog)	{ *FP = pow10 (*FP); }
FCode (p4_f_asin)	{ *FP = asin (*FP); }
FCode (p4_f_asinh)	{ *FP = asinh (*FP); }
FCode (p4_f_atan)	{ *FP = atan (*FP); }
FCode (p4_f_atan2)	{ FP [1] = atan2 (FP [1], FP [0]); FP++; }
FCode (p4_f_atanh)	{ *FP = atanh (*FP); }
FCode (p4_f_cos)	{ *FP = cos (*FP); }
FCode (p4_f_cosh)	{ *FP = cosh (*FP); }
FCode (p4_f_exp)	{ *FP = exp (*FP); }
#if 1  /* ante C99 */
FCode (p4_f_expm1)	{ *FP = exp (*FP) - 1.0; }
#else  /* post C99 */
FCode (p4_f_expm1)	{ *FP = expm1 (*FP); }
#endif
FCode (p4_f_ln)		{ *FP = log (*FP); }
#if 1  /* ante C99 */
FCode (p4_f_lnp1)	{ *FP = log (*FP + 1.0); }
#else  /* post C99 */
FCode (p4_f_lnp1)	{ *FP = log1p (*FP); }
#endif
FCode (p4_f_log)	{ *FP = log10 (*FP); }
FCode (p4_f_sin)	{ *FP = sin (*FP); }
FCode (p4_f_sincos)	{ --FP; FP [0] = cos (FP [1]); FP [1] = sin (FP [1]); }
FCode (p4_f_sinh)	{ *FP = sinh (*FP); }
FCode (p4_f_sqrt)	{ *FP = sqrt (*FP); }
FCode (p4_f_tan)	{ *FP = tan (*FP); }
FCode (p4_f_tanh)	{ *FP = tanh (*FP); }

/* environment queries */

static FCode (p__floating_stack)
{
    FX_PUSH ((PFE.f0 - PFE.fstack) / sizeof(double));
}

static FCode (p__max_float)
{
    *--FP = DBL_MAX;
}

#endif
/* words not from the ansi'94 forth standard  */
/* ================= INTERPRET =================== */

int pf_convert_float(void)
{
    double f;
    /* scanned word sits at PFE.word. (not at HERE) */
    if ( BASE != 10 )
        return 0; /* quick path */

    /* WORD-string is at HERE */
    if (! pf_to_float (PFE.word.ptr, PFE.word.len, &f))
        return 0; /* quick path */
/*
    if (STATE)
        {
#          if PFE_ALIGNOF_DFLOAT > PFE_ALIGNOF_CELL
	    if (P4_DFALIGNED (DP))
		FX_COMPILE2 (p4_f_literal);
#          endif
        FX_COMPILE1 (p4_f_literal);
        FX_FCOMMA (f);
    } else {
*/
        *--FP = f;
//    }
    return 1;
}

/* ************************************************** */

P4_LISTWORDS (floating) =
{
    P4_DVAL ("PRECISION",	 precision),
    P4_FXco ("SET-PRECISION",	 pf_set_precision),
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
//    P4_RTco ("FCONSTANT",	 p4_f_constant),
    P4_FXco ("FDEPTH",		 p4_f_depth),
    P4_FXco ("FDROP",		 p4_f_drop),
    P4_FXco ("FDUP",		 p4_f_dup),
//    P4_SXco ("FLITERAL",	 p4_f_literal),
    P4_FXco ("FLOAT+",		 p4_d_float_plus),
    P4_FXco ("FLOATS",		 p4_d_floats),
    P4_FXco ("FLOOR",		 p4_floor),
    P4_FXco ("FMAX",		 p4_f_max),
    P4_FXco ("FMIN",		 p4_f_min),
    P4_FXco ("FNEGATE",		 p4_f_negate),
    P4_FXco ("FOVER",		 p4_f_over),
    P4_FXco ("FROT",		 p4_f_rot),
    P4_FXco ("FROUND",		 p4_f_round),
    P4_FXco ("FSWAP",		 p4_f_swap),
//    P4_RTco ("FVARIABLE",	 p4_f_variable),
    P4_FXco ("REPRESENT",	 p4_represent),

    /* floating point extension words */
    P4_FXco ("F.",		 pf_f_dot),
    P4_FXco ("FE.",		 pf_f_e_dot),
    P4_FXco ("FS.",		 pf_f_s_dot),
/*
    P4_FXco ("DF!",		 p4_f_store),
    P4_FXco ("DF@",		 p4_f_fetch),
    P4_FXco ("DFALIGN",		 p4_d_f_align),
    P4_FXco ("DFALIGNED",	 p4_d_f_aligned),
    P4_FXco ("DFLOAT+",		 p4_d_float_plus),
    P4_FXco ("DFLOATS",		 p4_d_floats),
    P4_FXco ("F**",		 p4_f_star_star),
    P4_FXco ("FABS",		 p4_f_abs),
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
