SUBDIRS = centr ether lcltlk masque midi

all clean distclean::
	@for i in $(SUBDIRS); do $(MAKE) -C $$i $@ || exit 1; done

release::
	@for i in $(SUBDIRS); do $(MAKE) -C $$i $@ || exit 1; done
