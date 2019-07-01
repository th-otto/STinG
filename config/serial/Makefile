include ../../configvars

CFLAGS += -I../../include
LDFLAGS += -nostartfiles
LIBS += -lgem

SRCS = \
	serial.c \
	$(empty)
OBJS = cpxstart.o $(SRCS:.c=.o)

cpxstart.o: cpxstart.s
	$(AS) $(CFLAGS) -c -o $@ $<

all:: serial.cpx

serial.cpx: serial.hdr serial.bin
	cat $^ > $@

serial.bin: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean::
	$(RM) *.cpx *.bin
