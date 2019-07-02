include ../../configvars

CFLAGS += -I../../include
LDFLAGS += -nostartfiles
LIBS += -lgem

SRCS = \
	sting.c \
	$(empty)
OBJS = cpxstart.o $(SRCS:.c=.o)

all:: sting.cpx

sting.cpx: sting.hdr sting.bin
	cat $^ > $@

sting.bin: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean::
	$(RM) *.cpx *.bin

release::
	$(MKDIR) $(RELEASE_DIR)/cpx
	cp -a sting.cpx $(RELEASE_DIR)/cpx
