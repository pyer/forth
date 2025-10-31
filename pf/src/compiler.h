#ifndef __PF_COMPILER_H
#define __PF_COMPILER_H

extern p4cell *csp;        /* compiler security, saves sp here */
#define CSP        (csp)

struct p4_Runtime2              /* describes characteristics of CFA code */
{
    long magic;                 /* mark begin of structure */
    p4cell flag;                /* the call-threading flags for the exec[]s */
    char const *name;           /* the header name for it */
    p4code comp;             /* the word that will CREATE new headers */
    p4code exec[2];          /* and the values contained in created CFAs */
};

typedef struct p4_Runtime2 p4_Runtime2; /* and also for the CFA themselves */

#define P4RUNTIME1(C,E1)            \
p4_Runtime2 C##_Runtime =            \
{ P4_RUNTIME_MAGIC, 0, 0,           \
  P4CODE(C), { P4CODE(E1), NULL },  \
}

#define FX_RUNTIME1(X) do { extern p4_Runtime2 X##_Runtime;  \
                            FX_RCOMMA (X##_Runtime.exec[0]); } while(0)

struct p4_Semant		/* for words with different compilation */
{				            /* and execution semantics: */
    long magic;			/* mark begin of structure */
//    P4_CODE_SEE((*skips));    /* to decompile the data following xt */
    p4char const *name;	      /* compiled by */
    p4code comp;		          /* compilation/interpretation semantics */
    p4code exec[1];		        /* execution semantics */
};

typedef struct p4_Semant p4_Semant; /* pointer set for state smart words */

/*
#define P4_CODE_SEE(func) p4xt* func (p4xt* ip, char* p, p4_Semant* s)

*/

#define  P4_SKIPS_NOTHING           0
#define  P4_SKIPS_OFFSET            1
#define  P4_SKIPS_CELL              2
#define  P4_SKIPS_DCELL             3
#define  P4_SKIPS_STRING            5
#define  P4_SKIPS_2STRINGS          6
#define  P4_SKIPS_TO_TOKEN          7

#define P4COMPILE(C,E,S)            \
p4_Semant C##_Semant =              \
{                                   \
  P4_SEMANT_MAGIC,                  \
  NULL,                             \
  P4CODE (C),                       \
  { P4CODE (E) }                    \
}

/*
#define P4COMPILE(C,E,S)            \
p4_Semant C##_Semant =              \
{                                   \
  P4_SEMANT_MAGIC,                  \
  S,                                \
  NULL,                             \
  P4CODE (C),                       \
  { P4CODE (E) }                    \
}
*/

/* compile execution semantics from within C-code: */
#define FX_COMPILE(X)  do { extern  p4_Semant X##_Semant;  \
                            FX_ZCOMMA (&X##_Semant.exec[0]); } while(0)

/* 
 * -- compiler definitions 
 *
 */
FCode( pf_noop );

char * p4_str_dot (p4cell n, char *p, int base);

p4cell pf_aligned (p4cell n);
FCode (pf_align);

/** ?STACK ( -- )
 * check all stacks for underflow and overflow conditions,
 * and if such an error condition is detected => THROW
 */
FCode (pf_Q_stack);

/* FORTH-83 style system extension words */
FCode (pf_backward_mark);
FCode (pf_backward_resolve);
FCode (pf_forward_mark);
FCode (pf_forward_resolve);
FCode (pf_bracket_compile);

FCode_RT (pf_create_RT);
FCode  (pf_create);
FCode_RT (pf_builds_RT);
FCode  (pf_builds);
FCode_RT (pf_does_RT);
FCode  (pf_does);

FCode_RT (pf_colon_RT);
FCode  (pf_colon);
FCode  (pf_semicolon_execution);

FCode_RT (pf_constant_RT);
FCode  (pf_constant);
FCode_RT (pf_value_RT);
FCode  (pf_value);
FCode_RT (pf_variable_RT);
FCode  (pf_variable);

FCode_XE (pf_literal_execution);

  #if defined PF_WITH_FLOATING
    FCode_RT (p4_f_variable_RT);
    FCode_RT (p4_f_constant_RT);
  #endif

#endif
