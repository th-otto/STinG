include ../../configvars

CFLAGS += -I../../include

SRCS = \
	ping.c \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: ping.prg

ping.prg: $(OBJS)
	$(CC) -o $@ $(OBJS) -lgem
