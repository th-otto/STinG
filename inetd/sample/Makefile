include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem
LDFLAGS += -nostartfiles

SRCS = \
	sample.c \
	$(empty)
OBJS = ismentry.o $(SRCS:.c=.o)

all:: sample.ism

sample.ism: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean::
	$(RM) *.ism

release::
	$(MKDIR) $(RELEASE_DIR)/inetd/ism
	cp -a sample.ism $(RELEASE_DIR)/inetd/ism
	$(MKDIR) $(RELEASE_DIR)/inetd/sample
	cp -a ../module.h sample.c sample.rsc sample.rso sample.rsh sample.prj ismentry.s $(RELEASE_DIR)/inetd/sample
