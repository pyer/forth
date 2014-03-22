#ifndef __PF_SESSION_H
#define __PF_SESSION_H

/* this is called options structure, since this aggregate communicates
   the command-line. But it has enhanced over time. Just expect it to
   be THE data-structure unique to each thread (started via main).
   You can actually have MP_MAX forth-processors per thread, so if
   your OS isn`t multi-threaded, what the heck, use this feature!
*/

typedef struct p4_Session 	p4_Session;
typedef struct p4_Thread	p4_Thread;

typedef struct _p4_term_struct  p4_term_struct;

#define PATH_LENGTH 256

struct p4_File			/* describes a file */
{
    FILE *f;			   /* associated ANSI-C file structure */
    char mdstr[4];		   /* mode string for fopen() */
    char mode;			   /* mode code for open_file() */
//    signed char last_op;	   /* -1 write, 0 none, 1 read */
//    p4word len;			   /* if stream: length of input line */
//    p4ucelll size;		   /* if block file: size of file in blocks */
//    p4ucelll n;			   /* block in buffer or source line */
//    p4cell updated;		   /* if block file: block updated? */
//    union { _p4_off_t pos;	   /* saved position, e.g. beginning of line */
//        char compat[8]; } line;
//    char name[PATH_LENGTH];	   /* file name */
};

struct p4_Except
{
    p4cell magic;
    p4xcode** rpp;              /* P4_REGRP_T */
    p4xcode *ipp;               /* P4_REGIP_T */
    p4cell *spp;                /* P4_REGSP_T */
    p4cell *lpp;                /* P4_REGLP_T */
    double *fpp;                /* P4_REGFP_T */
    jmp_buf jmp;
    p4_Except *prev;
};

typedef struct p4_Exception p4_Exception;
struct p4_Exception
{
    struct p4_Exception* next;
    p4cell id;
    char const * name;
};


struct p4_Session
{
    int argc;
    char const ** argv;
    unsigned    isnotatty:2,	/* running in canonical mode */
	        stdio:1,	/* standard input isn't-tty: work as filter */
	        caps_on:1,	/* exchange lower and upper case chars */
	        find_any_case:1,/* make case-insensitive find default */
//	        lower_case_fn:1,/* convert file names to lower case? */
	        float_input:1,	/* disables floating point input when false */
		debug:1,	/* enable a few more outputs */
                upper_case_on:1,/* make lower case words find upper case */
                lastbit:1;      /* last bit */
//    int	cols, rows;	/* size of screen */
    p4ucelll	total_size;
    p4ucelll	stack_size;
    p4ucelll	ret_stack_size;
    p4ucelll	float_stack_size;

    char ** boot_name;          /* points to argv[0] usually... */
    char const** optv;
    p4ucell     optc;
    unsigned    wordlists;       /* p4ucell might be 64bit (16bit is okay) */
//    void*       modules;         /* p4Words* : dl-internal / dl-ext */
    p4ucell     padding[4];      /* padding cells for binary compatibility */

    /* newstyle options support via option-ext */
    struct
    {
        p4_namebuf_t* last;
        p4_namebuf_t* dp;
        p4_byte_t* dict;
        p4_byte_t* dictlimit;
        p4_byte_t  space[284]; /* atleast a few headers... */
    } opt; /* must be last in this structure !! */
};


struct p4_Thread
{
    p4_byte_t *dp;		/* actual top of the dictionary */

    p4_byte_t* dict;		/*  dictionary */
    p4_byte_t* dictlimit;
    p4cell* stack;		/*  data stack */
    p4cell* s0;
    double *fstack;		/*  floating point stack */
    double* f0;
    p4xcode** rstack;		/*  return stack */
    p4xcode**  r0;

    char* history;		/*  command line history buffer */
    char* history_top;
    p4_File* files;		/*  files */
    p4_File* files_top;

/* VM */
    p4xcode*  ip; /* the intruction pointer */
    p4xt      wp;    /* speed up the inner interpreter */
    p4cell*   sp; /* the stack pointer */
    p4xcode** rp; /* the return stack pointer */
//    p4cell*   lp; /* the pointer to local variables */
    double*   fp; /* the floating point stack pointer */

/* jmp_buf */
    jmp_buf loop;       /* QUIT and ABORT do a THROW which longjmp() */
       			   /* here thus C-stack gets cleaned up too */
/*Options*/
    p4_Session* set;        /* contains cpu-pointers */
#define P4_opt  (*PFE.set)
#define PFE_set (*PFE.set)

/*Dict*/
    p4_byte_t *fence;		/* can't forget below that address */
    p4_namebuf_t *last;		/* NFA of most recently CREATEd header */

    p4_Wordl *voc_link;		/* link to chained word lists */
    p4_Wordl **context;	        /* dictionary search order */
//    p4_Wordl *only__;		/* ONLY is always searched OBSOLETE */
    p4_Wordl *current;		/* points to vocabulary in extension */
    p4_Wordl **dforder;	        /* default dictionary search order */
    p4_Wordl *dfcurrent;        /* default definition wordlist */
    p4_char_t *hld;		/* auxiliary pointer for number output */
    p4cell dpl;			/* position of input decimal point */

    p4_Except *catchframe;	/* links to chain of CATCHed words */

    p4ucell to_in;		/* input parsing position */
    p4ucell scr;		/* latest LISTed block number */
    p4cell out;			/* current output column on screen */
    p4cell state;		/* interpreting (0) or compiling (-1) */
    p4cell *csp;		/* compiler security, saves sp here */
    p4ucell base;		/* of number i/o conversion */
    p4cell precision;		/* floating point output precision */

//    p4xt key;			/* executed by KEY */
//    p4xt emit;			/* executed by EMIT */
//    p4xt expect;		/* executed by EXPECT */
//    p4xt type;			/* executed by TYPE */

    p4cell wordl_flag;		/* do toupper() before dictionary search */
//    p4cell lower_case_fn;	/* do tolower() on file names */
    p4cell redefined_msg;	/* no `"xxx" is redefined' msg if false */
    p4cell float_input;		/* don't try floating pt input when false */
    p4cell reset_order;		/* if true: reset search order on ABORT */

    p4_File *stdIn;		/* C-library standard files */
    p4_File *stdOut;		/* mapped to Forth-files */
    p4_File *stdErr;
//    p4ucell more;		/* for a more-like effect */
//    p4ucell lines;

//    struct lined accept_lined;	/* better input-facilities for accep := 0*/
//    p4xt  fkey_xt[10];		/* fkey_executes_xt := 0*/
    void (*execute)(p4xt);	/* := normal_execute */

/* core.c */
    p4code semicolon_code;      /* the code to run at next semicolon */

/* main-sub / dict-sub */
//    int exitcode;
//    void (*system_terminal)(void);
    p4_Wordl *atexit_wl;	     /* atexit dictionary holder */
    p4_byte_t* volatile forget_dp;   /* temporary of forget */

/* term*.c */
    void* priv;         	/* private term area, better be also in p[] */
    p4_term_struct* term;
    char const ** rawkey_string;  /* pointer to terminal escape sequences */
    char const ** control_string; /* pointer to terminal control sequences */
                        	/* as used by termunix.c */
    int (*wait_for_stdin)(void);

    void (*on_stop) (void);     /* = p4_system_terminal; */
    void (*on_continue) (void); /* = p4_interactive_terminal; */
    void (*on_winchg) (void);   /* = p4_query_winsize; */
    void (*on_sigalrm) (void);  /* really from signal.c */

/* support.c/xception */
    p4code throw_cleanup;

/* new forth-wordlist mechanism */
    p4_Wordl* forth_wl;

    p4cell next_exception;
    p4_Exception* exception_link;

    struct {
	const p4_char_t* ptr;
	unsigned len;          /* p4ucell is 8byte on x86_64 but */
    } word;                    /* parsing is not exceeding 16bit anyway */

    void* chain_link;          /* see chain-ext.c */
    p4_Wordl* abort_wl;        /* see engine-sub/chainlist-ext REDO-WL */
    p4_Wordl* prompt_wl;       /* see engine-sub/chainlist-ext DO-WL */

    double asinh_MAX_over_4;          /* see complex-ext.c */
    double sqrt_MAX_over_4;           /* see complex-ext.c */

};

#define MIN_HOLD	0x100
#define MIN_PAD		0x400

# define p4_S0 PFE.s0
# define p4_F0 PFE.f0
# define p4_R0 PFE.r0

# define p4_DP          (PFE.dp)
# define p4_HLD		(PFE.hld)
# define p4_DPL		(PFE.dpl)
# define p4_PAD		((p4_char_t *)p4_DP + MIN_HOLD)
# define p4_FENCE	(PFE.fence)
# define p4_LAST	(PFE.last)
# define p4_VOC_LINK	(PFE.voc_link)
# define p4_CONTEXT	(PFE.context)
# define p4_DFORDER     (PFE.dforder)
# define p4_DFCURRENT   (PFE.dfcurrent)
# define p4_ONLY	(PFE.context[PFE_set.wordlists])
# define p4_CURRENT	(PFE.current)

# define DP		p4_DP
# define HLD		p4_HLD
# define DPL		p4_DPL
# define PAD		p4_PAD
# define FENCE		p4_FENCE
# define LAST		p4_LAST
# define VOC_LINK	p4_VOC_LINK
# define CONTEXT	p4_CONTEXT
# define DEFAULT_ORDER	p4_DFORDER
# define ONLY		p4_ONLY
# define CURRENT	p4_CURRENT

# define p4_DP_CHAR     p4_DP
# define p4_DP_CELL     ((p4cell*)(p4_DP))

# define P4_UPPER_CASE_FLAGS (WORDL_NOCASE|WORDL_UPPER_CASE|WORDL_UPPER_DEFS)

# define p4_SCR			(PFE.scr)
# define p4_STATE		(PFE.state)
# define p4_CSP			(PFE.csp)
# define p4_BASE		(PFE.base)
# define p4_PRECISION		(PFE.precision)
# define p4_WORDL_FLAG		(PFE.wordl_flag)
# define p4_UPPER_CASE		(PFE.wordl_flag & P4_UPPER_CASE_FLAGS)
# define p4_LOWER_CASE		(PFE.wordl_flag & WORDL_NOCASE) /*depracated*/
//# define p4_LOWER_CASE_FN	(PFE.lower_case_fn)
# define p4_REDEFINED_MSG	(PFE.redefined_msg)
# define p4_FLOAT_INPUT		(PFE.float_input)
# define p4_RESET_ORDER		(PFE.reset_order)

# if PFE_USE_QUOTED_PARSE
# define p4_QUOTED_PARSE        (PFE.quoted_parse)
# else
# define p4_QUOTED_PARSE        0
#endif

# define SCR		p4_SCR
# define STATE		p4_STATE
# define CSP		p4_CSP
# define BASE		p4_BASE
# define PRECISION	p4_PRECISION
# define WORDL_FLAG	p4_WORDL_FLAG
# define UPPER_CASE	p4_UPPER_CASE
# define LOWER_CASE	p4_LOWER_CASE
# define LOWER_CASE_FN	p4_LOWER_CASE_FN
# define REDEFINED_MSG	p4_REDEFINED_MSG
# define FLOAT_INPUT	p4_FLOAT_INPUT
# define RESET_ORDER	p4_RESET_ORDER

typedef p4_Wordl 	Wordl;
typedef p4_File 	File;
typedef p4_Except	Except;

#endif
