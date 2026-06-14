include test/tester.fth
." Testing <BUILDS ... DOES> ... "
Test{

\ Create a constant which is the opposite
\ 9 oups X X . ==> -9
: OUPS <BUILDS , DOES> @ NEGATE ;

9 OUPS x
S" Test x " x -9 = check
-42 OUPS y
S" Test y " y 42 = check
0 OUPS z
S" Test z " z 0= check

}Test

