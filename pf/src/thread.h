#ifndef __PF_THREAD_H
#define __PF_THREAD_H

/* this is called options structure, since this aggregate communicates
   the command-line. But it has enhanced over time. Just expect it to
   be THE data-structure unique to each thread (started via main).
   You can actually have MP_MAX forth-processors per thread, so if
   your OS isn`t multi-threaded, what the heck, use this feature!
*/

typedef struct p4_Thread	p4_Thread;

struct p4_Thread
{
    char* dp;			/* actual top of the dictionary */

    p4cell state;		/* interpreting (0) or compiling (-1) */
    p4ucell base;		/* of number i/o conversion */

    p4cell* stack;		/*  data stack */
    p4cell* s0;
    p4cell* sp;			/* the stack pointer */

    p4xt**  rstack;		/*  return stack */
    p4xt**  r0;
    p4xt**  rp;			/* the return stack pointer */

#if defined PF_WITH_FLOATING
    double* fstack;		/*  floating point stack */
    double* f0;
    double* fp;			/* the floating point stack pointer */
    p4cell precision;		/* floating point output precision */
#endif
    p4xt*   ip;			/* the intruction pointer */
    p4xt    wp;			/* speed up the inner interpreter */

    void (*execute)(p4xt);	/* := normal_execute */

    struct {
	const p4_char_t* ptr;
	unsigned len;          /* p4ucell is 8byte on x86_64 but */
    } word;                    /* parsing is not exceeding 16bit anyway */

};

struct p4_Thread* p4TH;

char* dict;
char* dictlimit;


#define PFE (*p4TH)

#define DP (PFE.dp)

#define S0 PFE.s0
#define SP (PFE.sp)
#define R0 PFE.r0
#define RP (PFE.rp)

#if defined PF_WITH_FLOATING
  #define F0 PFE.f0
  #define FP (PFE.fp)
  #define PRECISION (PFE.precision)
#endif

#define IP (PFE.ip)
#define WP (PFE.wp)

#define STATE (PFE.state)
#define BASE  (PFE.base)

#endif
