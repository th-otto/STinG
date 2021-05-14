include ../configvars

STG = sting.stg \
	chapter/config.sti \
	chapter/faq.sti \
	chapter/inetd.sti \
	chapter/intro.sti \
	chapter/modem.sti \
	chapter/module.sti \
	chapter/program.sti \
	chapter/tcp_ip.sti \
	$(empty)

all:: sting.hyp

sting.hyp: $(STG)
	$(HCP) -o $@ $<

clean::
	$(RM) *.hyp *.ref

release::
	$(MKDIR) $(RELEASE_DIR)/doc
	cp -a sting.hyp sting.ref stingnfo.txt stfsting.txt $(RELEASE_DIR)/doc
