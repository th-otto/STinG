include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem

SRCS = \
	shutdown.c \
	shut.s \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: shutdown.prg

shutdown.prg: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

release::
	$(MKDIR) $(RELEASE_DIR)/tools
	cp -a shutdown.prg $(RELEASE_DIR)/tools
