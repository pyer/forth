: test_include ." Test option '-i' OK" cr quit ;
: test_execute ." Test option '-e' OK" cr quit ;

HERE 2 C, CHAR B C, CHAR L C, constant FAKE-BL
: test_skip
  FAKE-BL FIND
  IF   ." Test option '-s' error"
  ELSE ." Test option '-s' OK"
  THEN
  drop cr quit ;
