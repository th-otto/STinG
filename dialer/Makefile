include ../configvars

CFLAGS += -I../include
LIBS += -lgem

DEVICE_OBJS = \
	device/dev_lib.c \
	device/dev_misc.c \
	device/port.c \
	$(empty)

SRCS = \
	config.c \
	dial.c \
	func.c \
	loggin.c \
	main.c \
	tool.c \
	window.c \
	$(DEVICE_OBJS) \
	$(empty)
OBJS = $(SRCS:.c=.o)

all:: dialer.app device/dev_test.tos call_it/call_it.prg rem_ctrl/fake.prg rem_ctrl/request.prg

dialer.app: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

device/dev_test.tos: device/dev_test.o $(DEVICE_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

call_it/call_it.prg: call_it/call_it.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

rem_ctrl/fake.prg: rem_ctrl/fake.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

rem_ctrl/request.prg: rem_ctrl/request.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean distclean::
	$(RM) device/*.o device/*.pdb device/*.lib device/*.tos call_it/*.o call_it/*.prg call_it/*.pdb rem_ctrl/*.o rem_ctrl/*.prg rem_ctrl/*.pdb

release::
	$(MKDIR) $(RELEASE_DIR)/sting/dialer
	cp -a dialer.app dialer.rsc others/dial.inf others/dial.scr others/dial.inf others/icon.rsc others/login.bat others/local.fee $(RELEASE_DIR)/sting/dialer
	$(MKDIR) $(RELEASE_DIR)/sting/dialer/docs
	cp -a others/fee.doc rem_ctrl/rem_ctrl.txt others/history.txt $(RELEASE_DIR)/sting/dialer/docs
	$(MKDIR) $(RELEASE_DIR)/sting/dialer/call_it
	cp -a call_it/call_it.prg call_it/call_it.c call_it/readme $(RELEASE_DIR)/sting/dialer/call_it
	$(MKDIR) $(RELEASE_DIR)/sting/dialer/rem_ctrl
	cp -a rem_ctrl/fake.prg rem_ctrl/fake.c rem_ctrl/request.prg rem_ctrl/request.c rem_ctrl/remctrl.h $(RELEASE_DIR)/sting/dialer/rem_ctrl
