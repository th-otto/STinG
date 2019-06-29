include ../../configvars

CFLAGS += -I../../include

SRCS = \
	sysinfo.c \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: sysinfo.ttp

sysinfo.ttp: $(OBJS)
	$(CC) -o $@ $(OBJS)
