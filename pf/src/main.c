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
#include <pfe/def-config.h>

#define _P4_SOURCE 1

#include <stdlib.h>
#include <errno.h>
#include <pfe/engine-set.h>
#include <pfe/os-string.h>
#include <pfe/logging.h>
#include <pfe/def-restore.h>
#include <pfe/term-sub.h>

#ifdef PFE_HAVE_SYS_RESOURCE_H
#include <sys/time.h>
#include <sys/resource.h>
#endif

/************************************************************************/
static char memory[TOTAL_SIZE]; /* BSS */
struct p4_Thread* p4TH;
/************************************************************************/

int main (int argc, char** argv)
{
    p4_Thread* thread = (p4_Thread*) memory;
    p4_Session session;
  
    p4_default_options(&session);
//    if ((i=p4_set_options (&session, argc, argv))) return i-1;
    p4_memset (thread, 0, sizeof(p4_Thread));
    thread->set = &session;
    P4_CALLER_SAVEALL;
    p4_run_boot_system(thread);
    p4_run_application(thread);
    p4_cleanup_terminal();
    P4_CALLER_RESTORE;
    return thread->exitcode;
}
