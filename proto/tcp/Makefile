include ../../configvars

CFLAGS += -I../../include

SRCS = \
	tcp.c \
	arrive.c \
	inout.c \
	tool.c \
	applyld.c \
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
	$(MKDIR) $(RELEASE_DIR)/doc
	cp -a tcp_stx.txt $(RELEASE_DIR)/doc
