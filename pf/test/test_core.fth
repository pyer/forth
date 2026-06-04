\ PF unit tests
include test/tester.fth

\ helpers
variable tmp 16 allot

: check_?dup ( -- flag )
  1 ?dup 1 = swap 1 = and
  0 ?dup 0 = and ;

." Testing core words"
Test{

S" check 0          " 0 0= check
S" check 1          " 1 1- 0= check
S" check 2          " 2 1- 1 = check
S" check 3          " 3 1 2 + = check
false tmp !
S" check !          " true tmp ! tmp @ check
S" check *          " 6 7 * 42 = check
S" check +          " 6 7 + 13 = check
6 tmp !
S" check +!         " 7 tmp +! tmp @ 13 = check
S" check -          " 42 dup - 0= check
S" check /          " 42 6 / 7 = check
S" check /MOD       " 44 6 /MOD 7 = swap 2 = and check
S" check 0<         " -1 0<  check
S" check 0=         "  0 0=  check
S" check 0<>        "  1 0<> check
S" check 0>         "  1 0>  check
S" check 1+         "  -1 1+ 0= check
S" check 1-         "   1 1- 0= check
S" check 2+         "  -2 2+ 0= check
S" check 2-         "   2 2- 0= check
S" check 2*         "   21 2* 42 = check
S" check 2/         "   42 2/ 21 = check
S" check <          " -1 0 <  check
S" check =          "  0 0 =  check
S" check <>         "  1 0 <> check
S" check >          "  1 0 >  check
S" check ?DUP       " check_?dup check
false tmp !
S" check @          " true tmp ! tmp @ check
S" check ABS        " -1 abs 1 = check
S" check AND        "  true false and false = check
0 tmp ! 42 tmp C!
S" check C!         " tmp @ 42 = check
S" check C@         " tmp C@ 42 = check

(  CELL+        p4_cell_plus)
(  CELLS        p4_cells)
(  CHAR+        p4_char_plus)
(  CHARS        p4_chars)
(  CMOVE        p4_cmove)
(  CMOVE>       p4_cmove_up)
(  MOVE         p4_move)

S" check DEPTH      " depth tmp ! 0 0 0 depth tmp @ - tmp ! drop drop drop tmp @ 3 = check
S" check DROP       " true false drop check
S" check DUP        " tmp @ dup = check
S" check EXECUTE    " ' true execute check
S" check FILL       " tmp 2 0 fill tmp @ 0= check
S" check INVERT     " -1 invert 0= check
S" check MAX        " 7 6 max 7 = check
S" check MIN        " 7 6 min 6 = check
S" check MOD        " 44 6 MOD 2 = check
S" check NEGATE     " -1 negate 1 = check
S" check NOT        " -1 invert 0= check
S" check OR         " true false or true = check
S" check OVER       " 3 2 1 over 2 = tmp ! drop drop drop tmp @ check
S" check PICK       " 3 2 1 2 pick 3 = tmp ! drop drop drop tmp @ check 
S" check ROLL       " 3 2 1 2 roll 3 = tmp ! drop drop tmp @ check
S" check ROT        " 3 2 1 rot 3 = tmp ! drop drop tmp @ check
S" check SWAP       " 0 1 swap 0= swap drop check
S" check XOR        " 0x55 0xAA xor 0xFF = check
}Test

." Testing core extension words"
Test{

(  BLANK        p4_blank)
(  ERASE        p4_erase)
S" check FALSE    " false 0=  check
S" check TRUE     " true -1 = check
S" check NIP      " true 1 2 nip 2 = and check
S" check TUCK     " 1 0 tuck drop drop 0= check
S" check WITHIN   " 1 2 3 within check

}Test

." Testing interpreter words"
Test{

base @ PAD ! ( PAD is used as a temporary variable )
S" check BASE       " base @ 10 = check
S" check DECIMAL    " base @ PAD ! decimal base @ PAD @ base ! 10 = check
S" check HEX        " base @ PAD ! hex     base @ PAD @ base ! 16 = check

    ( "SOURCE",       pf_source)
    ( "SIGN",         pf_sign)
    ( "<#",           pf_less_sh)
    ( "#",            pf_sh)
    ( "#>",           pf_sh_greater)
    ( "#S",           pf_sh_s)
    ( ">NUMBER",      pf_to_number)
: dummy 42 ;
S" check '          " ' dummy execute 42 = check
S" check LATEST     " ' dummy latest name>cfa = check

create foo
S" check HERE       " ' foo >body here = check
S" check >BODY      " ' foo >body here = check
S" check NAME>CFA   " ' foo latest name>cfa = check

S" check CHAR       " CHAR B 66 = check
S" check COUNT      " HERE 1 c, 66 c, COUNT 1 = swap C@ 66 = AND check

HERE 4 C, CHAR C C, CHAR H C, CHAR A C, CHAR R C, CONSTANT STRING1
S" check FIND standard word  " STRING1 find -1 = swap ' CHAR = AND check
HERE 1 C, CHAR [ C, CONSTANT STRING2
IMMEDIATE \ Set the last created word STRING immediate
S" check FIND immediate word " STRING2 find  1 = swap ' [ = AND check
HERE 3 C, CHAR N C, CHAR O C, CHAR P C, CONSTANT STRING3
S" check FIND undefined word " STRING3 find  0 = swap STRING3 = AND check

    ( "HOLD",         pf_hold)
    ( "PAD",          pf_pad)

    ( ".",            pf_dot)
    ( ".\"",          pf_dot_quote)
    ( ".R",           pf_dot_r)
    ( "C\"",          pf_c_quote)
    ( "S\"",          pf_s_quote)
    ( "\\",           pf_backslash)
    ( "(",            pf_paren)
    ( ".(",           pf_dot_paren)
    ( "INCLUDE",      pf_include)
    ( "INCLUDED",     pf_included)

}Test
