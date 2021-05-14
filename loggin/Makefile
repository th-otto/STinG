include ../configvars

SUBDIRS = logctrl logsting

all clean distclean::
	@for i in $(SUBDIRS); do $(MAKE) -C $$i $@ || exit 1; done

release::
	$(MKDIR) $(RELEASE_DIR)/dev-kit/logsting
	cp -a README.TXT $(RELEASE_DIR)/dev-kit/logsting
	@for i in $(SUBDIRS); do $(MAKE) -C $$i $@ || exit 1; done
