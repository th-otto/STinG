include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem

SRCS = \
	logctrl.c \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: logctrl.prg

logctrl.prg: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

release::
	$(MKDIR) $(RELEASE_DIR)/dev-kit/logsting
	cp -a logctrl.prg ctrl.rsc $(RELEASE_DIR)/dev-kit/logsting
