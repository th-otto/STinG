#ifdef __PUREC__
#include <aes.h>
#ifndef _WORD
#define _WORD int
#endif
#else
#include <gem.h>
#ifndef _WORD
#define _WORD short
#endif
#endif

int main(void)
{
	_WORD our_apid, dial_apid, message[8];

	our_apid = appl_init();

	if ((dial_apid = appl_find("DIALER  ")) >= 0)
	{
		message[0] = AC_OPEN;
		message[1] = our_apid;
		message[2] = 0;
		appl_write(dial_apid, 16, message);
	}

	appl_exit();
	return 0;
}
