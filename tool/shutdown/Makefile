include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem

SRCS = \
	shutdown.c \
	$(empty)
OBJS = $(SRCS:.c=.o) shut.o

all:: shutdown.prg

shutdown.prg: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

release::
	$(MKDIR) $(RELEASE_DIR)/sting/tools
	cp -a shutdown.prg $(RELEASE_DIR)/sting/tools
