include ../../configvars

CFLAGS += -I../../include

SRCS = \
	shutdown.c \
	shut.s \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: shutdown.prg

shutdown.prg: $(OBJS)
	$(CC) -o $@ $(OBJS) -lgem
