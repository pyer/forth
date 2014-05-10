#ifndef __PF_COMPILER_H
#define __PF_COMPILER_H

p4cell *csp;		/* compiler security, saves sp here */
#define CSP		(csp)

#define FX_COMPILE_COMMA_XE(X) FX_ZCOMMA(X)
#define FX_COMPILE_COMMA_RT(X) FX_ZCOMMA(X)

#define FX_RUNTIME1_RT(X) FX_XCOMMA(X##_RT_) /* a simply comma */

#define P4_XT_VALUE(__xt) (*(__xt))
#define FX_GET_RT(__rt)   __rt##_RT_

#define P4SEMANTICS(X) X##_Semant
#define P4RUNTIME_(X)  X##Runtime

/* compile execution semantics from within C-code: */
#define FX_DEF_COMPILE1(X) p4_Seman2  P4SEMANTICS(X)
#define FX_DEF_COMPILE2(X) p4_Seman2  P4SEMANTICS(X)
#define FX_DEF_COMPILES(X) p4_Semant  P4SEMANTICS(X)

#define FX_GET_COMPILE1(X) (&P4SEMANTICS(X).exec[0])
#define FX_GET_COMPILE2(X) (&P4SEMANTICS(X).exec[1])

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

#define P4COMPILES2(C,E1,E2,S,STYLE)		\
p4_Seman2 P4SEMANTICS(C) =			\
{						\
  P4_SEMANT_MAGIC,				\
  { S, STYLE },					\
  NULL,						\
  P4CODE (C),					\
  { P4CODE (E1), P4CODE (E2) }			\
}

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
  SEE                            \
}

#define P4RUNTIMES2_(C,E1,E2,FLAGS,SEE)         \
p4_Runtime2 P4RUNTIME_(C) =                     \
{ (const char*) "@",                          \
  P4_RUNTIME_MAGIC, FLAGS, 0,                   \
  P4CODE(C), { P4CODE(E1), P4CODE(E2) },        \
  SEE                            \
}
//  { SEE, NULL, NULL },

#define P4RUNTIMES1(C,E,FLAGS) P4RUNTIMES1_(C,E,FLAGS,0)
#define P4RUNTIMES2(C,E1,E2,FLAGS) P4RUNTIMES1_(C,E1,E2,FLAGS,0)

#define P4RUNTIME1(C,E1)    P4RUNTIMES1(C,E1,0)


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
