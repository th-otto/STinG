/*********************************************************************/
/*                                                                   */
/*     STinG : API and IP kernel package                             */
/*                                                                   */
/*                                                                   */
/*      Version 1.0                      from 2. Dezember 1996       */
/*                                                                   */
/*      Module for Port Installation and Handling                    */
/*                                                                   */
/*********************************************************************/


#include <tos.h>
#include <stdio.h>
#include <string.h>

#include "globdefs.h"


#define  M_YEAR    1996
#define  M_MONTH   12
#define  M_DAY     2
#define  M_VERSION "01.00"

static char const internal[] = "Internal";

PORT my_port = {
	internal,
	L_INTERNAL,
	TRUE,
	0L,
	LOOPBACK,
	0xffffffffUL,
	32768U,
	32768U,
	0,
	NULL,
	0,
	NULL,
	0,
	NULL,
	NULL
};

static PORT *search_port(const char *port_name)
{
	PORT *walk;

	for (walk = conf.ports; walk; walk = walk->next)
		if (strcmp(walk->name, port_name) == 0)
			return walk;

	return NULL;
}


int16 cdecl on_port(const char *port_name)
{
	PORT *this;

	if ((this = search_port(port_name)) == NULL)
		return FALSE;

	if (this->active)
		return TRUE;

	if ((*this->driver->set_state) (this, TRUE) == FALSE)
		return FALSE;

	this->active = TRUE;
	this->stat_sd_data = this->stat_rcv_data = this->stat_dropped = 0;

	return TRUE;
}


void cdecl off_port(const char *port_name)
{
	PORT *this;

	if ((this = search_port(port_name)) == NULL)
		return;

	if (!this->active)
		return;

	(*this->driver->set_state) (this, FALSE);

	this->active = FALSE;
}


int16 cdecl query_port(const char *port_name)
{
	PORT *this;

	if (port_name == NULL)
		return FALSE;

	if ((this = search_port(port_name)) == NULL)
		return FALSE;

	return this->active;
}


int16 cdecl cntrl_port(const char *port_name, uint32 argument, int16 code)
{
	PORT *this;
	PNTA *act_pnta;
	int16 result = E_NORMAL;

	if (port_name == NULL)
	{
		switch (code)
		{
		case CTL_KERN_FIRST_PORT:
			(act_pnta = (PNTA *) argument)->opaque = conf.ports;
			strncpy(act_pnta->port_name, ((PORT *)act_pnta->opaque)->name, act_pnta->name_len);
			return E_NORMAL;
		case CTL_KERN_NEXT_PORT:
			act_pnta = (PNTA *) argument;
			if ((act_pnta->opaque = ((PORT *)act_pnta->opaque)->next) == NULL)
				return E_NODATA;
			strncpy(act_pnta->port_name, ((PORT *)act_pnta->opaque)->name, act_pnta->name_len);
			return E_NORMAL;
		}
		return E_FNAVAIL;
	}

	if ((this = search_port(port_name)) == NULL)
		return E_NODATA;

	switch (code)
	{
	case CTL_KERN_FIND_PORT:
		*((PORT **) argument) = this;
		break;
	case CTL_GENERIC_GET_IP:
		*((uint32 *) argument) = this->ip_addr;
		break;
	case CTL_GENERIC_GET_MASK:
		*((uint32 *) argument) = this->sub_mask;
		break;
	case CTL_GENERIC_GET_MTU:
		*((int16 *) argument) = this->mtu;
		break;
	case CTL_GENERIC_GET_MMTU:
		*((int16 *) argument) = this->max_mtu;
		break;
	case CTL_GENERIC_GET_TYPE:
		*((int16 *) argument) = this->type;
		break;
	case CTL_GENERIC_GET_STAT:
		((int32 *) argument)[0] = this->stat_dropped;
		((int32 *) argument)[1] = this->stat_sd_data;
		((int32 *) argument)[2] = this->stat_rcv_data;
		break;
	case CTL_GENERIC_CLR_STAT:
		this->stat_sd_data = this->stat_rcv_data = this->stat_dropped = 0;
		break;
	default:
		result = (*this->driver->cntrl) (this, argument, code);
	}

	if (result == E_FNAVAIL)
	{
		switch (code)
		{
		case CTL_GENERIC_SET_MTU:
			if ((int32)argument > this->max_mtu)
				argument = this->max_mtu;
			if (argument < 68)
				argument = 68;
			this->mtu = (int16) argument;
			result = E_NORMAL;
			break;
		case CTL_GENERIC_SET_IP:
			this->ip_addr = argument;
			result = E_NORMAL;
			break;
		case CTL_GENERIC_SET_MASK:
			this->sub_mask = argument;
			result = E_NORMAL;
			break;
		}
	}

	return result;
}


static int16 cdecl my_set_state(PORT *port, int16 state)
{
	UNUSED(port);
	UNUSED(state);
	return TRUE;
}


static int16 cdecl my_cntrl(PORT *port, uint32 argument, int16 code)
{
	UNUSED(port);
	UNUSED(argument);
	UNUSED(code);
	return E_FNAVAIL;
}


static DRIVER my_driver = {
	my_set_state,
	my_cntrl,
	my_send,
	my_receive,
	internal,
	M_VERSION,
	((M_YEAR - 1980) << 9) | (M_MONTH << 5) | M_DAY,
	"Peter Rottengatter",
	NULL,
	NULL
};


void init_ports(void)
{
	my_driver.basepage = _BasPag;
	my_port.driver = &my_driver;

	conf.ports = &my_port;
	conf.drivers = &my_driver;
}
