#ifndef __PF_COMPILER_H
#define __PF_COMPILER_H

/* compile execution semantics from within C-code: */
#define FX_DEF_COMPILE1(X) p4_Seman2  P4SEMANTICS(X)
#define FX_DEF_COMPILE2(X) p4_Seman2  P4SEMANTICS(X)

# define FX_GET_COMPILE1(X) (&P4SEMANTICS(X).exec[0])
# define FX_GET_COMPILE2(X) (&P4SEMANTICS(X).exec[1])

#define FX_COMPILE_COMMA_XE(X) FX_ZCOMMA(X)
#define FX_COMPILE_COMMA_RT(X) FX_ZCOMMA(X)

#define P4_XT_VALUE(__xt) (*(__xt))
#define FX_GET_RT(__rt)   __rt##_RT_

struct p4_Runtime2              /* describes characteristics of CFA code */
{
    const char* type;         /* in place of p4_Word.name */
    long magic;                 /* mark begin of structure */
    p4cell flag;                /* the call-threading flags for the exec[]s */
    char const *name;           /* the header name for it */
    p4code comp;             /* the word that will CREATE new headers */
    p4code exec[2];          /* and the values contained in created CFAs */
    struct {
	P4_CODE_RUN((*see));    /* the decompiler routine */
	P4_CODE_RUN((*forget)); /* while running forget destroyers */
	P4_CODE_RUN((*atexit)); /* while running atexit destroyers */
    } run;                      /* we did not make an extra typedef for it */
};

typedef struct p4_Runtime2 p4_Runtime2; /* and also for the CFA themselves */
#define P4SEMANTICS(X) X##_Semant
#define P4RUNTIME_(X)  X##Runtime

#define FX_RUNTIME1_RT(X) FX_XCOMMA(X##_RT_) /* a simply comma */

#define FX_DEF_RUNTIME1(X) p4_Runtime2 P4RUNTIME_(X)
#define FX_GET_RUNTIME1(X) (P4RUNTIME_(X).exec[0])
#define FX_RUNTIME1(X) do { extern     FX_DEF_RUNTIME1(X);  \
                            FX_RCOMMA (FX_GET_RUNTIME1(X)); } while(0)

#define P4COMPILES(C,E,S,STYLE)			\
p4_Semant P4SEMANTICS(C) =			\
{						\
  P4_SEMANT_MAGIC,				\
  { S, STYLE },					\
  NULL,						\
  P4CODE (C),					\
  { P4CODE (E) }				\
}

#define FX_DEF_COMPILE1(X) p4_Seman2  P4SEMANTICS(X)
#define FX_DEF_COMPILE2(X) p4_Seman2  P4SEMANTICS(X)
#define FX_DEF_COMPILES(X) p4_Semant  P4SEMANTICS(X)

#define FX_GET_COMPILE1(X) (&P4SEMANTICS(X).exec[0])
#define FX_GET_COMPILE2(X) (&P4SEMANTICS(X).exec[1])
#define FX_COMPILE(X)  do { extern     FX_DEF_COMPILES(X);  \
                  FX_ZCOMMA (FX_GET_COMPILE1(X)); } while(0)
#define FX_COMPILE1(X) do { extern     FX_DEF_COMPILE1(X);  \
                  FX_ZCOMMA (FX_GET_COMPILE1(X)); } while(0)
#define FX_COMPILE2(X) do { extern     FX_DEF_COMPILE2(X);  \
                  FX_ZCOMMA (FX_GET_COMPILE2(X)); } while(0)

#define P4RUNTIME_(X)  X##Runtime

#define P4RUNTIMES1_(C,E1,FLAGS,SEE)            \
p4_Runtime2 P4RUNTIME_(C) =                     \
{ (const char*) "@",                          \
  P4_RUNTIME_MAGIC, FLAGS, 0,                   \
  P4CODE(C), { P4CODE(E1), NULL },              \
  { SEE, NULL, NULL }                           \
}

#define P4RUNTIMES2_(C,E1,E2,FLAGS,SEE)         \
p4_Runtime2 P4RUNTIME_(C) =                     \
{ (const char*) "@",                          \
  P4_RUNTIME_MAGIC, FLAGS, 0,                   \
  P4CODE(C), { P4CODE(E1), P4CODE(E2) },        \
  { SEE, NULL, NULL },                          \
}

#define P4RUNTIMES1(C,E,FLAGS) P4RUNTIMES1_(C,E,FLAGS,0)
#define P4RUNTIMES2(C,E1,E2,FLAGS) P4RUNTIMES1_(C,E1,E2,FLAGS,0)

#define P4RUNTIME1(C,E1)    P4RUNTIMES1(C,E1,0)


/* 
 * -- compiler definitions 
 *
 */

char * p4_str_dot (p4cell n, char *p, int base);

p4cell pf_aligned (p4cell n);
FCode (pf_align);

/** VLIST ( -- )
 *  The VLIST command had been present in FIG and other forth
 *  implementations. It has to list all accessible words. In PFE
 *  it list all words in the search order. Well, the point is,
 *  that we do really just look into the search order and are
 *  then calling => WORDS on that Wordl. That way you can see
 *  all accessible words in the order they might be found.
 *  Uses => ?CR
 */
FCode (p4_vlist);

/** !CSP ( -- )
 * put => SP into => CSP
 * <br> used in control-words
 */
FCode (pf_store_csp);

/** ?CSP ( -- )
 * check that => SP == => CSP otherwise => THROW
 * <br> used in control-words
 */
FCode (pf_Q_csp);

/** ?COMP ( -- )
 * check that the current => STATE is compiling
 * otherwise => THROW
 * <br> often used in control-words
 */
FCode (pf_Q_comp);

/** ?EXEC ( -- )
 * check that the current => STATE is executing
 * otherwise => THROW
 * <br> often used in control-words
 */
FCode (pf_Q_exec);

/** ?FILE ( file-id -- )
 * check the file-id otherwise (fixme)
 */
FCode (pf_Q_file);

/** ?LOADING ( -- )
 * check that the currently interpreted text is 
 * from a file/block, otherwise => THROW
 */
FCode (pf_Q_loading);

/** ?PAIRS ( a b -- )
 * if compiling, check that the two magics on
 * the => CS-STACK are identical, otherwise throw
 * <br> used in control-words
 */
void pf_Q_pairs (p4cell n);
FCode (pf_Q_pairs);

/** ?STACK ( -- )
 * check all stacks for underflow and overflow conditions,
 * and if such an error condition is detected => THROW
 */
FCode (pf_Q_stack);

/** (DICTLIMIT)   ( -- constvalue )
 * the upper limit of the forth writeable memory space,
 * the variable => DICTLIMIT must be below this line.
 * stack-space and other space-areas are often allocated
 * above => DICTLIMIT upto this constant.
 *
 * => DICTFENCE is the lower end of the writeable dictionary
 */
FCode(p4_paren_dictlimit);


/* FORTH-83 style system extension words */
FCode (pf_backward_mark);
FCode (pf_backward_resolve);
FCode (pf_forward_mark);
FCode (pf_forward_resolve);
FCode (pf_bracket_compile);

void pf_Q_pairs (p4cell n);
FCode (pf_Q_stack);

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

#endif
