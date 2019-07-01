SUBDIRS = ping save_ip shutdown sysinfo tracrout

all clean distclean::
	@for i in $(SUBDIRS); do $(MAKE) -C $$i $@ || exit 1; done
