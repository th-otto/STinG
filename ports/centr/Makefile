include ../../configvars

CFLAGS += -I../../include

SRCS = \
	centr.c \
	$(empty)
OBJS = $(SRCS:.c=.o) xfer.o

all:: centr.stx

centr.stx: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean distclean::
	$(RM) *.stx

release::
	$(MKDIR) $(RELEASE_DIR)/sting
	cp -a centr.stx $(RELEASE_DIR)/sting/centr.sty
