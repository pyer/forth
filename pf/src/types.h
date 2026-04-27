#ifndef __PF_TYPES_H
#define __PF_TYPES_H
/**
 * -- pf's data types
 *
 *  Copyright (C) Pierre Bazonnard 2013 - 2014.
 *
 */

 
typedef char  p4char; /* hopefully an 8-bit type */

typedef TYPEOF_CELL		p4cell;	 /* a stack item */
typedef unsigned TYPEOF_CELL	p4ucell; /* dito unsigned */


typedef struct { p4ucell quot, rem; } udiv_t;
typedef struct { p4cell  quot, rem; } fdiv_t;

typedef void (*p4code) (void);		/* pointer to executable code */
typedef p4code *p4xt;			/* type of the "execution token" */

#endif
