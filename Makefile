SUBDIRS = config sting tool

all clean distclean::
	@for i in $(SUBDIRS); do $(MAKE) -C $$i $@ || exit 1; done
