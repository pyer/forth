#ifndef __PFE_DEF_CELL_H
#define __PFE_DEF_CELL_H

/* 
 * -- The basic types
 *
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.4 $
 *     (modified $Date: 2008-04-20 04:46:31 $)
 *
 *  note that TYPEOF_CELL is either long or int.
 *  It must be atleast as big as a pointer.
 */
#include <pfe/def-config.h>
 
typedef unsigned char  p4char; /* hopefully an 8-bit type */
typedef unsigned short p4word; /* hopefully a 16-bit type */

typedef unsigned char const p4cchar;

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

typedef p4char p4_byte_t;          /* adressing element */
typedef p4char p4_char_t;          /* i/o char element */
typedef p4cell p4_cell_t;          /* computational element */
typedef p4cell p4_bool_t;          /* and as boolean flag */

#endif
