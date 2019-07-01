include ../configvars

CFLAGS += -I../include

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

all:: sting.prg

sting.prg: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
