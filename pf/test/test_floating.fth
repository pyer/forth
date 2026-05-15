include test/tester.fth

\ helpers
fvariable tmp

." Testing floating point words"
Test{
S" check PRECISON   " precision @ 6 = check \ default precision
\ S" check FALIGN",     p4_d_f_align)
\ S" check FALIGNED",   p4_d_f_aligned)
S" check >FLOAT     " S" 1.234" >float 1.234 F= and check
S" check S>F        " 1 S>F 1.0 F= check
S" check F>S        " 1.234 F>S 1 = check
S" check F!         " 1.234 tmp F! tmp F@ 1.234 F= check
S" check F*         " 1.2 3.4 F* 4.08 F= check
S" check F+         " 1.2 3.4 F+ 4.6  F= check
S" check F-         " 4.6 3.4 F- 1.2  F= check
S" check F/         " 4.8 4.0 F/ 1.2  F= check
S" check F0<        " -1.234 F0< check
S" check F0=        " 0.0 F0= check
S" check F<         " -1.2 3.4 F< check
S" check F=         " 1.2 1.2 F= check
1.2 tmp F!
S" check F@         " tmp F@ 1.2 F= check
( FCONSTANT )
S" check FDEPTH     " fdepth 0= check
S" check FDROP      " 1.2 3.4 FDROP 1.2 F= check
S" check FDUP       " 1.2 FDUP F= check
\ ("FLITERAL",   p4_f_literal)
\ S" check FLOAT+",     p4_float_plus)
\ S" check FLOATS",     p4_floats)
S" check FLOOR      " 1.9 floor 1.0 F= -3.2 floor -4.0 F= and check
S" check FMAX       " 1.2 -2.0 FMAX  1.2 F= check
S" check FMIN       " 1.2 -2.0 FMIN -2.0 F= check
S" check FNEGATE 1  " 1.234 FNEGATE -1.234 F= check
S" check FNEGATE 2  " -1.234 FNEGATE 1.234 F= check
\ S" check FOVER",     p4_f_over)
\ S" check FROT",     p4_f_rot)
\ S" check FROUND",     p4_f_round)
\ S" check FSWAP",     p4_f_swap)
\     P4_RTco ("FVARIABLE",   p4_f_variable)
\ S" check REPRESENT",   p4_represent)
}Test

." Testing floating point extension words"
Test{
\ S" check F.",     pf_f_dot)
\ S" check FE.",     pf_f_e_dot)
\ S" check F**",     p4_f_star_star)
S" check FABS 1     "  1.234 FABS 1.234 F= check
S" check FABS 2     " -1.234 FABS 1.234 F= check
}Test

