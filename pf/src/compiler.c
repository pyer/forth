/**
 *  Forth compiler
 *  Compile definitions, load-time with load-wordl, runtime with compile-comma
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.15 $
 *     (modified $Date: 2008-09-11 01:27:20 $)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <errno.h>

#include "config.h"
#include "types.h"
#include "const.h"
#include "macro.h"
#include "listwords.h"
#include "thread.h"

#include "compiler.h"
#include "exception.h"
#include "interpret.h"

/* -------------------------------------------------------------- */
p4_Runtime2 P4RUNTIME_(pf_colon);

FCode (p4_drop);
FCode (p4_swap);
FCode (p4_rot);

#define ___ {
#define ____ }

/* -------------------------------------------------------------- */
p4cell *csp;		/* compiler security, saves sp here */
/* -------------------------------------------------------------- */
FCode(pf_noop)
{
    /* well, nothing... */
}
/* -------------------------------------------------------------- */

/** ?CSP ( -- )
 * check that => SP == => CSP otherwise => THROW
 * <br> used in control-words
 */
FCode (pf_Q_csp)	
{
    if (SP != CSP)
        p4_throw (P4_ON_CONTROL_MISMATCH);
}

/** ?COMP ( -- )
 * check that the current => STATE is compiling
 * otherwise => THROW
 * <br> often used in control-words
 */
FCode (pf_Q_comp)		
{
    if (!STATE)
        p4_throw (P4_ON_COMPILE_ONLY);
}

/** ?EXEC ( -- )
 * check that the current => STATE is executing
 * otherwise => THROW
 * <br> often used in control-words
 */
FCode (pf_Q_exec)		
{
    if (STATE)
        p4_throw (P4_ON_COMPILER_NESTING);
}

/** ?PAIRS ( a b -- )
 * if compiling, check that the two magics on
 * the => CS-STACK are identical, otherwise throw
 * <br> used in control-words
 */
void pf_Q_pairs (p4cell n)
{
    if (n != *SP++)
        p4_throw (P4_ON_CONTROL_MISMATCH);
}

FCode (pf_Q_pairs)
{
    FX (pf_Q_comp);
    pf_Q_pairs (*SP++);
}

/** ?STACK ( -- )
 * check all stacks for underflow and overflow conditions,
 * and if such an error condition is detected => THROW
 */
FCode (pf_Q_stack)
{
    if (RP > R0)            p4_throw (P4_ON_RSTACK_UNDER);
    if (RP < PFE.rstack)        p4_throw (P4_ON_RSTACK_OVER);
    if (SP > S0)            p4_throw (P4_ON_STACK_UNDER);
    if (SP < PFE.stack)         p4_throw (P4_ON_STACK_OVER);
#if defined PF_WITH_FLOATING
    if (FP > F0)            p4_throw (P4_ON_FSTACK_UNDER);
    if (FP < PFE.fstack)        p4_throw (P4_ON_FSTACK_OVER);
#endif
    if (dictlimit - MIN_UNUSED < DP) 
        p4_throw (P4_ON_DICT_OVER);  
}

/* -------------------------------------------------------------- */
/** >R ( value -- R: value ) [ANS]
 * save the value onto the return stack. The return
 * stack must be returned back to clean state before
 * an exit and you should note that the return-stack
 * is also touched by the => DO ... => WHILE loop.
 * Use => R> to clean the stack and => R@ to get the
 * last value put by => >R
 */
FCode (pf_to_r)
{
    FX (pf_Q_comp);
    FX_COMPILE (pf_to_r);
}

FCode_XE (pf_to_r_execution)
{
//#define P4_PUSH(X,P)    (*P4_DEC (P, p4cell) = (X))
//#define P4_DEC(P,T)	(--(*(T **)&(P)))
//    P4_PUSH(*SP++, RP);
//    *P4_DEC (RP, p4cell) = *SP++;
    *(--(*(p4cell **)&(RP))) = *SP++;
}
P4COMPILES (pf_to_r, pf_to_r_execution, P4_SKIPS_NOTHING, P4_DEFAULT_STYLE);

/** R> ( R: a -- a R: ) [ANS]
 * get back a value from the return-stack that had been saved
 * there using => >R . This is the traditional form of a local
 * var space that could be accessed with => R@ later. If you
 * need more local variables you should have a look at => LOCALS|
 * which does grab some space from the return-stack too, but names
 * them the way you like.
 */
FCode (pf_r_from)
{
    FX (pf_Q_comp);
    FX_COMPILE (pf_r_from);
}

FCode_XE (pf_r_from_execution)
{
    *--SP = (p4cell) *RP++;
}
P4COMPILES (pf_r_from, pf_r_from_execution, P4_SKIPS_NOTHING, P4_DEFAULT_STYLE);

/** R@ ( R: a -- a R: a ) [ANS]
 * fetch the (upper-most) value from the return-stack that had
 * been saved there using =>">R" - This is the traditional form of a local
 * var space. If you need more local variables you should have a
 * look at => LOCALS| , see also =>">R" and =>"R>" . Without LOCALS-EXT
 * there are useful words like =>"2R@" =>"R'@" =>'R"@' =>'R!'
 */
FCode (pf_r_fetch)
{
    FX (pf_Q_comp);
    FX_COMPILE (pf_r_fetch);
}
FCode_XE (pf_r_fetch_execution)
{
    *--SP = *((p4cell*)RP);
}
P4COMPILES (pf_r_fetch, pf_r_fetch_execution, P4_SKIPS_NOTHING, P4_DEFAULT_STYLE);

/* -------------------------------------------------------------- */
/** <MARK ( -- DP-mark ) compile-only
 * memorizes the current => DP on the CS-STACK
 * used for => <RESOLVE later. Useful for creation of 
 * compiling words, eg. => BEGIN , see => AHEAD
 simulate:
   : <MARK ?COMP  HERE ;
 */
FCode (pf_backward_mark)	
{
    FX (pf_Q_comp);
    *--SP = (p4cell) DP;
}

/** <RESOLVE ( DP-mark -- ) compile-only
 * resolves a previous => <MARK , actually pushes
 * the DP-address memorized at <MARK into the dictionary.
 * Mostly used after => BRANCH or => ?BRANCH in compiling
 * words like => UNTIL
 simulate:
   : <RESOLVE ?COMP  , ;
 */
FCode (pf_backward_resolve)		
{
    FX (pf_Q_comp);
    FX_QCOMMA (*SP++);
}

/** MARK> ( -- DP-mark ) compile-only
 * makes room for a pointer in the dictionary to
 * be resolved through => RESOLVE> and does therefore
 * memorize that cell's address on the CS-STACK
 * Mostly used after => BRANCH or => ?BRANCH in compiling
 * words like => IF or => ELSE
 simulate:
   : MARK> ?COMP  HERE 0 , ;
 */
FCode (pf_forward_mark)	
{
    FX (pf_backward_mark);
    FX_QCOMMA(0);
}

/** RESOLVE> ( DP-mark -- ) compile-only
 * resolves a pointer created by => MARK>
 * Mostly used in compiling words like => THEN
 simulate:
   : RESOLVE> ?COMP  HERE SWAP ! ;
 */
FCode (pf_forward_resolve)
{
    FX (pf_Q_comp);
    *(p4char **) *SP++ = DP;
}

/* -------------------------------------------------------------- */
FCode_XE (pf_semicolon_execution);
FCode (pf_semicolon);

static P4_CODE_RUN(pf_builds_RT_SEE)
{
    strcat (p, "CREATE ");
    strncat (p, (char*) NAMEPTR(nfa), NAMELEN(nfa));
    return 0;
}

/** "((BUILDS))" ( -- pfa ) [HIDDEN]
 * the runtime compiled by => CREATE which
 * is not much unlike a => VARIABLE
 * (in ANS Forth Mode we reserve an additional DOES-field)
 */
FCode (pf_builds_RT)
{
    *--SP = (p4cell)( WP_PFA + 1 );
}

/** CREATE ( 'name' -- ) [ANS]
 * create a name with runtime => ((VAR)) so that everywhere the name is used
 * the pfa of the name's body is returned. This word is not immediate and
 * according to the ANS Forth documents it may get directly used in the
 * first part of a => DOES> defining word - in traditional forth systems
 * the word =>"<BUILDS" was used for that and => CREATE was defined to be
 * the first part of a => VARIABLE word (compare with =>"CREATE:" and the
 * portable expression =>"0" =>"BUFFER:")
 */

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
p4_Runtime2 pf_buildsRuntime;
FCode (pf_builds)
{
    p4_header_in();
    P4_NAMEFLAGS(LATEST) |= P4xISxRUNTIME;
    //FX_RUNTIME1 (pf_builds);
    FX_RCOMMA (pf_buildsRuntime.exec[0]);
    FX_RCOMMA (0);
}
P4RUNTIMES1_(pf_builds, pf_builds_RT, 0, pf_builds_RT_SEE);

/** "(DOES>)" ( -- pfa ) [HIDDEN]
 * execution compiled by => DOES>
 */
FCode_XE (pf_does_execution)
{
    p4xt xt;
    xt = name_to_cfa (LATEST);
    P4_XT_VALUE(xt) = FX_GET_RT (pf_does);
    *P4_TO_DOES_CODE(xt) = IP; /* into CFA[1] */
    FX (pf_semicolon_execution);   /* double-EXIT */
}

/** "DOES>" ( -- does* ) [ANS] [END] [NEW]
 * does twist the last => CREATE word to carry
 * the => (DOES>) runtime. That way, using the
 * word will execute the code-piece following => DOES>
 * where the pfa of the word is already on stack.
 * (note: FIG option will leave pfa+cell since does-rt is stored in pfa)
 */
FCode (pf_does)
{
    if (STATE)
    {
        FX (pf_Q_csp);
        FX_COMPILE (pf_does);
    }else{
        /* NOTE: some details depend on pf_does_execution above */
        p4xt xt;
        FX (pf_align);

        xt = name_to_cfa (LATEST);
        P4_XT_VALUE(xt) = FX_GET_RT (pf_does);
        *P4_TO_DOES_CODE(xt) = (p4xt*) DP; /* into CFA[1] */

        /* now, see pf_colon */
        CSP = SP;
        STATE = P4_TRUE;
    }
}
P4COMPILES (pf_does, pf_does_execution, P4_SKIPS_NOTHING, P4_DOES_STYLE);

/** "((DOES>))" ( -- pfa ) [HIDDEN]
 * runtime compiled by DOES>
 */
FCode_RT (pf_does_RT)
{
    *--SP = (p4cell) P4_TO_DOES_BODY(WP);  /* from CFA[2] */
    *--RP = IP; IP = *P4_TO_DOES_CODE(WP); /* from CFA[1] */
}
P4RUNTIME1(pf_does, pf_does_RT);


/* -------------------------------------------------------------- */
/** "(NEST)" ( -- ) [HIDDEN]
 * compiled by => :
 * (see also => (NONAME) compiled by => :NONAME )
 */
FCode_RT (pf_colon_RT)
{
    *--RP = IP;
    IP = (p4xt *) WP_PFA;
}

/** ":" ( 'name' -- ) [ANS] [NEW]
 * create a header for a nesting word and go to compiling
 * mode then. This word is usually ended with => ; but
 * the execution of the resulting colon-word can also
 * return with => EXIT
 */
p4_Runtime2 pf_colonRuntime;
FCode (pf_colon)
{
    FX (pf_Q_exec);
    p4_header_in();
    P4_NAMEFLAGS(LATEST) |= P4xISxRUNTIME;
    P4_NAMEFLAGS(LATEST) |= P4xSMUDGED;
    //FX_RUNTIME1 (pf_colon);
    FX_RCOMMA (pf_colonRuntime.exec[0]);
    CSP = SP;
    STATE = P4_TRUE;
}
P4RUNTIME1(pf_colon, pf_colon_RT);


/** "((;))" ( -- ) [HIDDEN] [EXIT]
 * compiled by => ; and maybe => ;AND --
 * it will perform an => EXIT
 */
FCode_XE (pf_semicolon_execution)
{
    IP = *RP++;
}

/** ";" ( -- ) [ANS] [EXIT] [END]
 * compiles => ((;)) which does => EXIT the current
 * colon-definition. It does then end compile-mode
 * and returns to execute-mode. See => : and => :NONAME
 */
FCode (pf_semicolon)
{
    FX (pf_Q_csp);
    STATE = P4_FALSE;
    P4_NAMEFLAGS(LATEST) &= ~P4xSMUDGED;
    FX_COMPILE (pf_semicolon);
}

P4COMPILES (pf_semicolon, pf_semicolon_execution, P4_SKIPS_NOTHING, P4_SEMICOLON_STYLE);

/** EXIT ( -- ) [ANS] [EXIT]
 * will unnest the current colon-word so it will actually
 * return the word calling it. This can be found in the
 * middle of a colon-sequence between => : and => ;
 */
FCode (pf_exit)
{
    FX_COMPILE (pf_exit);
}
P4COMPILES (pf_exit, pf_semicolon_execution, P4_SKIPS_NOTHING, P4_DEFAULT_STYLE);

/** IMMEDIATE ( -- ) [ANS]
 * make the => LATEST word immediate, see also => CREATE
 */
FCode (pf_immediate)
{
    P4_NAMEFLAGS(LATEST) |= P4xIMMEDIATE;
}

/* -------------------------------------------------------------- */
/** "," ( value* -- | value# -- | value -- [?] ) [ANS]
 * store the value in the dictionary
 simulate:
   : , DP  1 CELLS DP +!  ! ;
 */
FCode (pf_comma)
{
    FX_VCOMMA (*SP++);
}

/** C, ( value# -- ) [ANS]
 * store a new byte-value in the dictionary, implicit 1 ALLOT,
 * see => ,
 */
FCode (pf_c_comma)
{
    *DP++ = (p4char) *SP++;
}

/* -------------------------------------------------------------- */
#define P4_ALIGNED(P)	(((size_t)(P) & (PFE_ALIGNOF_CELL - 1)) == 0)

/** ALIGN ( -- ) [ANS]
 * will make the dictionary aligned, usually to a
 * cell-boundary, see => ALIGNED
 */
FCode (pf_align)
{
    while (! P4_ALIGNED (DP))
        *DP++ = 0;
}

/**
 * return cell-aligned address
 */
p4cell pf_aligned (p4cell n)
{
    while (!P4_ALIGNED (n))
        n++;
    return n;
}

/** ALIGNED ( addr -- addr' ) [ANS]
 * uses the value (being usually a dictionary-address)
 * and increment it to the required alignment for the
 * dictionary which is usually in => CELLS - see also
 * => ALIGN
 */
FCode (pf_aligned)
{
    *SP = pf_aligned (*SP);
}

/** ALLOT ( allot-count -- ) [ANS]
 * make room in the dictionary - usually called after
 * a => CREATE word like => VARIABLE or => VALUE
 * to make for an array of variables. Does not
 * initialize the space allocated from the dictionary-heap.
 * The count is in bytes - use => CELLS ALLOT to allocate
 * a field of cells.
 */
FCode (pf_allot)
{
    DP += *SP++;
}

/** _._ ( i str* str# base -- str* )
 * This is for internal use only (SEE and debugger),
 * The real => . etc. words use => HOLD and the memory area below => PAD
 */
char pf_number2digit(p4ucell n);

char * p4_str_dot (p4cell n, char *p, int base)
{
    int sign = 0;
    char *bl;
    p4ucell u = (p4ucell)n;

    *--p = '\0';
    bl = p - 1;
    if (n < 0) {
        u = -n;
        sign = 1;
    }
    *--p = '\0';

    do {
    //    *--p = p4_num2dig (p4_u_d_div ((p4udcell *) &d, base));
        udiv_t res;
        res.quot = u / BASE;
        res.rem  = u % BASE;
        u = res.quot;
//    pf_hold (pf_number2digit(res.rem));
        *--p = pf_number2digit(res.rem);
    } while (u);

    if (sign)
       *--p = '-';
    *bl = ' ';
    return p;
}

static P4_CODE_RUN(pf_constant_RT_SEE)
{
    strcat (p, p4_str_dot (*P4_TO_BODY (xt), p+200, BASE));
    strcat (p, "CONSTANT ");
    strncat (p, (char*) NAMEPTR(nfa), NAMELEN(nfa));
    return 0;
}

/** "((CONSTANT))" ( -- ) [HIDDEN]
 * runtime compiled by => CONSTANT
 */
FCode_RT (pf_constant_RT)
{
    *--SP = WP_PFA[0];
}

/** CONSTANT ( value 'name' -- ) [ANS] [DOES: -- value ]
 * => CREATE a new word with runtime => ((CONSTANT))
 * so that the value placed here is returned everytime
 * the constant's name is used in code. See => VALUE
 * for constant-like names that are expected to change
 * during execution of the program. In a ROM-able
 * forth the CONSTANT-value may get into a shared
 * ROM-area and is never copied to a RAM-address.
 */
p4_Runtime2 pf_constantRuntime;
FCode (pf_constant)
{
    p4_header_in();
    P4_NAMEFLAGS(LATEST) |= P4xISxRUNTIME;
    //FX_RUNTIME1 (pf_constant);
    FX_RCOMMA (pf_constantRuntime.exec[0]);
    FX_VCOMMA (*SP++);
}
P4RUNTIMES1_(pf_constant, pf_constant_RT, 0,pf_constant_RT_SEE);

static P4_CODE_RUN(pf_value_RT_SEE)
{
    strcat (p, p4_str_dot (*P4_TO_BODY (xt), p+200, BASE));
    strcat (p, "VALUE ");
    strncat (p, (char*) NAMEPTR(nfa), NAMELEN(nfa));
    return 0;
}

/** "((VALUE))" ( -- value ) [HIDDEN]
 * runtime compiled by => VALUE
 */
FCode_RT (pf_value_RT)
{
    *--SP = (p4cell) WP_PFA[0];
}

/** VALUE ( value 'name' -- ) [HIDDEN] [DOES: -- value ]
 * => CREATE a word and initialize it with value. Using it
 * later will push the value back onto the stack. Compare with
 * => VARIABLE and => CONSTANT - look also for => LOCALS| and
 * => VAR
 */
p4_Runtime2 pf_valueRuntime;
FCode (pf_value)
{
    p4_header_in();
    P4_NAMEFLAGS(LATEST) |= P4xISxRUNTIME;
    //FX_RUNTIME1 (pf_value);
    FX_RCOMMA (pf_valueRuntime.exec[0]);
    FX_VCOMMA (*SP++);
}
P4RUNTIMES1_ (pf_value, pf_value_RT, 0,pf_value_RT_SEE);

/** "((VAR))" ( -- pfa ) [HIDDEN]
 * the runtime compiled by => VARIABLE
 */
FCode_RT (pf_variable_RT)
{
    *--SP = (p4cell) WP_PFA;
}

/** VARIABLE ( 'name' -- ) [ANS] [DOES: -- name* ]
 * => CREATE a new variable, so that everytime the variable is
 * name, the address is returned for using with => @ and => !
 * - be aware that in FIG-forth VARIABLE did take an argument
 * being the initial value. ANSI-forth does different here.
 */
p4_Runtime2 pf_variableRuntime;
FCode (pf_variable)
{
    p4_header_in();
    P4_NAMEFLAGS(LATEST) |= P4xISxRUNTIME;
    //FX_RUNTIME1(pf_variable);
    FX_RCOMMA (pf_variableRuntime.exec[0]);
    FX_VCOMMA (0);
}
P4RUNTIME1(pf_variable, pf_variable_RT);
/* -------------------------------------------------------------- */
/** "((LIT))" ( -- value ) [HIDDEN]
 * execution compiled by => LITERAL
 */
FCode_XE (pf_literal_execution)
{
    *--SP = *((*(p4cell **)&(IP))++);
}

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
FCode (pf_literal)
{
    if (STATE)
    {
        FX_COMPILE (pf_literal);
        FX_SCOMMA (*SP++);
    }
}
P4COMPILES (pf_literal, pf_literal_execution, P4_SKIPS_CELL, P4_DEFAULT_STYLE);

/* -------------------------------------------------------------- */
/** [ ( -- ) [ANS]
 * leave compiling mode - often used inside of a colon-definition
 * to make fetch some very constant value and place it into the
 * currently compiled colon-defintion with => , or => LITERAL
 * - the corresponding unleave word is => ]
 */
FCode (pf_left_bracket)
{
    FX (pf_Q_comp);
    STATE = P4_FALSE;
}

/** ] ( -- ) [ANS]
 * enter compiling mode - often used inside of a colon-definition
 * to end a previous => [ - you may find a  => , or => LITERAL
 * nearby in example texts.
 */
FCode (pf_right_bracket)
{
    STATE = P4_TRUE;
}

/** ['] ( [name] -- name-xt* ) [ANS]
 * will place the execution token of the following word into
 * the dictionary. See => ' for non-compiling variant.
 */
FCode (pf_bracket_tick)
{
    FX (pf_tick);
    if (STATE)
    {
        FX_COMPILE (pf_bracket_tick);
        FX (pf_comma);
    }
}
P4COMPILES (pf_bracket_tick, pf_literal_execution, P4_SKIPS_NOTHING, P4_DEFAULT_STYLE);

/** [CHAR] ( [word] -- char# ) [ANS]
 * in compile-mode, get the (ascii-)value of the first charachter
 * in the following word and compile it as a literal so that it
 * will pop up on execution again. See => CHAR and forth-83 => ASCII
 */
FCode (pf_bracket_char)
{
    FX (pf_char);
    if (STATE)
    {
        FX_COMPILE (pf_bracket_char);
        FX (pf_comma);
    }
}
P4COMPILES (pf_bracket_char, pf_literal_execution, P4_SKIPS_CELL, P4_DEFAULT_STYLE);

/** ] ( -- ) [ANS]
 * enter compiling mode - often used inside of a colon-definition
 * to end a previous => [ - you may find a  => , or => LITERAL
 * nearby in example texts.
 */
FCode (p4_right_bracket)
{
    STATE = P4_TRUE;
}

/** "COMPILE," ( some-xt* -- ) [ANS]
 * place the execution-token on stack into the dictionary - in
 * traditional forth this is not even the least different than
 * a simple => , but in call-threaded code there's a big
 * difference - so COMPILE, is the portable one. Unlike
 * => COMPILE , => [COMPILE] and => POSTPONE this word does
 * not need the xt to have actually a name, see => :NONAME
 */
FCode (pf_compile_comma)
{
    FX_XCOMMA ((p4xt)( *SP++ ));
}

/** [COMPILE] ( [word] -- ) [ANS]
 * while compiling the next word will be place in the currently
 * defined word no matter if that word is immediate (like => IF )
 * - compare with => COMPILE and => POSTPONE
 */
FCode (pf_bracket_compile)
{
    FX (pf_Q_comp);
    FX (pf_tick);
    FX (pf_compile_comma);
}

/* -------------------------------------------------------------- */
#define	FX_BRANCH	(IP = (p4xt*)*IP)
#define FX_SKIP_BRANCH  (IP++)

/** "(?BRANCH)" ( -- ) [HIDDEN]
 * execution word compiled by => IF - just some simple => ?BRANCH
 */
FCode_XE (pf_q_branch_execution)
{
    if (!*SP++)
        FX_BRANCH;
    else
        FX_SKIP_BRANCH;
}

/** "(BRANCH)" ( -- ) [HIDDEN]
 * execution compiled by => ELSE - just a simple => BRANCH
 */
FCode_XE (pf_branch_execution)
{
    FX_BRANCH;
}

/** IF ( value -- ) [ANS]
 * checks the value on the stack (at run-time, not compile-time)
 * and if true executes the code-piece between => IF and the next
 * => ELSE or => THEN . Otherwise it has compiled a branch over
 * to be executed if the value on stack had been null at run-time.
 */
FCode (pf_if)
{
    FX_COMPILE (pf_if);
    FX (pf_forward_mark);
    *--SP = (p4cell) P4_ORIG_MAGIC;
}
P4COMPILES (pf_if, pf_q_branch_execution, P4_SKIPS_OFFSET, P4_IF_STYLE);

/** ELSE ( -- )
 * will compile an => ((ELSE)) => BRANCH that performs an
 * unconditional jump to the next => THEN - and it resolves
 * an => IF for the non-true case
 */
FCode (pf_else)
{
    pf_Q_pairs (P4_ORIG_MAGIC);
    FX_COMPILE (pf_else);
    FX (pf_forward_mark);
    *--SP = (p4cell) P4_ORIG_MAGIC;
    FX (p4_rot) ;
    FX (pf_forward_resolve) ;
}
P4COMPILES (pf_else, pf_branch_execution, P4_SKIPS_OFFSET, P4_ELSE_STYLE);

/** THEN ( -- ) [ANS]
 * does resolve a branch coming from either => IF or => ELSE
 */
FCode (pf_then)
{
    FX_COMPILE (pf_then);
    pf_Q_pairs (P4_ORIG_MAGIC);
    FX (pf_forward_resolve);
}
P4COMPILES (pf_then, pf_noop, P4_SKIPS_NOTHING, P4_THEN_STYLE);

/* -------------------------------------------------------------- */
/** BEGIN ( -- ) [ANS] [LOOP]
 * start a control-loop, see => WHILE and => REPEAT
 */
FCode (pf_begin)
{
    FX_COMPILE (pf_begin);
    FX (pf_backward_mark);
    *--SP = P4_DEST_MAGIC;
}
P4COMPILES (pf_begin, pf_noop, P4_SKIPS_NOTHING, P4_BEGIN_STYLE);

/** UNTIL ( test-flag -- ) [ANS] [REPEAT]
 * ends an control-loop, see => BEGIN and compare with => WHILE
 */
FCode (pf_until)
{
    pf_Q_pairs (P4_DEST_MAGIC);
    FX_COMPILE (pf_until);
    FX (pf_backward_resolve);
}
P4COMPILES (pf_until, pf_q_branch_execution, P4_SKIPS_OFFSET, P4_UNTIL_STYLE);

/** WHILE ( test-flag -- ) [ANS]
 * middle part of a => BEGIN .. => WHILE .. => REPEAT
 * control-loop - if cond is true the code-piece up to => REPEAT
 * is executed which will then jump back to => BEGIN - and if
 * the cond is null then => WHILE will branch to right after
 * the => REPEAT
 * (compare with => UNTIL that forms a => BEGIN .. => UNTIL loop)
 */
FCode (p4_two_swap)
{
    p4cell h;

    h = SP[0];
    SP[0] = SP[2];
    SP[2] = h;
    h = SP[1];
    SP[1] = SP[3];
    SP[3] = h;
}


FCode (pf_while)
{
    pf_Q_pairs (P4_DEST_MAGIC);
    *--SP = P4_DEST_MAGIC;
    FX_COMPILE (pf_while);
    FX (pf_forward_mark);
    *--SP = (p4cell) P4_ORIG_MAGIC;
    FX (p4_two_swap);
}
P4COMPILES (pf_while, pf_q_branch_execution, P4_SKIPS_OFFSET, P4_WHILE_STYLE);

/** REPEAT ( -- ) [ANS] [REPEAT]
 * ends an unconditional loop, see => BEGIN
 */
FCode (pf_repeat)
{
    pf_Q_pairs (P4_DEST_MAGIC);
    FX_COMPILE (pf_repeat);
    FX (pf_backward_resolve);
    pf_Q_pairs (P4_ORIG_MAGIC);
    FX (pf_forward_resolve);
}
P4COMPILES (pf_repeat, pf_branch_execution, P4_SKIPS_OFFSET, P4_REPEAT_STYLE);

/** AGAIN ( -- ) [ANS] [REPEAT]
 * ends an infinite loop, see => BEGIN and compare with
 * => WHILE
 */
FCode (pf_again)
{
    pf_Q_pairs (P4_DEST_MAGIC);
    FX_COMPILE (pf_again);
    FX (pf_backward_resolve);
}
P4COMPILES (pf_again, pf_branch_execution, P4_SKIPS_OFFSET, P4_AGAIN_STYLE);


/* implementation detail:
 * DO will compile (DO) and forward-address to LOOP
 * (DO) will set RP[2] to its point after that forward-adress
 * LOOP can just jump to RP[2]
 * LEAVE can jump via RP[2][-1] forward-address
 */

/** "((DO))" ( end# start# -- ) [HIDDEN]
 * compiled by => DO
 */
FCode (pf_do_execution)
{
    RP -= 3;                     /* push onto return-stack: */
    RP[2] = ++IP;                /* IP to BRANCH back to just after DO */
    RP[1] = (p4xt *) SP[1];   /* upper limit */
    RP[0] = (p4xt *) SP[0];   /* lower limit and counter */
    SP +=2;
}

/** DO ( end# start# | end* start* -- R: some,loop ) [ANS] [LOOP]
 *  pushes $end and $start onto the return-stack ( => >R )
 *  and starts a control-loop that ends with => LOOP or
 *  => +LOOP and may get a break-out with => LEAVE . The
 *  loop-variable can be accessed with => I
 */
FCode (pf_do)
{
    FX_COMPILE (pf_do);
    FX (pf_forward_mark);
    *--SP = (p4cell) P4_LOOP_MAGIC;
}
P4COMPILES (pf_do, pf_do_execution, P4_SKIPS_OFFSET, P4_DO_STYLE);

/** LEAVE ( R: some,loop -- R: some,loop ) [ANS]
 * quit the innermost => DO .. => LOOP  - it does even
 * clean the return-stack and branches to the place directly
 * after the next => LOOP
 */
FCode_XE (pf_leave_execution)
{
    IP = RP[2] - 1; /* the place after the next LOOP */
    RP += 3;        /* UNLOOP */
    FX_BRANCH;
}

FCode (pf_leave)
{
    FX_COMPILE (pf_leave);
//    RP += 3;        /* terminate loop */
}
P4COMPILES (pf_leave, pf_leave_execution, P4_SKIPS_NOTHING, P4_DEFAULT_STYLE);
/** "((LOOP))" ( -- ) [HIDDEN]
 * execution compiled by => LOOP
 */
FCode (pf_loop_execution)
{
    RP[0] = (p4xt *)((p4cell)(*RP) + 1);
    if (RP[0]<RP[1])	/* counter < upper limit ? */
    {
        IP = RP[2];     /* if yes: loop back (BRANCH) */
    } else
    {
        RP += 3;        /* if no: terminate loop */
    }
}

/** LOOP ( R: some,loop -- ) [ANS] [REPEAT]
 * resolves a previous => DO thereby compiling => ((LOOP)) which
 * does increment/decrement the index-value and branch back if
 * the end-value of the loop has not been reached.
 */
FCode (pf_loop)
{
    pf_Q_pairs (P4_LOOP_MAGIC);
    FX_COMPILE (pf_loop);
    FX (pf_forward_resolve);
}
P4COMPILES (pf_loop, pf_loop_execution, P4_SKIPS_OFFSET, P4_LOOP_STYLE);

/** "((+LOOP))" ( increment# -- ) [HIDDEN]
 * compiled by => +LOOP
 */
FCode (pf_plus_loop_execution)
{
    p4cell i = *SP++;
    RP[0] = (p4xt *)((p4cell)(*RP) + i);
    if (RP[0]<RP[1])	/* counter < upper limit ? */
    {
        IP = RP[2];     /* if yes: loop back (BRANCH) */
    } else
    {
        RP += 3;        /* if no: terminate loop */
    }
}

/** +LOOP ( increment# R: some,loop -- ) [ANS]
 * compile => ((+LOOP)) which will use the increment
 * as the loop-offset instead of just 1. See the
 * => DO and => LOOP construct.
 */
FCode (pf_plus_loop)
{
    pf_Q_pairs (P4_LOOP_MAGIC);
    FX_COMPILE (pf_plus_loop);
    FX (pf_forward_resolve);
}
P4COMPILES (pf_plus_loop, pf_plus_loop_execution, P4_SKIPS_NOTHING, P4_LOOP_STYLE);

/** UNLOOP ( R: some,loop -- ) [ANS]
 * drop the => DO .. => LOOP runtime variables from the return-stack,
 * usually used just in before an => EXIT call. Using this multiple
 * times can unnest multiple nested loops.
 */
FCode_XE (pf_unloop_execution)
{
    RP += 3;        /* terminate loop */
}

FCode (pf_unloop)
{
    FX_COMPILE (pf_unloop);
}
P4COMPILES (pf_unloop, pf_unloop_execution, P4_SKIPS_NOTHING, P4_DEFAULT_STYLE);

/** I ( R: some,loop -- S: i# ) [ANS]
 * returns the index-value of the innermost => DO .. => LOOP
 */
FCode (pf_i)
{
    FX_COMPILE (pf_i);
}

FCode (pf_i_execution)
{
    *--SP = (p4cell)RP[0];
}
P4COMPILES (pf_i, pf_i_execution, P4_SKIPS_NOTHING, P4_DEFAULT_STYLE);

/** J ( R: some,loop -- S: j# ) [ANS]
 * get the current => DO ... => LOOP index-value being
 * the not-innnermost. (the second-innermost...)
 * see also for the other loop-index-values at
 * => I and => K
 */
FCode (pf_j)
{
    FX_COMPILE (pf_j);
}

FCode (pf_j_execution)
{
    *--SP = (p4cell)RP[3];
}
P4COMPILES (pf_j, pf_j_execution, P4_SKIPS_NOTHING, P4_DEFAULT_STYLE);

FCode (pf_k)
{
    FX_COMPILE (pf_k);
}

FCode (pf_k_execution)
{
    *--SP = (p4cell)RP[6];
}
P4COMPILES (pf_k, pf_k_execution, P4_SKIPS_NOTHING, P4_DEFAULT_STYLE);

/*
   : X ( n -- )
       CASE
       test1 OF ( -- ) ... ENDOF
       testn OF ( -- ) ... ENDOF
       ( n -- ) ... ( default )
       ENDCASE ...
   ;
*/
/** CASE ( value -- value ) [ANS]
 * start a CASE construct that ends at => ENDCASE
 * and compares the value on stack at each => OF place
 */
FCode (pf_case)
{
    FX_COMPILE (pf_case);
    *--SP = (p4cell) CSP;
    CSP = SP;
    *--SP = (p4cell) P4_CASE_MAGIC;
}
P4COMPILES (pf_case, pf_noop, P4_SKIPS_NOTHING, P4_CASE_STYLE);

/** ENDCASE ( value -- ) [ANS]
 * ends a => CASE construct that may surround multiple sections of
 * => OF ... => ENDOF code-portions. The => ENDCASE has to resolve the
 * branches that are necessary at each => ENDOF to point to right after
 * => ENDCASE
 */
FCode (pf_endcase)
{
    pf_Q_pairs (P4_CASE_MAGIC);
    FX_COMPILE (pf_endcase);
    while (SP < CSP)
        FX (pf_forward_resolve);
    CSP = (p4cell *) *SP++;
}
P4COMPILES (pf_endcase, p4_drop, P4_SKIPS_NOTHING, P4_ENDCASE_STYLE);

/** "((OF))" ( check val -- check ) [HIDDEN]
 * execution compiled by => OF
 */
FCode_XE (pf_of_execution)
{
    if (SP[0] != SP[1])          /* tos equals second? */
    { SP += 1; FX_BRANCH; }      /* no: drop top, branch */
    else
    { SP += 2; FX_SKIP_BRANCH; } /* yes: drop both, don't branch */
}

/** OF ( value test -- value ) [ANS]
 * compare the case-value placed lately with the comp-value
 * being available since => CASE - if they are equal run the
 * following code-portion up to => ENDOF after which the
 * case-construct ends at the next => ENDCASE
 */
FCode (pf_of)
{
    pf_Q_pairs (P4_CASE_MAGIC);
    FX_COMPILE (pf_of);
    FX (pf_forward_mark);
    *--SP = (p4cell) P4_OF_MAGIC;
}
P4COMPILES (pf_of, pf_of_execution, P4_SKIPS_OFFSET, P4_OF_STYLE);

/** ENDOF ( -- ) [ANS]
 * resolve the branch need at the previous => OF to mark
 * a code-piece and leave with an unconditional branch
 * at the next => ENDCASE (opened by => CASE )
 */
FCode (pf_endof)
{
    pf_Q_pairs (P4_OF_MAGIC);
    FX_COMPILE (pf_endof);
    FX (pf_forward_mark);
    FX (p4_swap);
    FX (pf_forward_resolve);
    *--SP = (p4cell) P4_CASE_MAGIC;
}
P4COMPILES (pf_endof, pf_branch_execution, P4_SKIPS_OFFSET, P4_ENDOF_STYLE);

/* -------------------------------------------------------------- */
/*
STATE  
<BUILDS CREATE  DOES>
:  ;
IMMEDIATE
, C,  ALLOT
CONSTANT  
VARIABLE
LITERAL  
[']  [COMPILE]  ]

IF ELSE THEN
BEGIN UNTIL WHILE REPEAT AGAIN
DO I J LEAVE LOOP
VOCABULARY

not implemented
+LOOP
COMPILE

not here
."
ABORT"

 */

P4_LISTWORDS (compiler) =
{
    /* state checks */
    P4_FXco ("?COMP",        pf_Q_comp),
    P4_FXco ("?EXEC",        pf_Q_exec),
    P4_FXco ("?PAIRS",       pf_Q_pairs),
    P4_FXco ("?STACK",       pf_Q_stack),
    /* return stack */
    P4_SXco (">R",           pf_to_r),
    P4_SXco ("R>",           pf_r_from),
    P4_SXco ("R@",           pf_r_fetch),
    /* definition checks */
    //P4_DVaR ("STATE",        state),
    P4_DVAR ("STATE",        state),
    P4_RTco ("<BUILDS",      pf_builds),
    P4_RTco ("CREATE",       pf_builds), // CREATE and <BUILDS are synonyms
    P4_SXco ("DOES>",        pf_does),
    P4_RTco (":",            pf_colon),
    P4_SXco (";",            pf_semicolon),
    P4_SXco ("EXIT",         pf_exit),
    P4_FXco ("IMMEDIATE",    pf_immediate),
    P4_FXco (",",            pf_comma),
    P4_FXco ("C,",           pf_c_comma),
    P4_FXco ("ALIGN",        pf_align),
    P4_FXco ("ALIGNED",      pf_aligned),
    P4_FXco ("ALLOT",        pf_allot),
    P4_RTco ("CONSTANT",     pf_constant),
    P4_RTco ("VALUE",        pf_value),
    P4_RTco ("VARIABLE",     pf_variable),

    P4_SXco ("LITERAL",      pf_literal),
    P4_IXco ("[",            pf_left_bracket),
    P4_FXco ("]",            pf_right_bracket),
    P4_SXco ("[']",          pf_bracket_tick),
    P4_SXco ("[CHAR]",       pf_bracket_char),
    P4_FXco ("COMPILE,",     pf_compile_comma),
    P4_IXco ("[COMPILE]",    pf_bracket_compile),
/*
    P4_IXco ("BRANCH",		pf_branch),
    P4_IXco ("?BRANCH",		pf_q_branch),
*/
    P4_SXco ("IF",           pf_if),
    P4_SXco ("ELSE",         pf_else),
    P4_SXco ("THEN",         pf_then),
    P4_SXco ("BEGIN",        pf_begin),
    P4_SXco ("UNTIL",        pf_until),
    P4_SXco ("WHILE",        pf_while),
    P4_SXco ("REPEAT",       pf_repeat),
    P4_SXco ("AGAIN",        pf_again),

    P4_SXco ("DO",           pf_do),
    P4_SXco ("LEAVE",        pf_leave),
    P4_SXco ("LOOP",         pf_loop),
    P4_SXco ("+LOOP",        pf_plus_loop),
    P4_SXco ("UNLOOP",       pf_unloop),
    P4_SXco ("I",            pf_i),
    P4_SXco ("J",            pf_j),
    P4_SXco ("K",            pf_k),

    P4_SXco ("CASE",         pf_case),
    P4_SXco ("OF",           pf_of),
    P4_SXco ("ENDOF",        pf_endof),
    P4_SXco ("ENDCASE",      pf_endcase),

};
P4_COUNTWORDS (compiler, "Compiler words");
