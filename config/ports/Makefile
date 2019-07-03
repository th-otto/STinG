include ../../configvars

CFLAGS += -I../../include
LDFLAGS += -nostartfiles
LIBS += -lgem

SRCS = \
	stngport.c \
	$(empty)
OBJS = cpxstart.o $(SRCS:.c=.o)

all:: stngport.cpx

stngport.cpx: stngport.hdr stngport.bin
	cat $^ > $@

stngport.hdr: stngport.cph
	$(PERL) ../genhdr.pl -o $@ $<

stngport.bin: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean::
	$(RM) *.cpx *.bin *.hdr

release::
	$(MKDIR) $(RELEASE_DIR)/cpx
	cp -a stngport.cpx $(RELEASE_DIR)/cpx
