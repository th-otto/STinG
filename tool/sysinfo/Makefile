include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem

SRCS = \
	sysinfo.c \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: sysinfo.tos

sysinfo.tos: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

release::
	$(MKDIR) $(RELEASE_DIR)/sting/tools
	cp -a sysinfo.tos $(RELEASE_DIR)/sting/tools
