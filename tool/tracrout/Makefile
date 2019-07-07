include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem

SRCS = \
	tracrout.c \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: tracrout.prg

tracrout.prg: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

release::
	$(MKDIR) $(RELEASE_DIR)/sting/tools
	cp -a tracrout.prg tracrout.rsc $(RELEASE_DIR)/sting/tools
