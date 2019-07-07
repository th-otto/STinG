include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem

SRCS = \
	ping.c \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: ping.prg

ping.prg: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

release::
	$(MKDIR) $(RELEASE_DIR)/sting/tools
	cp -a ping.prg ping.rsc $(RELEASE_DIR)/sting/tools
	$(MKDIR) $(RELEASE_DIR)/dev-kit
	cp -a ping.c $(RELEASE_DIR)/dev-kit
