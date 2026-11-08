include test/tester.fth
." Testing IF ELSE THEN ..."

: test-if-then ( -- flag)
  true IF 1 THEN 1 = ;
: test-if ( -- flag)
  true  IF 1 ELSE 2 THEN 1 = ;
: test-else ( -- flag)
  false IF 1 ELSE 2 THEN 2 = ;

Test{
S" check before  " DEPTH 2 = check
S" check IF THEN " test-if-then check
S" check IF      " test-if check
S" check ELSE    " test-else check
S" check after   " DEPTH 2 = check
}Test

CR
." Testing BEGIN WHILE REPEAT ..."

VARIABLE counter

: test-while ( -- flag)
  0 counter !
  BEGIN
    counter @ 3 <
  WHILE
    1 counter +!
  REPEAT
  counter @ 3 = ;

Test{
S" check before  " DEPTH 2 = check
S" check WHILE   " test-while check
S" check after   " DEPTH 2 = check
}Test

CR
." Testing BEGIN UNTIL ..."

: test-until ( -- flag)
  0 counter !
  BEGIN
    1 counter +!
    counter @ 3 >
  UNTIL
  counter @ 4 = ;

Test{
S" check before  " DEPTH 2 = check
S" check UNTIL   " test-until check
S" check after   " DEPTH 2 = check
}Test

CR
." Testing CASE ..."

: test-case ( x -- y)
  CASE
    1 OF 11 ENDOF
    2 OF 22 ENDOF
    3 OF 33 ENDOF
    44 swap
  ENDCASE ;

Test{
S" check before  " DEPTH 2 = check
S" check case 1  " 1 test-case 11 = check
S" check case 2  " 2 test-case 22 = check
S" check case 3  " 3 test-case 33 = check
S" check case 4  " 4 test-case 44 = check
S" check after   " DEPTH 2 = check
}Test

CR
." Testing DO LOOP ..."

: test-loop ( -- flag)
  1
  3 0 DO
    1 +
  LOOP
  4 = ;

: test+loop ( -- flag)
  1
  7 0 DO
    1 +
  2 +LOOP
  5 = ;

Test{
S" check before   " DEPTH 2 = check
S" check DO LOOP  " test-loop check
S" check DO +LOOP " test+loop check
S" check after    " DEPTH 2 = check
}Test

