include ../../configvars

CFLAGS += -I../../include

SRCS = \
	serial.c \
	compress.c \
	xfer.c \
	p_autom.c \
	p_misc.c \
	p_xcp.c \
	$(empty)
OBJS = $(SRCS:.c=.o) calls.o

all:: serial.stx

serial.stx: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean distclean::
	$(RM) *.stx

release::
	$(MKDIR) $(RELEASE_DIR)/sting
	cp -a serial.stx $(RELEASE_DIR)/sting/serial.sty
	$(MKDIR) $(RELEASE_DIR)/doc
	cp -a ser_stx.txt $(RELEASE_DIR)/doc
