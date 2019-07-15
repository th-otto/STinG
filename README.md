# STiNG TCP/IP stack for Atari computers running TOS

This is the source of the version 1.26 of the STinG
drivers and utilties, originally developed by
Peter Rottengatter and later enhanced by Ulf Ronald Andersson.

The sources in this branch have been reorganized and cleaned
up a bit, with a few obvious bug fixes, but no major
enhancements. They are primary of historic interest, there
is no major development planned.

Original sources, as found in the 1.20 version and updated
to version 1.26, can be found on the [original branch](https://github.com/th-otto/STinG/tree/orig).

Note that this branch also contains Makefiles for use
with a gcc cross-compiler toolchain. These are currently only
useful to detect some potential problems not detected by
Pure-C, but won't produce working executables (the assembler
interfaces haven't yet been cleaned up, and still expect
Pure-C calling convention using register variables).

When recompiling the StinG kernel, don't forget to set the memory
protection flags in the program header to global (value $17 at offset
$19). Unfortunately, the Pure-C linker has no option to set this flags,
so you have to use some external tool for that. Modules are not
affected by this, since all allocation is routed to the kernel.
