Pyer's Forth
============

Here is my own Forth environment picked from PFE.
My goal is just to have a Forth environment on my Linux machines and on my Beaglebone Black.

Do not use my 'pf', unless you know what you want.
Please visit the website for the project at http://pfe.sourceforge.net 

*TRUNK*

Full pf with vocabularies

*BRANCH* one\_dictionary

In this branch, pf has no search-order extensions, only one dictionary of Forth words.
Words current, context, only, order, etc... are removed.

*deploy on Ubuntu*
  /usr/local/bin/pf
  /usr/local/etc/pf.fth
  /usr/local/share/pf.help

*deploy on Angstrom (Beaglebone Black)*
  /usr/bin/pf
  /usr/etc/pf.fth
  /usr/share/pf.help


Pierre BAZONNARD
pierre.bazonnard@gmail.com
