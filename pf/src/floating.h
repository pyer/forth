#ifndef __PF_FLOATING_H
#define __PF_FLOATING_H

extern p4fcell* fstack;
extern p4fcell* f0;
extern p4fcell* fp;
extern p4cell precision;

int pf_to_float (const char *p, p4cell n, p4fcell *r);

FCode (p4_f_constant_RT);
FCode (p4_f_variable_RT);

#endif
