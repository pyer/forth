\ PF unit tests

CR ." Running Pier's Forth tests" CR

: check ( flag -- )
   ( depth @ 1 = and )
   if ." ok" else ." FAILED" then
   cr ;

." Core words" CR
." check 0          " 0 0= check
." check 1          " 1 1- 0= check
." check 2          " 2 1- 1 = check
." check 3          " 3 1 2 + = check
variable tmp 16 allot
false tmp !
." check !          " true tmp ! tmp @ check
." chech *          " 6 7 * 42 = check
." chech +          " 6 7 + 13 = check
6 tmp !
." chech +!         " 7 tmp +! tmp @ 13 = check
." chech -          " 42 dup - 0= check
." chech /          " 42 6 / 7 = check
." chech /MOD       " 44 6 /MOD 7 = swap 2 = and check
." chech 0<         " -1 0<  check
." chech 0=         "  0 0=  check
." chech 0<>        "  1 0<> check
." chech 0>         "  1 0>  check
." chech 1+         "  -1 1+ 0= check
." chech 1-         "   1 1- 0= check
." chech 2+         "  -2 2+ 0= check
." chech 2-         "   2 2- 0= check
." chech 2*         "   21 2* 42 = check
." chech 2/         "   42 2/ 21 = check
." chech 1-         "   1 1- 0= check
." chech <          " -1 0 <  check
." chech =          "  0 0 =  check
." chech <>         "  1 0 <> check
." chech >          "  1 0 >  check
." chech >R         "  0 check
." chech ?DUP       "  0 check
false tmp !
." check @          " true tmp ! tmp @ check
." chech ABS        " -1 abs 1 = check
." chech AND        "  true false and false = check
0 tmp ! 42 tmp C!
." check C!         " tmp @ 42 = check
." check C@         " tmp C@ 42 = check
." check COUNT      " tmp count 42 = swap tmp 1+ = and check
." check DEPTH      " 0 0 0 depth tmp ! drop drop drop tmp @ 3 = check
." check DROP       " true false drop check
." check DUP        " tmp @ dup = check
." check EXECUTE    " ' true execute check
." check FILL       " tmp 2 0 fill tmp @ 0= check

." check INVERT     " -1 invert 0= check

." check DO LOOP    " 
." check MAX        " 7 6 max 7 = check
." check MIN        " 7 6 min 6 = check
." check MOD        " 44 6 MOD 2 = check
." check NEGATE     " -1 negate 1 = check
." check NOT        " -1 invert 0= check
." check OR         " true false or true = check
." check OVER       " 3 2 1 over 2 = tmp ! drop drop drop tmp @ check
." check PICK       " 3 2 1 2 pick 3 = tmp ! drop drop drop tmp @ check 
." check R>         " 0 check
." check R@         " 0 check
." check ROLL       " 3 2 1 2 roll 3 = tmp ! drop drop tmp @ check
." check ROT        " 3 2 1 rot 3 = tmp ! drop drop tmp @ check
." check SWAP       " 0 1 swap 0= swap drop check
." check XOR        " 0x55 0xAA xor 0xFF = check
