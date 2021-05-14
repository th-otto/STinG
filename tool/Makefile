include ../configvars

SUBDIRS = ping save_ip shutdown sysinfo tracrout

all clean distclean::
	@for i in $(SUBDIRS); do $(MAKE) -C $$i $@ || exit 1; done

release::
	$(MKDIR) $(RELEASE_DIR)/tools
	cp -a tools.txt $(RELEASE_DIR)/tools
	@for i in $(SUBDIRS); do $(MAKE) -C $$i $@ || exit 1; done
