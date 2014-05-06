( pf.help version 1.0 )

: ?  @ . ;

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

( Beaglebone Black leds )

: led0 s" /sys/class/leds/beaglebone:green:usr0/brightness" ;
: led1 s" /sys/class/leds/beaglebone:green:usr1/brightness" ;
: led2 s" /sys/class/leds/beaglebone:green:usr2/brightness" ;
: led3 s" /sys/class/leds/beaglebone:green:usr3/brightness" ;

: led_on ( filename len -- )
   s" w" fopen
   [char] 1 over fputc drop
   fclose ;

: led_off ( filename len -- )
   s" w" fopen
   [char] 0 over fputc drop
   fclose ;

: led0_on  ( -- ) led0 led_on  ;
: led0_off ( -- ) led0 led_off ;
: led1_on  ( -- ) led1 led_on  ;
: led1_off ( -- ) led1 led_off ;
: led2_on  ( -- ) led2 led_on  ;
: led2_off ( -- ) led2 led_off ;
: led3_on  ( -- ) led3 led_on  ;
: led3_off ( -- ) led3 led_off ;


.version
cr

: hello ." Hello world !" cr ;
hello
