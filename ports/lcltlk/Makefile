include ../../configvars

CFLAGS += -I../../include

SRCS = \
	lcltlk.c \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: lcltlk.stx

lcltlk.stx: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean distclean::
	$(RM) *.stx

release::
	$(MKDIR) $(RELEASE_DIR)/sting
	cp -a lcltlk.stx $(RELEASE_DIR)/sting
