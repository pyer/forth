#ifndef __INTERPRET_H
#define __INTERPRET_H

#define TO_IN		PFE.to_in
#define PAD		((p4_char_t *)PFE.dp + MIN_HOLD)
#define HLD		(PFE.hld)

void p4_lower (p4_char_t *p, int n);
void p4_upper (p4_char_t *p, int n);

#define UPPERMAX 32
#define UPPERCOPY(_upper_, _nm_, _l_) \
        if ((_l_) < UPPERMAX) {       \
          memcpy ((_upper_), (_nm_), (_l_)); \
          p4_upper ((_upper_), (_l_)); \
        } else { \
          *(int*)(_upper_) = 0; \
        }

void pf_call (p4xt xt);
void pf_normal_execute (p4xt xt);

p4char * pf_tick_nfa (void);
p4xt pf_tick_cfa (void);

void pf_skip_spaces(void);
void pf_parse_word( char delimiter );
FCode (pf_parse_comma_quote);

p4char* p4_header_comma (const p4char *name, int len, p4_Wordl *wid);
p4char* p4_header_in (p4_Wordl* wid);
char* pf_word ( char );

FCode (pf_tick);
FCode (pf_reveal);

void p4_interpret(void);
void pf_interpret( char *buf, int len );
void pf_include(const char *name, int len);
#endif
