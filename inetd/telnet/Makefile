include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem
LDFLAGS += -nostartfiles

SRCS = \
	telnetd.c \
	$(empty)
OBJS = ismentry.o $(SRCS:.c=.o)

ismentry.o: ismentry.s
	$(AS) $(CFLAGS) -c -o $@ $<

all:: telnetd.ism

telnetd.ism: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean::
	$(RM) *.ism

release::
	$(MKDIR) $(RELEASE_DIR)/inetd/ism
	cp -a telnetd.ism $(RELEASE_DIR)/inetd/ism
