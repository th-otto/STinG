include ../configvars

SUBDIRS = ftp http telnet time sample

CFLAGS += -I../include
LIBS += -lgem

SRCS = \
	config.c \
	func.c \
	ismapi.c \
	main.c \
	window.c \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: inetd.app

inetd.app: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

all clean distclean::
	@for i in $(SUBDIRS); do $(MAKE) -C $$i $@ || exit 1; done

release::
	$(MKDIR) $(RELEASE_DIR)/inetd
	cp -a inetd.app inetd.rsc inetd.srv readme.txt $(RELEASE_DIR)/inetd
	echo "C:\\STING\\INETD\\ISM" > $(RELEASE_DIR)/inetd/inetd.inf
	$(MKDIR) $(RELEASE_DIR)/inetd/ism
	for i in $(SUBDIRS); do $(MAKE) -C $$i $@ || exit 1; done
