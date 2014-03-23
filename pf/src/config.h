#ifndef ___CONFIG_H
#define ___CONFIG_H
/*
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.7 $
 *     (modified $Date: 2008-05-11 12:29:19 $)
 */
#if defined ARM
#include "config_arm.h"
#define PF_BOOT_FILE "/etc/pf.fth"
#else
#include "config_x86.h"
#define PF_BOOT_FILE "/usr/local/etc/pf.fth"
#endif

#define TOTAL_SIZE (1024*1024) /* the shorthand for default-computations */
struct p4_Thread* p4TH;
#define PFE (*p4TH)
#define SP (PFE.sp)
#define RP (PFE.rp)
#define IP (PFE.ip)

#ifndef _pfe_const
#define _pfe_const const
#endif

#ifndef _pfe_inline
#define _pfe_inline inline
#endif

#ifndef _pfe_restrict
#define _pfe_restrict restrict
#endif

#ifndef _pfe_off_t
#define _pfe_off_t off_t
#endif

/* library defines */

#ifndef _extern
#define _extern extern
#endif

# define _export

/* suspend problems with important defines from pfe/_config.h */

#ifndef PFE_BYTEORDER
# if defined WORDS_BIGENDIAN
# define PFE_BYTEORDER 4321
# elif defined BYTEORDER
# define PFE_BYTEORDER BYTEORDER
# else
#  ifdef __GNUC__
#  warning no byteorder defined, assuming little-endian
#  define PFE_BYTEORDER 1234
#  else
#  error no byteorder defined, define BYTEORDER or PFE_BYTEORDER
#  endif
# endif
#endif

#define PFE_WITH_FFA    1       /* use seperate FlagField */

#if defined PFE_HAVE_FTELLO && defined PFE_HAVE_FSEEKO
#define PFE_USE_FSEEKO
#define _p4_ftello ftello
#define _p4_fseeko fseeko
#define _p4_off_t  _pfe_off_t
#else
#define _p4_ftello ftell
#define _p4_fseeko fseek
#define _p4_off_t  long
#endif

/* A glibc bug: fseeko is only prototyped when UNIX98 or LFS or GNU_SOURCE set.
 * The gcc will warn about the missing prototype, but link to the existing
 * fseeko in the libc. However, in a largefile-sensitive system it should be
 * linked to fseeko64 instead, otherwise we get a nice callframe mismatch!
 */
#if defined PFE_USE_FSEEKO
#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE 1 /* AC_SYS_LAREFILE does not set this on its own */
#endif
#endif

#ifndef P4_OFF_T_MAX
#define P4_OFF_T_MAX (((_p4_off_t)1) << (sizeof(_p4_off_t)*8-1))
#endif

#endif
