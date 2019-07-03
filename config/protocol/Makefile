include ../../configvars

CFLAGS += -I../../include
LDFLAGS += -nostartfiles
LIBS += -lgem

SRCS = \
	stngprot.c \
	$(empty)
OBJS = cpxstart.o $(SRCS:.c=.o)

all:: stngprot.cpx

stngprot.cpx: stngprot.hdr stngprot.bin
	cat $^ > $@

stngprot.hdr: stngprot.cph
	$(PERL) ../genhdr.pl -o $@ $<

stngprot.bin: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean::
	$(RM) *.cpx *.bin *.hdr

release::
	$(MKDIR) $(RELEASE_DIR)/cpx
	cp -a stngprot.cpx $(RELEASE_DIR)/cpx
