include ../configvars

CFLAGS += -I../include

SRCS = \
	api.c \
	fragm.c \
	icmp.c \
	install.c \
	ip.c \
	kernel.c \
	port.c \
	protocol.c \
	setup.c \
	$(empty)
OBJS = $(SRCS:.c=.o) thread.o

all:: sting.prg

sting.prg: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

release::
	$(MKDIR) $(RELEASE_DIR)/auto
	$(MKDIR) $(RELEASE_DIR)/sting
	cp -a sting.prg $(RELEASE_DIR)/auto
	echo "C:\\STING\\" > $(RELEASE_DIR)/auto/sting.inf
	cp -a default.cfg route.tab update.txt $(RELEASE_DIR)/sting
