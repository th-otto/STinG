include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem

SRCS = \
	ping.c \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: ping.prg

ping.prg: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
