#ifndef __INTERPRET_H
#define __INTERPRET_H

// display a message when a word is redefined
int redefined_msg;
void show_word(void);

p4_namebuf_t *LATEST;

/* you must differentiate between VAR-style body and DOES-style body */
#define CFA_TO_LINK(C)   ((p4char**)(C) - 1 )
#define LINK_TO_CFA(C)   ((p4xt)(C) + 1 )

#define P4_TO_BODY(C)     ((p4cell *)((p4xt)(C) + 1))
#define P4_BODY_FROM(P)   ((p4xt)((p4cell *)(P) - 1))
#define P4_TO_DOES_BODY(C)  ((p4cell *)((p4xt)(C) + 2))
#define P4_TO_DOES_CODE(C)  ((p4xt **)((p4xt)(C) + 1))

p4char** name_to_link (const p4char* p);
p4xt name_to_cfa (const p4char *p);
p4char * cfa_to_name (p4xt xt);
p4cell * cfa_to_body (p4xt xt);

void pf_load_words (const p4Words* ws);
p4char* p4_header_comma (const p4char *name, int len);
p4char* p4_header_in (void);

char pf_category (p4code p);

void pf_call (p4xt xt);
void pf_normal_execute (p4xt xt);

p4char * pf_tick_nfa (void);
p4xt pf_tick_cfa (void);

void pf_skip_spaces(void);
void pf_parse_word( char delimiter );
FCode (pf_parse_comma_quote);

void pf_dot_name (const p4_namebuf_t *nfa);

char* pf_word ( char );
char* cap_word ( char );

FCode (pf_char);
FCode (pf_tick);
FCode (pf_reveal);

void pf_interpret( char *buf, int len, int n );
void pf_include(const char *name, int len);

#endif
