/*********************************************************************/
/*                                                                   */
/*     Low Level Port : Serielle Schnittstellen                      */
/*                                                                   */
/*                                                                   */
/*      Version 0.1                         vom 16. Juli 1997        */
/*                                                                   */
/*      Modul zum PPP State Automaton Handling                       */
/*                                                                   */
/*********************************************************************/


#include <tos.h>
#include <stdio.h>
#include <string.h>

#include "transprt.h"
#include "port.h"

#include "serial.h"





static const char *const log_text[] = {
	"???", "Conf.-Request :", "Conf.-Ack. :", "Conf.-Not-Ack. :",
	"Conf.-Reject :", "Term.-Request :", "Term.-Ack. :", "Code-Reject :"
};

static char const doing[] = "Doing";
static char const receiving[] = "Peer sends";
static uint8 cnt_init[] = { 12, 1, 12, 5, 1, 1, 12, 12, 12, 12, 1, 1, 1, 5 };



static void this_layer_up(SERIAL_PORT *port, MACHINE *machine, int16 event)
{
	UNUSED(machine);
	UNUSED(event);
	machine->up_down(port, PPP_UP);
}


static void this_layer_down(SERIAL_PORT *port, MACHINE *machine, int16 event)
{
	UNUSED(machine);
	UNUSED(event);
	machine->up_down(port, PPP_DOWN);
}


static void this_layer_start(SERIAL_PORT *port, MACHINE *machine, int16 event)
{
	UNUSED(machine);
	UNUSED(event);
	machine->do_lower(port, PPP_UP);
}


static void this_layer_fin(SERIAL_PORT *port, MACHINE *machine, int16 event)
{
	UNUSED(machine);
	UNUSED(event);
	machine->do_lower(port, PPP_DOWN);
}


static void init_restart_cnt(SERIAL_PORT *port, MACHINE *machine, int16 event)
{
	UNUSED(port);
	machine->timer_start = TIMER_now();
	machine->timer_elapsed = 2000L;
	machine->timer_run = TRUE;
	machine->restart_cnt = cnt_init[event];
}


static void zero_restart_cnt(SERIAL_PORT *port, MACHINE *machine, int16 event)
{
	UNUSED(port);
	UNUSED(event);
	machine->timer_start = TIMER_now();
	machine->timer_elapsed = 3000L;
	machine->timer_run = TRUE;
	machine->restart_cnt = 0;
}


static void send_conf_req(SERIAL_PORT *port, MACHINE *machine, int16 event)
{
	UNUSED(event);
	if (machine->conf_len == -1)
	{
		machine->create(port);
		if (machine->conf_len == -1)
			return;
	}

	ppp_log_it(port, machine, doing, log_text[PPP_CONF_REQU], identifier);
	ppp_log_options(port, machine->conf, machine->conf_len, machine->offset, machine->xtra);

	machine->identi = identifier;
	send_cp(port, machine->protocol, PPP_CONF_REQU, identifier++, machine->conf_len, machine->conf);
}


static void send_conf_ack(SERIAL_PORT *port, MACHINE *machine, int16 event)
{
	uint8 *data;
	uint8 *last;
	uint8 *walk;
	uint16 length;

	UNUSED(event);
	data = port->ppp.data;
	last = data + (length = (data[2] << 8) | data[3]);

	ppp_log_it(port, machine, doing, log_text[machine->flags & 15], data[1]);
	ppp_log_options(port, data + 4, length - 4, machine->offset, machine->xtra);

	send_cp(port, machine->protocol, machine->flags & 15, data[1], length - 4, data + 4);

	for (walk = data + 4; walk < last; walk += walk[machine->offset] + machine->xtra)
		machine->accept(port, walk);
}


static void send_conf_nack(SERIAL_PORT *port, MACHINE *machine, int16 event)
{
	UNUSED(event);
	ppp_log_it(port, machine, doing, log_text[machine->flags & 15], port->ppp.data[1]);
	ppp_log_options(port, machine->block, machine->block_len, machine->offset, machine->xtra);

	send_cp(port, machine->protocol, machine->flags & 15, port->ppp.data[1], machine->block_len, machine->block);
}


static void send_term_req(SERIAL_PORT *port, MACHINE *machine, int16 event)
{
	ppp_log_it(port, machine, doing, log_text[PPP_TERM_REQU], identifier);

	UNUSED(event);
	machine->identi = identifier;
	send_cp(port, machine->protocol, PPP_TERM_REQU, identifier++, 0, NULL);
}


static void send_term_ack(SERIAL_PORT *port, MACHINE *machine, int16 event)
{
	UNUSED(event);
	ppp_log_it(port, machine, doing, log_text[PPP_TERM_ACK], port->ppp.data[1]);

	send_cp(port, machine->protocol, PPP_TERM_ACK, port->ppp.data[1], ((port->ppp.data[2] << 8) | port->ppp.data[3]) - 4, port->ppp.data + 4);
}

#include "trantab.h"

static void do_automaton(SERIAL_PORT *port, MACHINE *machine, int16 event)
{
	TRANSITION *trans;

	machine->event = -1;

	trans = &trantab[machine->state][event];

	if (trans->first)
		trans->first(port, machine, event);
	if (trans->second)
		trans->second(port, machine, event);
	if (trans->third)
		trans->third(port, machine, event);

	machine->state = trans->new_state;

	if (trantab[machine->state][PPP_TONF].first == NULL)
		machine->timer_run = FALSE;

	if (machine->event != -1)
		do_automaton(port, machine, machine->event);
}


void user_event(SERIAL_PORT *port, int event, MACHINE *machine)
{
	port->generic.flags &= ~FLG_DCD_UP;

	do_automaton(port, machine, event);
}


void timer_event(SERIAL_PORT *port, MACHINE *machine)
{
	int16 event;

	if (--machine->restart_cnt > 0)
	{
		event = PPP_TONF;
		machine->timer_start = TIMER_now();
		ppp_log_it(port, machine, "Timeout", "(Retry)", -1);
	} else
	{
		event = PPP_TOFATAL;
		machine->timer_run = FALSE;
		ppp_log_it(port, machine, "Timeout", "(Fatal)", -1);
	}

	do_automaton(port, machine, event);
}


static void proc_options(SERIAL_PORT *port, uint8 *data, uint8 *last, MACHINE *machine)
{
	uint8 *walk;
	uint8 *block;
	uint8 *write;
	uint8 offs;
	uint8 xtra;
	int16 level;
	int16 event;
	int16 new[16];
	int16 aux;

	if ((block = KRmalloc(1024L)) == NULL)
		return;
	level = PPP_CONF_ACK;

	offs = machine->offset;
	xtra = machine->xtra;

	write = block;
	for (walk = data + 4; walk < last; walk += walk[offs] + xtra)
	{
		if ((aux = walk[offs] + xtra) == 0)
			break;
		switch (machine->process(port, walk, (uint8 *) new))
		{
		case PPP_CONF_ACK:
			break;
		case PPP_CONF_NAK:
			if (level == PPP_CONF_ACK)
			{
				write = block;
				level = PPP_CONF_NAK;
			}
			if (level == PPP_CONF_NAK)
			{
				memcpy(write, &new[1], new[0]);
				write += new[0];
			}
			break;
		case PPP_CONF_REJCT:
			if (level != PPP_CONF_REJCT)
			{
				write = block;
				level = PPP_CONF_REJCT;
			}
			if (level == PPP_CONF_REJCT)
			{
				memcpy(write, walk, aux);
				write += aux;
			}
			break;
		}
	}

	event = level == PPP_CONF_ACK ? PPP_CREQOK : PPP_CREQBAD;
	machine->block_len = (int16) (write - block);
	machine->block = block;
	machine->flags = (machine->flags & 0xfff0u) | level;

	do_automaton(port, machine, event);

	KRfree(block);
}


static void proc_notack(SERIAL_PORT *port, uint8 *data, uint8 *last, MACHINE *machine)
{
	uint8 *walk;
	uint8 *next;
	uint8 *end;
	uint8 offs;
	uint8 xtra;
	int16 length;
	int16 new[16];
	int16 aux;

	next = machine->conf;
	end = next + machine->conf_len;

	offs = machine->offset;
	xtra = machine->xtra;

	for (walk = data + 4; walk < last; walk += length)
	{
		if ((length = walk[offs] + xtra) == 0)
			break;
		if (offs != 0)
		{
			while (next[0] != walk[0] && next < end)
				next += next[offs] + xtra;
		}
		if (next >= end)
		{
			if (machine->negotiate(port, NULL, walk, (uint8 *) new))
				break;
			memcpy(end, &new[1], new[0]);
			next = end = end + new[0];
		} else
		{
			if (machine->negotiate(port, next, walk, (uint8 *) new))
				break;
			aux = next[offs] + xtra;
			memcpy(next + new[0], next + aux, end - next - aux);
			end += new[0] - aux;
			memcpy(next, &new[1], new[0]);
			next += new[0];
		}
	}

	machine->conf_len = end - machine->conf;
}


static void proc_reject(uint8 *data, uint8 *last, MACHINE *machine)
{
	uint8 *walk;
	uint8 *next;
	uint8 *end;
	uint8 offs;
	uint8 xtra;
	int16 length;

	next = machine->conf;
	end = next + machine->conf_len;

	offs = machine->offset;
	xtra = machine->xtra;

	for (walk = data + 4; walk < last; walk += length)
	{
		if ((length = walk[offs] + xtra) == 0)
			break;
		while (memcmp(next, walk, length) != 0 && next < end)
			next += next[offs] + xtra;
		if (next >= end)
		{
			break;
		} else
		{
			end -= length;
			memcpy(next, next + length, end - next);
		}
	}

	machine->conf_len = end - machine->conf;
}


void network_event(SERIAL_PORT *port, MACHINE *machine)
{
	uint8 *data;
	uint8 *walk;
	uint8 offs;
	uint8 xtra;
	uint16 length;
	uint16 flag;
	char string[10];
	char *diag;

	data = port->ppp.data;
	length = (data[2] << 8) | data[3];

	offs = machine->offset;
	xtra = machine->xtra;

	if (*data == 0 || *data > machine->codes)
	{
		diag = "unknown opcode : $...";
		number_to_string((uint32) * data, &diag[17], 2);
		ppp_log_it(port, machine, receiving, diag, -1);
		if (PPP_CODE_REJCT <= machine->codes)
			send_cp(port, machine->protocol, PPP_CODE_REJCT, identifier++, length, data);
		return;
	}

	if (*data <= PPP_CODE_REJCT)
	{
		ppp_log_it(port, machine, receiving, log_text[*data], data[1]);
		if (*data <= PPP_CONF_REJCT)
		{
			ppp_log_options(port, data + 4, length - 4, offs, xtra);
		}
	}

	switch (*data)
	{
	case PPP_CONF_REQU:
		proc_options(port, data, data + length, machine);
		break;
	case PPP_CONF_ACK:
		if (data[1] == machine->identi)
		{
			for (walk = &data[4]; walk < data + length; walk += walk[offs] + xtra)
			{
				if (machine->implement(port, walk))
					break;
			}
			do_automaton(port, machine, PPP_CACK);
		} else
		{
			port->generic.stat_dropped++;
		}
		break;
	case PPP_CONF_NAK:
		if (data[1] == machine->identi)
		{
			proc_notack(port, data, data + length, machine);
			do_automaton(port, machine, PPP_CNAK);
		} else
		{
			port->generic.stat_dropped++;
		}
		break;
	case PPP_CONF_REJCT:
		if (data[1] == machine->identi)
		{
			proc_reject(data, data + length, machine);
			do_automaton(port, machine, PPP_CNAK);
		} else
		{
			port->generic.stat_dropped++;
		}
		break;
	case PPP_TERM_REQU:
		do_automaton(port, machine, PPP_TREQ);
		break;
	case PPP_TERM_ACK:
		do_automaton(port, machine, PPP_TACK);
		break;
	case PPP_CODE_REJCT:
		strcpy(string, "      ]\r\n");
		number_to_string(*(data + 4), &string[3], 2);
		string[3] = '[';
		ppp_log_text(port, string);
		flag = (*(data + 4) == 0 || *(data + 4) > machine->essent) ? PPP_REJOK : PPP_REJFATAL;
		do_automaton(port, machine, flag);
		break;
	case PPP_PRTCL_REJCT:
		switch ((data[4] << 8) | data[5])
		{
		case PPP_LCP:
		case PPP_IPCP:
		case PPP_IP:
			ppp_log_it(port, machine, receiving, "reject essential protocol !", -1);
			flag = PPP_REJFATAL;
			break;
		case PPP_VJHC_C:
		case PPP_VJHC_UNC:
			ppp_log_it(port, machine, receiving, "reject VJHC.", -1);
			port->generic.flags &= ~FLG_VJHC;
			flag = PPP_REJOK;
			break;
		default:
			flag = PPP_REJOK;
			break;
		}
		do_automaton(port, &port->ppp.lcp, flag);
		break;
	case PPP_ECHO_REQU:
		if (*(uint32 *) (data + 4) != port->ppp.remote_magic)
		{
			port->generic.stat_dropped++;
		} else
		{
			*(uint32 *) (data + 4) = port->ppp.local_magic;
			send_cp(port, PPP_LCP, PPP_ECHO_REPLY, data[1], length - 4, data + 4);
		}
		break;
	case PPP_ECHO_REPLY:
	case PPP_DISC_REQU:
		if (*(uint32 *) (data + 4) != port->ppp.remote_magic)
			port->generic.stat_dropped++;
		break;
	}
}


