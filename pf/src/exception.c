/** 
 * --    The Optional Exception Word Set
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.8 $
 *     (modified $Date: 2008-09-11 01:27:20 $)
 *
 *  @description
 *      These words implement an exception system in the
 *      widely known => THROW &amp; => CATCH concept.
 *
 *      see the PFE-SIG wordset for catching OS traps.
 *
 */


#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <setjmp.h>

#include "config.h"
#include "types.h"
#include "const.h"
#include "macro.h"
#include "listwords.h"
#include "thread.h"

#include "compiler.h"
#include "exception.h"
#include "interpret.h"
#include "terminal.h"

/************************************************************************/
jmp_buf jump_loop;		/* QUIT and ABORT do a THROW which longjmp() */
				/* here thus C-stack gets cleaned up too */

p4_Except *catchframe = NULL;	/* links to chain of CATCHed words */
				/* and no exceptions to be caught */
/************************************************************************/
/**
 * just call longjmp on PFE.loop
 */
void pf_longjmp_loop(int arg)
{
    longjmp (jump_loop, arg);
}

/************************************************************************/
typedef struct p4_Exception p4_Exception;
struct p4_Exception
{
    struct p4_Exception* next;
    p4cell id;
    char const * name;
};

p4cell next_exception = 0;
p4_Exception* exception_link;

/*
 * show the error, along with info like the block, filename, line numer.
 */
static void show_error (const char* str)
{
    if (! str) str = "";
    int len = strlen(str);
    pf_outf ("\nError: %.*s ", len, str);

    if (PFE.word.ptr && PFE.word.len)
    {
        pf_type (PFE.word.ptr, PFE.word.len);
    }
    pf_longjmp_abort ();
}

static void throw_msg (int id, char *msg)
{
    static const char *throw_explanation[] =
    {
        /*  -1 */ NULL, /* ABORT */
        /*  -2 */ NULL, /* ABORT" */
        /*  -3 */ "stack overflow",
        /*  -4 */ "stack underflow",
        /*  -5 */ "return-stack overflow",
        /*  -6 */ "return-stack underflow",
        /*  -7 */ "do-loops nested too deeply during execution",
        /*  -8 */ "dictionary overflow",
        /*  -9 */ "invalid memory address",
        /* -10 */ "division by zero",
        /* -11 */ "result out of range",
        /* -12 */ "argument type mismatch",
        /* -13 */ "undefined word ",
        /* -14 */ "interpreting a compile-only word",
        /* -15 */ "invalid FORGET (not between FENCE and HERE)",
        /* -16 */ "attempt to use a zero-length string as a name",
        /* -17 */ "pictured numeric output string overflow",
        /* -18 */ "parsed string overflow (input token longer than 255)",
        /* -19 */ "definition name too long",
        /* -20 */ "write to a read-only location",
        /* -21 */ "unsupported operation",
        /* -22 */ "control structure mismatch",
        /* -23 */ "address alignment exception",
        /* -24 */ "invalid numeric argument",
        /* -25 */ "return stack imbalance",
        /* -26 */ "loop parameters unavailable",
        /* -27 */ "invalid recursion",
        /* -28 */ "user interrupt",
        /* -29 */ "compiler nesting (exec/comp state incorrect)",
        /* -30 */ "obsolescent feature",
        /* -31 */ ">BODY used on non-CREATEDd definition",
        /* -32 */ "invalid name argument",
        /* -33 */ "block read exception",
        /* -34 */ "block write exception",
        /* -35 */ "invalid block number",
        /* -36 */ "invalid file position",
        /* -37 */ "file I/O exception",
        /* -38 */ "non-existent file",
        /* -39 */ "unexpected end of file",
        /* -40 */ "invalid BASE for floating-point conversion",
        /* -41 */ "loss of precision",
        /* -42 */ "floating-point divide by zero",
        /* -43 */ "floating-point result out of range",
        /* -44 */ "floating-point stack overflow",
        /* -45 */ "floating-point stack underflow",
        /* -46 */ "floating-point invalid argument",
        /* -47 */ "CURRENT deleted (forget on DEFINITIONS vocabulary)",
        /* -48 */ "invalid POSTPONE",
        /* -49 */ "search-order overflow (ALSO failed)",
        /* -50 */ "search-order underflow (PREVIOUS failed)",
        /* -51 */ "compilation word list changed",
        /* -52 */ "control flow stack overflow",
        /* -53 */ "exception stack overflow",
        /* -54 */ "floating-point underflow",
        /* -55 */ "floating-point unidentified fault",
        /* -56 */ NULL, /* QUIT */
        /* -57 */ "error in sending or receiving a character",
        /* -58 */ "[IF], [ELSE] or [THEN] error",
        /* these Forth200x THROW-IORS:X are not used in PFE */
        /* -59 */ "ALLOCATE error",
        /* -60 */ "FREE error",
        /* -61 */ "RESIZE error",
        /* -62 */ "CLOSE-FILE error",
        /* -63 */ "CREATE-FILE error",
        /* -64 */ "DELETE-FILE error",
        /* -65 */ "FILE-POSITION error",
        /* -66 */ "FILE-SIZE error",
        /* -67 */ "FILE-STATUS error",
        /* -68 */ "FLUSH-FILE error",
        /* -69 */ "OPEN-FILE error",
        /* -70 */ "READ-FILE error",
        /* -71 */ "READ-LINE error",
        /* -72 */ "RENAME-FILE error",
        /* -73 */ "REPOSITION-FILE error",
        /* -74 */ "RESIZE-FILE error",
        /* -75 */ "WRITE-FILE error",
        /* -76 */ "WRITE-LINE error",
    };

    if (-1 - DIM (throw_explanation) < id && id <= -1)
    {
        /* ANS-Forth throw codes, messages are in throw_explanation[] */
        strcpy (msg, throw_explanation[-1 - id]);
    }
    else if (-1024 < id && id <= -256)
    {
        /* Signals, see signal-ext.c,
	   those not handled and not fatal lead to THROW */
        sprintf (msg, "Received signal %d", -256 - id);
    }
    else if (-2048 < id && id <= -1024)
    {
        /* File errors, see FX_IOR / P4_IOR(flag) */
        sprintf (msg, "I/O Error %d : %s", -1024-id, strerror (-1024-id));
    }
    else if (-32767 < id && id <= -2048)
    {
	/* search the exception_link for our id */
	p4_Exception* expt = exception_link;
	strcpy (msg, "module-specific error-condition");
	while (expt)
	{
	    if (expt->id == id)
	    {
		strcpy (msg, expt->name);
		break;
	    }
	    expt = expt->next;
	}
    }
    else if (0 < id)
    {
	strcpy (msg, strerror (id));
    }
    else
    {
        sprintf (msg, "%d THROW unassigned", id);
    }
}

/**
 * the CATCH impl
 */
int p4_catch (p4xt xt)
{
    register int returnvalue;
    auto p4_Except frame;

    frame.magic = P4_EXCEPTION_MAGIC;
    frame.ipp = IP;
    frame.spp = SP;
#if defined PF_WITH_FLOATING
    frame.fpp = FP;
#endif
    frame.rpp = RP;
    frame.prev = catchframe;  catchframe = &frame;
    returnvalue = setjmp (frame.jmp);
    if (! returnvalue) {
        pf_call (xt);
    }
    catchframe = frame.prev;
    RP = frame.rpp;
    return returnvalue;
}

/** CATCH ( catch-xt* -- 0 | throw#! ) [ANS]
 * execute the given execution-token and catch
 * any exception that can be caught therein.
 * software can arbitrarily raise an exception
 * using => THROW - the value 0 means there
 * was no exception, other denote implementation
 * dependent exception-codes.
 */
FCode (p4_catch)
{
    p4cell catch_code = p4_catch ((p4xt) *SP++);
    *--SP = catch_code;
}


/**
 * the THROW impl
 */
void p4_throwstr (int id, const char* description)
{
    p4_Except *frame = catchframe;
    char msg[256];
    char* addr = (char*) description;
    int len = 0;
    if (description)
        len = strlen(description);

    if (frame && frame->magic == P4_EXCEPTION_MAGIC)
    {
        IP = frame->ipp;
        SP = frame->spp;
#if defined PF_WITH_FLOATING
        FP = frame->fpp;
#endif
	RP = frame->rpp;
        longjmp (frame->jmp, id);
    }

    *--RP = IP;
    CSP = (p4cell*) RP;         /* come_back marker */
    switch (id)
    {
     case P4_ON_ABORT_QUOTE:
     {
	 show_error (addr);
     }
     case P4_ON_ABORT:
         pf_longjmp_abort ();
     case P4_ON_QUIT:
         pf_longjmp_quit ();
     default:
         throw_msg (id, msg);
         if (addr)
         {
             strcat (msg, " : ");
             if (! len)
                 strcat (msg, addr);
             else
             {
                 msg[len+strlen(msg)] = '\0';
                 strncat (msg, addr, len);
             }
         }
         show_error (msg);
    }
}

void p4_throw (int id)
{
    p4_throwstr (id, NULL);
}

/** THROW ( throw#! -- [THROW] | throw# -- ) [ANS]
 * raise an exception - it will adjust the depth
 * of all stacks and start interpreting at the point 
 * of the latest => CATCH <br>
 * if n is null nothing happens, the -1 (ie. => FALSE )
 * is the raise-code of => ABORT - the other codes
 * are implementation dependent and will result in
 * something quite like => ABORT
 */
FCode (p4_throw)
{
    p4cell n = *SP++;
    if (n)
	 p4_throw (n);
}

/** ABORT ( -- [THROW] ) [ANS]
 * throw - cleanup some things and go back to the QUIT routine
 : ABORT -1 THROW ;
 */
FCode (pf_abort)
{
    p4_throw (P4_ON_ABORT);
}

/** ((ABORT")) ( -- ) [HIDDEN]
 * compiled by => ABORT" what"
 */ 
FCode_XE (pf_abort_quote_execution)
{
    char msg[256];
    char *p = (char *)IP;
    int l = *p++;
    strncpy ( msg, p, l ); 
    FX_SKIP_STRING;
    if (*SP++ != 0)
        p4_throwstr (P4_ON_ABORT_QUOTE, p);
}
/** 'ABORT"' ( [string<">] -- [THROW] ) [ANS]
 * throw like => ABORT but print an additional error-message
 * to stdout telling what has happened.
 */
FCode (pf_abort_quote)
{
    FX_COMPILE (pf_abort_quote);
    FX (pf_parse_comma_quote);
}
P4COMPILES (pf_abort_quote, pf_abort_quote_execution,
  P4_SKIPS_STRING, P4_DEFAULT_STYLE);

/* ((EXCEPTION-STRING)) ( -- zstring* id )
 */ 
FCode_RT (pf_exception_string_RT)
{
    p4_Exception* expt = (p4_Exception*) WP_PFA;
    *--SP = (p4cell) expt->name;
    *--SP = (p4cell) expt->id;
}

/** (EXCEPTION-STRING: ( exception# [description<closeparen>] -- )
 * append a node with the given id and a pointer to an 
 * extern zstring to the => NEXT-EXCEPTION chain-list.
 */
p4_Runtime2 pf_exception_stringRuntime;
FCode (pf_exception_string)
{
    p4_header_in();
    P4_NAMEFLAGS(LATEST) |= P4xISxRUNTIME;
    FX_RCOMMA (pf_exception_stringRuntime.exec[0]);
    {
      p4cell id = *SP++;
      p4_Exception* expt = (void*) DP;
      DP += sizeof(*expt);
      if (id < next_exception)
         next_exception = id - 1;
      expt->next = exception_link;
      exception_link = expt;
      expt->name = (char*) DP;
      expt->id = id;
    }
    pf_parse_word(')'); /* PARSE-NOHERE-NOTHROW */
    memcpy (DP, PFE.word.ptr, PFE.word.len);
    DP += PFE.word.len;
}
P4RUNTIME1(pf_exception_string, pf_exception_string_RT);

P4_LISTWORDS (exception) =
{
    P4_FXco ("CATCH",			p4_catch),
    P4_FXco ("THROW",			p4_throw),
    P4_FXco ("ABORT",			pf_abort),
    P4_SXco ("ABORT\"",			pf_abort_quote),
};
P4_COUNTWORDS (exception, "Exception");

