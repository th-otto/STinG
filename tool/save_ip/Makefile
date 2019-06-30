include ../../configvars

CFLAGS += -I../../include

SRCS = \
	save_ip.c \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: save_ip.prg

save_ip.prg: $(OBJS)
	$(CC) -o $@ $(OBJS) -lgem
