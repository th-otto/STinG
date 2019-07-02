include ../../configvars

CFLAGS += -I../../include
LDFLAGS += -nostartfiles
LIBS += -lgem

SRCS = \
	serial.c \
	$(empty)
OBJS = cpxstart.o $(SRCS:.c=.o)

all:: serial.cpx

serial.cpx: serial.hdr serial.bin
	cat $^ > $@

serial.bin: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean::
	$(RM) *.cpx *.bin

release::
	$(MKDIR) $(RELEASE_DIR)/cpx
	cp -a serial.cpx $(RELEASE_DIR)/cpx
	cp -a README $(RELEASE_DIR)/cpx/serial.txt
