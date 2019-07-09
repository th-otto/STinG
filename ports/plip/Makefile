include ../../configvars

CFLAGS += -I../../include

SRCS = \
	$(empty)
OBJS = cen_plip.o

# currently not compilable by gas
#all:: cen_plip.stx cen_plep.stx

all:: cen_plip.hyp

cen_plip.stx: cen_plip.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

cen_plep.stx: cen_plep.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

cen_plip.hyp: cen_plip.stg
	$(HCP) -o $@ $<

clean distclean::
	$(RM) *.stx *.hyp *.ref

release::
	$(MKDIR) $(RELEASE_DIR)/sting
	cp -a cen_plip.stx $(RELEASE_DIR)/sting/cen_plip.sty
	cp -a cen_plep.stx $(RELEASE_DIR)/sting/cen_plep.sty
	$(MKDIR) $(RELEASE_DIR)/doc
	cp -a cen_plip.hyp $(RELEASE_DIR)/doc
