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
#if defined SIZEOF_INT && SIZEOF_INT >= SIZEOF_VOIDP
# define       SIZEOF_CELL SIZEOF_INT
# define       TYPEOF_CELL int
#elif defined SIZEOF_LONG && SIZEOF_LONG >= SIZEOF_VOIDP
# define       SIZEOF_CELL SIZEOF_LONG
# define       TYPEOF_CELL long
#else 
# error cell type and size not detected.
#endif

#if defined SIZEOF_INT && SIZEOF_INT == SIZEOF_CELL / 2 
# define       PFE_TYPEOF_HALFCELL int
#elif defined SIZEOF_SHORT && SIZEOF_SHORT == SIZEOF_CELL / 2 
# define       PFE_TYPEOF_HALFCELL short
#else
# error halfcell type not detected
#endif

#define SIZEOF_FCELL SIZEOF_DOUBLE
#define TYPEOF_FCELL double

typedef TYPEOF_CELL		p4cell;	 /* a stack item */
typedef unsigned TYPEOF_CELL	p4ucell; /* dito unsigned */


typedef struct { p4ucell quot, rem; } udiv_t;
typedef struct { p4cell  quot, rem; } fdiv_t;

typedef p4char p4_byte_t;          /* adressing element */
typedef p4char p4_char_t;          /* i/o char element */
typedef p4cell p4_cell_t;          /* computational element */
typedef p4cell p4_bool_t;          /* and as boolean flag */

typedef void (*p4code) (void);		/* pointer to executable code */
typedef p4code *p4xt;			/* type of the "execution token" */

/* declare a primitive */
#define  FCode(X)    void X##_(void) 
#define FXCode_RT(X)      X##_(void)
#define  FCode_RT(X) void X##_(void)
#define FXCode_XE(X)      X##_(void)
#define  FCode_XE(X) void X##_(void)

#define  P4CODE(X)     X##_
#define  FX(X)         X##_() 
#endif
