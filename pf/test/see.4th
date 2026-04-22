." Testing SEE ..."
CR

CREATE new-word
see NEW-WORD

<BUILDS new-builds
see NEW-BUILDS

: def1 create  42 , does> @ . ;
def1 word1
see WORD1

: def2 <builds 42 , does> @ . ;
def2 word2
see WORD2

42 constant LEVEL
see LEVEL

variable VAR
see VAR

3.14 fconstant PI
see PI

fvariable FVAR
see FVAR

: HELLO ." Hello world !" cr ;
see HELLO

: answer [ 42 ] literal . ;
see answer

: pi? [ 3.14 ] fliteral f. ;
see pi?

bye
