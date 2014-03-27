#ifndef __PF_TYPES_H
#define __PF_TYPES_H
/**
 * -- pf's data types
 *
 *  Copyright (C) Pierre Bazonnard 2013 - 2014.
 *
 */

/* determines the dimension of any given vector */
#ifndef DIM
#define DIM(X)		((int)(sizeof (X) / sizeof *(X)))
#endif

 
typedef char  p4char; /* hopefully an 8-bit type */
//typedef unsigned short p4word; /* hopefully a 16-bit type */

/* a cell has atleast the size of a pointer but the type of an integer */
#if defined PFE_SIZEOF_INT && PFE_SIZEOF_INT >= PFE_SIZEOF_VOIDP
# define       PFE_SIZEOF_CELL PFE_SIZEOF_INT
# define       PFE_TYPEOF_CELL int
#elif defined PFE_SIZEOF_LONG && PFE_SIZEOF_LONG >= PFE_SIZEOF_VOIDP
# define       PFE_SIZEOF_CELL PFE_SIZEOF_LONG
# define       PFE_TYPEOF_CELL long
#else 
# error cell type and size not detected.
#endif

#if defined PFE_SIZEOF_INT && PFE_SIZEOF_INT == PFE_SIZEOF_CELL / 2 
# define       PFE_TYPEOF_HALFCELL int
#elif defined PFE_SIZEOF_SHORT && PFE_SIZEOF_SHORT == PFE_SIZEOF_CELL / 2 
# define       PFE_TYPEOF_HALFCELL short
#else
# error halfcell type not detected
#endif

#define PFE_ALIGNOF_CELL   PFE_SIZEOF_INT
#define PFE_ALIGNOF_SFLOAT PFE_SIZEOF_FLOAT
#define PFE_ALIGNOF_DFLOAT PFE_SIZEOF_DOUBLE

typedef PFE_TYPEOF_CELL			p4cell;	 /* a stack item */
typedef unsigned PFE_TYPEOF_CELL	p4ucell; /* dito unsigned */

# if PFE_SIZEOF_CELL+0 == PFE_SIZEOF_LONG+0
typedef long p4celll;                  /* using the C type "long" saves us */
typedef unsigned long p4ucelll;        /* a couple of warnings on LP64 */
# else
typedef p4cell p4celll;       /* FIXME: default p4cell should be "long" */
typedef p4ucell p4celll;      /* instead of the traditional "int" default */
# endif


typedef struct
{ 
    p4cell hi; 
    p4ucell lo; 
} p4dcell;	/* dito, double precision signed */

typedef struct 
{ 
    p4ucell hi;
    p4ucell lo; 
} p4udcell;	/* dito, double precision unsigned */

typedef struct				/* "map" of a cell */
{
#if PFE_BYTEORDER == 4321
    unsigned PFE_TYPEOF_HALFCELL hi;
    unsigned PFE_TYPEOF_HALFCELL lo;
#else
    unsigned PFE_TYPEOF_HALFCELL lo;
    unsigned PFE_TYPEOF_HALFCELL hi;
#endif
} p4ucell_hi_lo;

typedef struct { p4ucell quot, rem; } udiv_t;
typedef struct { p4cell  quot, rem; } fdiv_t;

typedef p4char p4_byte_t;          /* adressing element */
typedef p4char p4_char_t;          /* i/o char element */
typedef p4cell p4_cell_t;          /* computational element */
typedef p4cell p4_bool_t;          /* and as boolean flag */

typedef void (*p4code) (void);		/* pointer to executable code */
typedef p4code *p4xt;			/* type of the "execution token" */
typedef p4xt    p4xcode;                /* compiled "execution token" */

/* declare a primitive */
#define  FCode(X)    void X##_(void) 
#define FXCode_RT(X)      X##_(void)
#define  FCode_RT(X) void X##_(void)
#define FXCode_XE(X)      X##_(void)
#define  FCode_XE(X) void X##_(void)

#define  P4CODE(X)     X##_
#define  FX(X)         X##_() 
#endif
