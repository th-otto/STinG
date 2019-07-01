include ../../configvars

CFLAGS += -I../../include

SRCS = \
	cnfsting.c \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: cnfsting.prg

cnfsting.prg: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)
