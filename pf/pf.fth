( pf.help version 1.0 )

: decimal 10 base ! ;
: hex     16 base ! ;

32 CONSTANT BL

: >= < not ;
: <= > not ;

: 2DROP  drop drop ;
: 2DUP   rot rot ;
: 2SWAP  3 roll 3 roll ;
: D=     rot = rot rot = = ; 

: ON     true  swap ! ;
: OFF    false swap ! ;

.version
cr

: hello ." Hello world !" cr ;
hello
