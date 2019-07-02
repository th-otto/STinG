include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem
LDFLAGS += -nostartfiles

SRCS = \
	timed.c \
	$(empty)
OBJS = ismentry.o $(SRCS:.c=.o)

all:: timed.ism

timed.ism: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean::
	$(RM) *.ism

release::
	$(MKDIR) $(RELEASE_DIR)/inetd/ism
	cp -a timed.ism $(RELEASE_DIR)/inetd/ism
