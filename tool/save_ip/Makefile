include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem

SRCS = \
	save_ip.c \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: save_ip.gtp

save_ip.gtp: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

release::
	$(MKDIR) $(RELEASE_DIR)/sting/tools
	cp -a save_ip.gtp $(RELEASE_DIR)/sting/tools
