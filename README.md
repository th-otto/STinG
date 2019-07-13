# STiNG TCP/IP stack for Atari computers running TOS

This is the source of the version 1.26 of the STinG
drivers and utilties, originally developed by
Peter Rottengatter and later enhanced by Ulf Ronald Andersson.

The sources in this branch should compile to binary
identical versions of the released archives, provided
that you have the needed Pure-C libraries
(there are slightly different version of the standard libraries,
most programs seem to be linked against a PCSTDLIB.LIB
from Sep 4. 1992 with 24962 bytes).

This version was created taking the original sources of
the 1.20 version, then updating it to 1.26 by comparing
the binaries of newer versions. Some modules (e.g. tcp.stx)
were updated after the 1.26 release, and are contained here, too.
