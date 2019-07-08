include ../../configvars

CFLAGS += -I../../include

SRCS = \
	ether.c \
	tools.c \
	$(empty)
OBJS = $(SRCS:.c=.o) hard.o

all:: ether.stx

ether.stx: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean distclean::
	$(RM) *.stx

release::
	$(MKDIR) $(RELEASE_DIR)/sting
	cp -a ether.stx $(RELEASE_DIR)/sting/ether.sty
