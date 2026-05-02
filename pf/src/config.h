#ifndef ___CONFIG_H
#define ___CONFIG_H

#define TARGET_OS "linux-gnu"
#define PF_VERSION  "2.4a"

#define PF_BOOT_FILE "/usr/lib/pf.4th"
#define PF_HELP_FILE "/usr/share/man/pf.txt"

#if defined ARM
/* ARM processor options */
#define SIZEOF_CHAR   1
#define SIZEOF_SHORT  2
#define SIZEOF_INT    4
#define SIZEOF_LONG   4
#define SIZEOF_FLOAT  4
#define SIZEOF_DOUBLE 8
#define SIZEOF_VOIDP  4

#else
/* X86-64 processor options */
#define SIZEOF_CHAR   1
#define SIZEOF_SHORT  2
#define SIZEOF_INT    4
#define SIZEOF_LONG   8
#define SIZEOF_FLOAT  4
#define SIZEOF_DOUBLE 8
#define SIZEOF_VOIDP  8

#endif

/* Both x86 and arm are little endian */
/* 1234 = LIL_ENDIAN, 4321 = BIGENDIAN */
#define BYTEORDER 1234

//#define PF_WITH_FFA      1       /* use seperate FlagField */
#define PF_WITH_FLOATING 1       /* enable floating point numbers */

#define PATH_LENGTH 256

#define TOTAL_SIZE (1024*1024) /* the shorthand for default-computations */

/* a cell has at least the size of a pointer but the type of an integer */
#if defined SIZEOF_INT && SIZEOF_INT >= SIZEOF_VOIDP
# define       SIZEOF_CELL SIZEOF_INT
# define       TYPEOF_CELL int
#elif defined SIZEOF_LONG && SIZEOF_LONG >= SIZEOF_VOIDP
# define       SIZEOF_CELL SIZEOF_LONG
# define       TYPEOF_CELL long
#else 
# error cell type and size not detected.
#endif

#define SIZEOF_FCELL SIZEOF_DOUBLE
#define TYPEOF_FCELL double

#endif
