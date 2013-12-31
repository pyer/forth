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

#include <pfe/engine-set.h>
#include <pfe/option-set.h>
#include <stdlib.h>
#include <pfe/os-string.h>
#include <errno.h>

#ifdef PFE_HAVE_SYS_RESOURCE_H
#include <sys/time.h>
#include <sys/resource.h>
#endif

static char memory[P4_KB*1024]; /* BSS */

int main (int argc, char** argv)
{
    p4_Thread* thread;
    p4_Session session;
    int i;
  
    p4_default_options(&session);
    if ((i=p4_set_options (&session, argc, argv))) return i-1;
    thread = (p4_Thread*) memory;
    p4_memset (thread, 0, sizeof(p4_Thread));

    /* how to override the size of the dict if the user did use an option? */
//    p4_SetDictMem(thread, memory+sizeof(p4_Thread), 0);
    thread->set = &session;
    return p4_Exec (thread); 
}

