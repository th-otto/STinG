include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem

SRCS = \
	udp.c \
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
