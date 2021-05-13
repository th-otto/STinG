include ../../configvars

CFLAGS += -I../../include

SRCS = \
	nfeth.c \
	tools.c \
	$(empty)
OBJS = $(SRCS:.c=.o) nfethasm.o

all:: nfeth.stx

nfeth.stx: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean distclean::
	$(RM) *.stx

release::
	$(MKDIR) $(RELEASE_DIR)/sting
	cp -a nfeth.stx $(RELEASE_DIR)/sting/nfeth.sty
