include ../../configvars

CFLAGS += -I../../include

SRCS = \
	$(empty)
OBJS = cen_plip.o

# current not compilable by gas
#all:: cen_plip.stx

all:: cen_plip.hyp

cen_plip.stx: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

cen_plip.hyp: cen_plip.stg
	$(HCP) -o $@ $<

clean distclean::
	$(RM) *.stx *.hyp *.ref

release::
	$(MKDIR) $(RELEASE_DIR)/sting
	cp -a cen_plip.stx $(RELEASE_DIR)/sting
	$(MKDIR) $(RELEASE_DIR)/doc
	cp -a cen_plip.hyp $(RELEASE_DIR)/doc
