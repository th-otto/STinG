SUBDIRS = include config sting tool loggin inetd docu proto ports dialer

all clean distclean::
	@for i in $(SUBDIRS); do $(MAKE) -C $$i $@ || exit 1; done

clean allclean::
	$(RM) *.pdb

release::
	@export RELEASE_DIR=`pwd´/release; \
	$(MKDIR) $(RELEASE_DIR); \
	cp -a history.txt readme.1st $(RELEASE_DIR); \
	for i in $(SUBDIRS); do $(MAKE) -C $$i $@ || exit 1; done
