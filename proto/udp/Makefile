include ../../configvars

CFLAGS += -I../../include

SRCS = \
	udp.c \
	applyld.c \
	$(empty)
OBJS = $(SRCS:.c=.o) sema.o

all:: udp.stx

udp.stx: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean distclean::
	$(RM) *.stx

release::
	$(MKDIR) $(RELEASE_DIR)/sting
	cp -a udp.stx $(RELEASE_DIR)/sting
	$(MKDIR) $(RELEASE_DIR)/doc
	cp -a udp_stx.txt $(RELEASE_DIR)/doc
