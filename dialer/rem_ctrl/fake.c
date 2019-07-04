/*********************************************************************/
/*                                                                   */
/*     This program pretents being a Dialer server, so that it       */
/*      can be connected to by a dial client. It's there to show     */
/*      how the remote control dialing facility works.               */
/*                                                                   */
/*     (c) Peter Rottengatter              from 16. Januar 1997      */
/*                                                                   */
/*********************************************************************/


#include <aes.h>
#include <tos.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "transprt.h"
#include "remctrl.h"


#define  TIMEOUT   60


TPL *tpl;

static char alert[200];
static char const not_there[] = "[1][ |  STinG is not loaded or enabled !   ][ Hmmm ]";
static char const corrupted[] = "[1][ |  STinG structures corrupted !   ][ Oooops ]";
static char const found_it[] = "[3][ |  Driver \'%s\',|  by %s, found,   |  version %s.][ Okay ]";
static char const no_module[] = "[1][ |  STinG Transport Driver not found !   ][ Grmbl ]";
static char const query[] = "[3][ |  Dialing request using script file   |    \'%s\',|    up to %d tries !   ][ Done | Error ]";
static char const number[] = "[2][ |  Parameter is %d. Wanna change it ?   ][ < | Okay | > ]";
static char const hangup[] = "[3][ |  Hang up request received !   ][ Okay ]";
static char const proceed[] = "[2][ |  Proceed faking a dialer ?   ][ Proceed | Abort ]";
static char const no_udp[] = "[1][ |  Couldn't open UDP connection !  ][ Hmm ]";



static long get_sting_cookie(void)
{
	long *work;

	work = *(long **) 0x5a0L;
	if (work == 0)
		return 0;
	for (; *work != 0L; work += 2)
		if (*work == STIK_COOKIE_MAGIC)
			return *++work;

	return 0;
}


static void do_some_work(void)
{
	NDB *ndb;
	time_t timeout;
	_WORD handle;
	short message[2];
	_WORD response;
	_WORD ready;

	if ((handle = UDP_open(0, IP_DIALER_PORT)) < 0)
	{
		form_alert(1, no_udp);
		return;
	}

	for (;;)
	{

		timeout = time(NULL) + TIMEOUT;

		do
		{
			evnt_timer(200);

			if ((ndb = CNget_NDB(handle)) != NULL)
			{
				switch (*(int16 *) ndb->ndata)
				{
				case IP_DIAL_REQUEST:
					sprintf(alert, query, ndb->ndata + 4, *(int16 *) (ndb->ndata + 2));
					if (form_alert(1, alert) == 1)
					{
						message[0] = IP_DIAL_DONE, message[1] = 0;
					} else
					{
						message[0] = IP_DIAL_ERROR;
						response = 0;
						do
						{
							sprintf(alert, number, response);
							ready = 0;
							switch (form_alert(1, alert))
							{
							case 1:
								response--;
								break;
							case 2:
								ready = 1;
								break;
							case 3:
								response++;
								break;
							}
						} while (!ready);
						message[1] = response;
					}
					break;
				case IP_DIAL_HANGUP:
					form_alert(1, hangup);
					message[0] = IP_DIAL_DONE;
					message[1] = 0;
					break;
				}
				KRfree(ndb->ptr);
				KRfree(ndb);
				UDP_send(handle, (char *) message, 4);
				UDP_close(handle);

				if ((handle = UDP_open(0, IP_DIALER_PORT)) < 0)
				{
					form_alert(1, no_udp);
					return;
				}
			}
		} while (time(NULL) < timeout);

		if (form_alert(1, proceed) != 1)
			break;
	}

	UDP_close(handle);
}


static void gem_program(void)
{
	DRV_LIST *sting_drivers;

	sting_drivers = (DRV_LIST *) Supexec(get_sting_cookie);

	if (sting_drivers == 0)
	{
		form_alert(1, not_there);
		return;
	}
	if (strcmp(sting_drivers->magic, STIK_DRVR_MAGIC) != 0)
	{
		form_alert(1, corrupted);
		return;
	}

	tpl = (TPL *) (*sting_drivers->get_dftab) (TRANSPORT_DRIVER);

	if (tpl != NULL)
	{
		sprintf(alert, found_it, tpl->module, tpl->author, tpl->version);
		form_alert(1, alert);
		do_some_work();
	} else
	{
		form_alert(1, no_module);
	}
}


int main(void)
{
	gl_apid = appl_init();

	gem_program();

	appl_exit();
	return 0;
}
