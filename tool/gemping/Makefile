CROSS = m68k-atari-mint-
CC = $(CROSS)gcc
CFLAGS = -O2 -fomit-frame-pointer -Wall -W -Werror

PROGRAMS = ping

ifneq ($(CROSS),)
NET_OBJS = inet.o gethost.o
PROGRAMS += gemping.prg
PROGRAMS += sntp.prg
endif

all: $(PROGRAMS)

ping: ping.o $(NET_OBJS)
	$(CC) $(CFLAGS) -Wl,-Map,ping.map -s -o $@ $^ -lm

gemping.prg: gemping.o $(NET_OBJS)
	$(CC) $(CFLAGS) -s -o $@ $^ -lm -lgem

sntp.prg: sntp.o $(NET_OBJS)
	$(CC) $(CFLAGS) -s -o $@ $^ -lm -lgem

ping.o gemping.o: icmp.h

clean::
	rm -f *.o *.pdb *.prg *.ttp *.map $(PROGRAMS)
