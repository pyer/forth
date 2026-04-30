#ifndef __PF_COMPILER_H
#define __PF_COMPILER_H



extern p4cell *csp;        /* compiler security, saves sp here */
#define CSP        (csp)

extern p4cell STATE;

struct p4_Runtime2              /* describes characteristics of CFA code */
{
    long magic;                 /* mark begin of structure */
    char const *name;           /* the header name for it */
    p4code comp;             /* the word that will CREATE new headers */
    p4code exec[2];          /* and the values contained in created CFAs */
};

typedef struct p4_Runtime2 p4_Runtime2; /* and also for the CFA themselves */

#define P4RUNTIME1(C,E1)            \
p4_Runtime2 C##_Runtime =           \
{ P4_RUNTIME_MAGIC, 0,              \
  C##_, { E1##_, NULL }   \
}

#define FX_RUNTIME1(X) do { extern p4_Runtime2 X##_Runtime;  \
                            FX_RCOMMA (X##_Runtime.exec[0]); } while(0)

struct p4_Semant		/* for words with different compilation */
{				            /* and execution semantics: */
    long magic;			/* mark begin of structure */
    p4char const *name;	      /* compiled by */
    p4code comp;		          /* compilation/interpretation semantics */
    p4code exec[1];		        /* execution semantics */
    int skips;                /* to decompile the data following xt */
};

typedef struct p4_Semant p4_Semant; /* pointer set for state smart words */

#define  P4_SKIPS_NOTHING           0
#define  P4_SKIPS_OFFSET            1
#define  P4_SKIPS_CELL              2
#define  P4_SKIPS_STRING            3
#define  P4_SKIPS_TO_TOKEN          4
#define  P4_SKIPS_FLOAT             8

#define P4COMPILE(C,E,S)            \
p4_Semant C##_Semant =              \
{                                   \
  P4_SEMANT_MAGIC,                  \
  NULL,                             \
  C##_,                             \
  { E##_ },                         \
  S                                 \
}

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

FCode (pf_create_RT);
FCode (pf_create);
FCode (pf_builds_RT);
FCode (pf_builds);
FCode (pf_defer_RT);
FCode (pf_defer);
FCode (pf_does_RT);
FCode (pf_does);

FCode (pf_colon_RT);
FCode (pf_colon);

FCode (pf_semicolon_execution);
FCode (pf_does_execution);
FCode (pf_literal_execution);

FCode (pf_constant_RT);
FCode (pf_constant);
FCode (pf_variable_RT);
FCode (pf_variable);

#endif
