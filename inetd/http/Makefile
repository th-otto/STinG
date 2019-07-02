include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem
LDFLAGS += -nostartfiles

SRCS = \
	httpd.c \
	$(empty)
OBJS = ismentry.o $(SRCS:.c=.o)

all:: httpd.ism

httpd.ism: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean::
	$(RM) *.ism

release::
	$(MKDIR) $(RELEASE_DIR)/inetd/ism
	cp -a httpd.ism $(RELEASE_DIR)/inetd/ism
