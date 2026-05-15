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
S" chech *          " 6 7 * 42 = check
S" chech +          " 6 7 + 13 = check
6 tmp !
S" chech +!         " 7 tmp +! tmp @ 13 = check
S" chech -          " 42 dup - 0= check
S" chech /          " 42 6 / 7 = check
S" chech /MOD       " 44 6 /MOD 7 = swap 2 = and check
S" chech 0<         " -1 0<  check
S" chech 0=         "  0 0=  check
S" chech 0<>        "  1 0<> check
S" chech 0>         "  1 0>  check
S" chech 1+         "  -1 1+ 0= check
S" chech 1-         "   1 1- 0= check
S" chech 2+         "  -2 2+ 0= check
S" chech 2-         "   2 2- 0= check
S" chech 2*         "   21 2* 42 = check
S" chech 2/         "   42 2/ 21 = check
S" chech <          " -1 0 <  check
S" chech =          "  0 0 =  check
S" chech <>         "  1 0 <> check
S" chech >          "  1 0 >  check
S" chech ?DUP       " check_?dup check
false tmp !
S" check @          " true tmp ! tmp @ check
S" chech ABS        " -1 abs 1 = check
S" chech AND        "  true false and false = check
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

