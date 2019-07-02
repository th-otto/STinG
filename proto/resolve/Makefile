include ../../configvars

CFLAGS += -I../../include
LIBS += -lgem

SRCS = \
	resolve.c \
	query.c \
	cache.c \
	$(empty)
OBJS = $(SRCS:.c=.o) reso_sub.o yield.o

all:: resolve.stx base.prg read.prg

resolve.stx: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

base.prg: base.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

read.prg: read.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean distclean::
	$(RM) *.stx

release::
	$(MKDIR) $(RELEASE_DIR)/sting
	cp -a resolve.stx $(RELEASE_DIR)/sting
