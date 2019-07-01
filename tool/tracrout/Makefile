include ../../configvars

CFLAGS += -I../../include

SRCS = \
	tracrout.c \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: tracrout.prg

tracrout.prg: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
