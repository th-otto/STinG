include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem

SRCS = \
	sysinfo.c \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: sysinfo.ttp

sysinfo.ttp: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

release::
	$(MKDIR) $(RELEASE_DIR)/sting/tools
	cp -a sysinfo.ttp $(RELEASE_DIR)/sting/tools
