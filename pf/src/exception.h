#ifndef _PF_EXCEPTION_H
#define _PF_EXCEPTION_H

extern jmp_buf jump_loop;

#define pf_longjmp_abort() (longjmp(jump_loop, 'A'))
#define pf_longjmp_exit()  (longjmp(jump_loop, 'X'))

/**
 * the THROW impl
 */
void p4_throwstr (int id, const char* description);
void p4_throw (int id);

#endif
