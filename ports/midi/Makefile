include ../../configvars

CFLAGS += -I../../include

SRCS = \
	midi.c \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: midi.stx

midi.stx: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean distclean::
	$(RM) *.stx

release::
	$(MKDIR) $(RELEASE_DIR)/sting
	cp -a midi.stx $(RELEASE_DIR)/sting
	$(MKDIR) $(RELEASE_DIR)/doc
	cp -a modslip.txt $(RELEASE_DIR)/doc
