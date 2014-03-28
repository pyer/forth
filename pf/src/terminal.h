/**
 *  TERMINAL
 *
 *  @description
 *      The Terminal Wordset contains the console I/O
 */
/*@{*/

/************************************************************************/
int get_cols(void);
int get_rows(void);
int get_outs(void);
void pf_init_terminal(void);

/************************************************************************/
/*
static int  pf_getkey (void) { return fgetc(stdin); }
static void pf_putc_noflush (char c) { fputc(c, stdout); }
static void pf_put_flush (void) { fflush (stdout); }
static void pf_putc (char c) { fputc(c, stdout); fflush(stdout); }
static void pf_puts (const char* s) { fputs (s, stdout); }
*/
#define pf_bell() putchar('\a');
/************************************************************************/

void pf_outc (char c) ; /*{*/
void pf_outs (const char *s) /* type a string */ ; /*{*/
int pf_outf (const char *s,...); /*;*/
void pf_emits (int n, const char c) ; /*{*/
void pf_tab (int n) ; /*{*/
/** TYPE ( string-ptr string-len -- ) [ANS]
 * prints the string-buffer to stdout, see => COUNT and => EMIT
 */
void pf_type (const char *str, p4cell len);

int  pf_getkey (void);
FCode (pf_key);

FCode (pf_emit);
FCode (pf_emits);
FCode (pf_cr);
FCode (pf_type);
FCode (pf_backspace);
FCode (pf_space);
FCode (pf_spaces);

/** _accept_ ( str* str# -- span# )
 * better input facility using lined if possible, otherwise
 * call _expect_noecho when running in a pipe or just _expect_ if no
 * real terminal attached.
 */
//int pf_accept (p4_char_t *tib, int n) ; /*{*/
int pf_accept (char *tib, int len);

FCode (pf_more);
int pf_more_Q (void);
FCode (pf_more_Q);
