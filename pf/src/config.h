#ifndef ___CONFIG_H
#define ___CONFIG_H

#define TARGET_OS "linux-gnu"
#define PF_VERSION  "2.2a"

#if defined ARM
/* ARM processor options */
#define SIZEOF_CHAR   1
#define SIZEOF_SHORT  2
#define SIZEOF_INT    4
#define SIZEOF_LONG   4
#define SIZEOF_FLOAT  4
#define SIZEOF_DOUBLE 8
#define SIZEOF_VOIDP  4

#define PF_BOOT_FILE "/usr/etc/pf.4th"

#else
/* X86-64 processor options */
#define SIZEOF_CHAR   1
#define SIZEOF_SHORT  2
#define SIZEOF_INT    4
#define SIZEOF_LONG   8
#define SIZEOF_FLOAT  4
#define SIZEOF_DOUBLE 8
#define SIZEOF_VOIDP  8

#define PF_BOOT_FILE "/etc/pf.4th"

#endif

/* Both x86 and arm are little endian */
/* 1234 = LIL_ENDIAN, 4321 = BIGENDIAN */
#define BYTEORDER 1234

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
