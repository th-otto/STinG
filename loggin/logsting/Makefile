include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem

SRCS = \
	install.c \
	api.c \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: logsting.prg

logsting.prg: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
