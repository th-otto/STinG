include ../../configvars

CFLAGS += -I../../include
LDFLAGS += -nostartfiles
LIBS += -lgem

SRCS = \
	stngport.c \
	$(empty)
OBJS = cpxstart.o $(SRCS:.c=.o)

cpxstart.o: cpxstart.s
	$(AS) $(CFLAGS) -c -o $@ $<

all:: stngport.cpx

stngport.cpx: stngport.hdr stngport.bin
	cat $^ > $@

stngport.bin: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean::
	$(RM) *.cpx *.bin
