#ifndef __PFE_OS_CTYPE_H
#include <ctype.h>
/*
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.3 $
 *     (modified $Date: 2008-04-20 04:46:30 $)
 */

/* fscking solaris */
#if defined HOST_OS_SOLARIS
#define p4_isspace(x) isspace((unsigned int)(x))
#define p4_isupper(x) isupper((unsigned int)(x))
#define p4_isalnum(x) isalnum((unsigned int)(x))
#define p4_isprint(x) isprint((unsigned int)(x))
#define p4_iscntrl(x) iscntrl((unsigned int)(x))
#define p4_isascii(x) isascii((unsigned int)(x))
#else
#define p4_isspace(x) isspace((unsigned char)(x))
#define p4_isupper(x) isupper((unsigned char)(x))
#define p4_isalnum(x) isalnum((unsigned char)(x))
#define p4_isprint(x) isprint((unsigned char)(x))
#define p4_iscntrl(x) iscntrl((unsigned char)(x))
# ifdef isascii
#define p4_isascii(x) isascii((unsigned char)(x))
# else
#define p4_isascii(X) ((unsigned char)(X) < 0x80)
# endif
#endif

#endif
