#include <pfe/def-config.h>

#ifdef PFE_WITH_X11
#include <src/term-x11.c>
#endif

#if PFE_TERM_DEF == 1
#include <src/term-wincon.c>
#elif PFE_TERM_DEF == 2
#include <src/term-dj.c>
#elif PFE_TERM_DEF == 3
#include <src/term-k12.c>
#elif PFE_TERM_DEF == 4
#include <src/term-emx.c>
#elif PFE_TERM_DEF == 5
#include <src/term-curses.c>
#elif PFE_TERM_DEF == 6
#include <term-lib.c>
#elif PFE_TERM_DEF == 7
#include <src/term-curses.c>
#elif PFE_TERM_DEF == 8
#include <src/term-lib.c>
#elif PFE_TERM_DEF == 9
#include <src/term-wat.c>
#elif PFE_TERM_DEF == 11
#include <src/term-x11.c>
#else
#include <term-lib.c>
#endif


