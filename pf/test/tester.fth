\ PF unit tests

variable tests
variable success
variable errors
variable stack-depth

: tests+
  1 tests +! ;

: success+
  1 success +! ;

: errors+
  1 errors +! ;

: check ( flag -- )
  tests+
  if success+ drop drop
  else type ." failed" cr errors+ then ;

: Test{
  0 tests !
  0 success !
  0 errors !
  cr
  depth stack-depth ! ;

: }Test ( -- )
  depth stack-depth @ =
  if   tests @ . ." tests, " success @ . ." success, " errors @ . ." errors"
  else ." Stack error"
  then
  cr cr ;

