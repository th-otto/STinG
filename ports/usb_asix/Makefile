#
# Makefile for usb_asix.stx
#

#LIBS = libcmini.a
CC = m68k-atari-mint-gcc
LD = $(CC)
CPPFLAGS = 
CFLAGS = $(CPPFLAGS) -m68000 -mshort -fno-builtin -O2 -Wall -Wundef -Wold-style-definition -fomit-frame-pointer -I../../include
LDFLAGS = -s

PROGRAMS = 
PROGRAMS += usb_asix.stx
PROGRAMS += uatool.ttp

all:  $(PROGRAMS)

OBJS = 
HEADERS = 

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

usb_asix.stx: init.o usbsting.o arpcache.o asix.o utility.o
	$(LD) $(CFLAGS) $^ -nostartfiles -nostdlib $(LDFLAGS) -o $@

uatool.ttp: uatool.o utility.o
	$(LD) uatool.o $(LDFLAGS) -o $@

clean:
	-rm -f $(PROGRAMS) *.o
