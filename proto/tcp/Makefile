include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem

SRCS = \
	tcp.c \
	arrive.c \
	inout.c \
	tool.c \
	$(empty)
OBJS = $(SRCS:.c=.o) check.o

all:: tcp.stx

tcp.stx: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean distclean::
	$(RM) *.stx

release::
	$(MKDIR) $(RELEASE_DIR)/sting
	cp -a tcp.stx $(RELEASE_DIR)/sting
