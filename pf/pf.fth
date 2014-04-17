( pf.help version 1.0 )

: decimal 10 base ! ;
: hex     16 base ! ;

32 CONSTANT BL

: cursor_on  27 emit ." [?25h" ;
: cursor_off 27 emit ." [?25l" ;

: >= < not ;
: <= > not ;

: 2DROP  drop drop ;
: 2DUP   over over ;
: 2SWAP  3 roll 3 roll ;
: D=     rot = rot rot = and ; 

: ON     true  swap ! ;
: OFF    false swap ! ;

.version
cr

: hello ." Hello world !" cr ;
hello
