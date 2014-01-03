/** 
 * -- Process command line, get memory and start up.
 * 
 *  Copyright (C) Tektronix, Inc. 1999 - 2001.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.3 $
 *     (modified $Date: 2008-04-20 04:46:30 $)
 *
 *  @description
 *  Process command line, get memory and start up the interpret loop of PFE
 */
/*@{*/
/*
#if defined ARM
#include "include/config_arm.h"
#else
#include "include/config_x86.h"
#endif
*/
#include <pfe/def-config.h>

#define _P4_SOURCE 1

#include <stdlib.h>
#include <errno.h>
#include <pfe/engine-set.h>
#include <pfe/os-string.h>
#include <pfe/logging.h>

#ifdef PFE_HAVE_SYS_RESOURCE_H
#include <sys/time.h>
#include <sys/resource.h>
#endif


/************************************************************************/
/* Analyze command line options:                                        */
/************************************************************************/

#ifndef CAPS_ON                 /* USER-CONFIG: */
#define	CAPS_ON		0	/* do you like (faked) caps lock by default? */
#endif
#ifndef UPPER_CASE_ON           /* USER-CONFIG: */
#define	UPPER_CASE_ON	1	/* allow "dup" to find "DUP" by default */
#endif
#ifndef LOWER_CASE_ON           /* USER-CONFIG: */
#define	LOWER_CASE_ON	1	/* allow "Dup" to find "dup" by default */
#endif
#ifndef LWRCASE_FN_ON           /* USER-CONFIG: */
#define LWRCASE_FN_ON	1	/* convert file names to lower case? */
#endif
#ifndef FLOAT_INPUT_ON          /* USER-CONFIG: */
#define FLOAT_INPUT_ON	1	/* allow input of floating point numbers */
#endif

#ifndef TEXT_COLS               /* USER-CONFIG: */
#define	TEXT_COLS	80	/* used only in case p4th fails determining */
#endif
#ifndef TEXT_ROWS               /* USER-CONFIG: */
#define	TEXT_ROWS	25	/* the screen size */
#endif

#define TOTAL_SIZE (P4_KB*1024) /* the shorthand for default-computations */

#ifdef _K12_SOURCE
#undef  LOWER_CASE_ON
#define LOWER_CASE_ON 1
#endif

/*export*/ PFE_CC_THREADED struct p4_Thread  p4_reg;
/*export*/ PFE_CC_THREADED struct p4_Session p4_opt;

/**
 * fill the session struct with precompiled options
_export void
p4_SetOptionsDefault(p4_sessionP set, int len)
 */
void p4_default_options(p4_sessionP set)
{
    if (! set) return;

    p4_memset(set, 0, sizeof(*set));

    /* newstyle option-ext support */
    set->opt.dict = set->opt.space;
    set->opt.dp = set->opt.dict;
    set->opt.last = 0;
    int len = sizeof(*set);
    set->opt.dictlimit = ((p4char*)set) + len;

    set->argv = 0;
    set->argc = 0;
    set->optv = 0;
    set->optc = 0;
    set->boot_name = 0;
    set->isnotatty = 0;
    set->stdio = 0;
    set->caps_on = CAPS_ON;
    set->find_any_case = LOWER_CASE_ON;
    set->lower_case_fn = LWRCASE_FN_ON;
    set->upper_case_on = UPPER_CASE_ON;
#  ifndef P4_NO_FP
    set->float_input = FLOAT_INPUT_ON;
#  else
    set->float_input = 0;
#  endif
    set->license = 0;
    set->warranty = 0;
    set->quiet = 0;
    set->verbose = 0;
    set->debug = 0;
    set->bye = 0;
    set->cols = TEXT_COLS;
    set->rows = TEXT_ROWS;
    set->total_size = TOTAL_SIZE;
    /* TOTAL_SIZE dependent defaults are moved to dict_allocate */
    set->stack_size = 0;
    set->ret_stack_size = 0;

    /*set->boot_include = 0;*/
    set->cpus = P4_MP;
}

static char memory[P4_KB*1024]; /* BSS */
struct p4_Thread* p4TH;

int main (int argc, char** argv)
{
    p4_Thread *thread;
    p4_Session session;
  
    p4_default_options(&session);
//    if ((i=p4_set_options (&session, argc, argv))) return i-1;
    thread = (p4_Thread*) memory;
    p4_memset (thread, 0, sizeof(p4_Thread));

    /* how to override the size of the dict if the user did use an option? */
//    p4_SetDictMem(thread, memory+sizeof(p4_Thread), 0);
    thread->set = &session;
    return p4_Exec (thread); 
}
