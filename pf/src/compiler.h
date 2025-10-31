#ifndef __PF_COMPILER_H
#define __PF_COMPILER_H

extern p4cell *csp;        /* compiler security, saves sp here */
#define CSP        (csp)

#define P4RUNTIME1(C,E1)            \
p4_Runtime2 C##_Runtime =            \
{ P4_RUNTIME_MAGIC, 0, 0,           \
  P4CODE(C), { P4CODE(E1), NULL },  \
}

#define FX_RUNTIME1(X) do { extern p4_Runtime2 X##_Runtime;  \
                            FX_RCOMMA (X##_Runtime.exec[0]); } while(0)

#define P4COMPILE(C,E,S)            \
p4_Semant C##_Semant =              \
{                                   \
  P4_SEMANT_MAGIC,                  \
  S,                                \
  NULL,                             \
  P4CODE (C),                       \
  { P4CODE (E) }                    \
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
