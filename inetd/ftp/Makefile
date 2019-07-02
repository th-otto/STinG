include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem
LDFLAGS += -nostartfiles

SRCS = \
	ftpd.c \
	$(empty)
OBJS = ismentry.o $(SRCS:.c=.o)

all:: ftpd.ism

ftpd.ism: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean::
	$(RM) *.ism

release::
	$(MKDIR) $(RELEASE_DIR)/inetd/ism
	cp -a ftpd.ism $(RELEASE_DIR)/inetd/ism
