include ../../configvars

CFLAGS += -I../../include
LDFLAGS += -nostartfiles
LIBS += -lgem

SRCS = \
	stngprot.c \
	$(empty)
OBJS = cpxstart.o $(SRCS:.c=.o)

cpxstart.o: cpxstart.s
	$(AS) $(CFLAGS) -c -o $@ $<

all:: stngprot.cpx

stngprot.cpx: stngprot.hdr stngprot.bin
	cat $^ > $@

stngprot.bin: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean::
	$(RM) *.cpx *.bin
