include ../../configvars

CFLAGS += -I../../include

SRCS = \
	$(empty)
OBJS = masque.o

# current not compilable by gas
#all:: masque.stx

all:: masque.hyp

masque.stx: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

masque.hyp: masque.stg
	$(HCP) -o $@ $<

clean distclean::
	$(RM) *.stx *.hyp *.ref

release::
	$(MKDIR) $(RELEASE_DIR)/sting
	cp -a masque.stx $(RELEASE_DIR)/sting
	$(MKDIR) $(RELEASE_DIR)/doc
	cp -a masque.hyp masque.txt $(RELEASE_DIR)/doc
