Pyer's Forth
============

Version
-------
- Pyer's Forth 2.1a
- (May  6 2014 19:30:15)
- Not Forth-2012 System (Subset)


Compliance to Forth-2012 System
===============================
PF doesn't comply to Forth-2012 because a lot of words are missing.
Implemented words are Forth-2012 compliant.

Core words
----------
- Some words are not implemented.

File-Access words
-----------------
- ( is not conform, doesn't refill the input buffer; ) must be on the same line.
- INCLUDE-FILE is not implemented.
- SOURCE-ID is not implemented.

File-Access extension words
---------------------------
- FILE-STATUS, FLUSH-FILE, INCLUDE are implemented.
- Other words aren't.

Added words
-----------
- HELP and MAN which are synonyms.
- FOPEN
- FGETC
- FGETS
- FPUTC
- FPUTS
- FCLOSE
- ERRNO

