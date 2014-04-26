#ifndef ___CONFIG_H
#define ___CONFIG_H

#define PF_VERSION  "2.0c"

#if defined ARM
#include "config_arm.h"
#define PF_BOOT_FILE "/usr/etc/pf.fth"
#define PF_HELP_FILE "/usr/share/pf.help"
#else
#include "config_x86.h"
#define PF_BOOT_FILE "/usr/local/etc/pf.fth"
#define PF_HELP_FILE "/usr/local/share/pf.help"
#endif

#define PF_WITH_FFA      1       /* use seperate FlagField */
#define PF_WITH_FLOATING 1       /* enable floating point numbers */

#define PATH_LENGTH 256

#define TOTAL_SIZE (1024*1024) /* the shorthand for default-computations */


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

#endif
