CROSSPREFIX=m68k-atari-mint-

CC=$(CROSSPREFIX)gcc
WARN=-Wall -W -Wstrict-prototypes -Wmissing-prototypes -Werror
CFLAGS= -O2 -fomit-frame-pointer $(WARN) -I../include

SRCS = \
	api.c \
	fragm.c \
	icmp.c \
	install.c \
	ip.c \
	kernel.c \
	port.c \
	protocol.c \
	setup.c \
	$(empty)
OBJS = $(SRCS:.c=.o)

all: sting.prg

sting.prg: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean::
	$(RM) *.o *.prg *.pdb
