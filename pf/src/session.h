#ifndef __PF_SESSION_H
#define __PF_SESSION_H

/* this is called options structure, since this aggregate communicates
   the command-line. But it has enhanced over time. Just expect it to
   be THE data-structure unique to each thread (started via main).
   You can actually have MP_MAX forth-processors per thread, so if
   your OS isn`t multi-threaded, what the heck, use this feature!
*/

typedef struct p4_Thread	p4_Thread;

struct p4_Thread
{
    p4_byte_t *dp;		/* actual top of the dictionary */

    p4_byte_t* dict;		/*  dictionary */
    p4_byte_t* dictlimit;
    p4cell* stack;		/*  data stack */
    p4cell* s0;
    double *fstack;		/*  floating point stack */
    double* f0;
    p4xt** rstack;		/*  return stack */
    p4xt**  r0;

    char* history;		/*  command line history buffer */
    char* history_top;

/* VM */
    p4xt*  ip; /* the intruction pointer */
    p4xt      wp;    /* speed up the inner interpreter */
    p4cell*   sp; /* the stack pointer */
    p4xt** rp; /* the return stack pointer */
    double*   fp; /* the floating point stack pointer */

/*Dict*/
    p4_namebuf_t *latest;	/* NFA of most recently CREATEd header */

//    p4_Except *catchframe;	/* links to chain of CATCHed words */

    p4cell state;		/* interpreting (0) or compiling (-1) */
    p4ucell base;		/* of number i/o conversion */
    p4cell precision;		/* floating point output precision */

    void (*execute)(p4xt);	/* := normal_execute */

/* core.c */
    p4code semicolon_code;      /* the code to run at next semicolon */

/* term*.c */
    int (*wait_for_stdin)(void);

    void (*on_stop) (void);     /* = p4_system_terminal; */
    void (*on_continue) (void); /* = p4_interactive_terminal; */
    void (*on_winchg) (void);   /* = p4_query_winsize; */
    void (*on_sigalrm) (void);  /* really from signal.c */


    struct {
	const p4_char_t* ptr;
	unsigned len;          /* p4ucell is 8byte on x86_64 but */
    } word;                    /* parsing is not exceeding 16bit anyway */

    double asinh_MAX_over_4;          /* see complex-ext.c */
    double sqrt_MAX_over_4;           /* see complex-ext.c */

};

# define DP     (PFE.dp)
#define LATEST	(PFE.latest)
# define SCR		(PFE.scr)
# define STATE		(PFE.state)
# define BASE		(PFE.base)
# define PRECISION	(PFE.precision)

#endif
