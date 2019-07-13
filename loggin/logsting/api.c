/*********************************************************************/
/*                                                                   */
/*     LogSTinG : API Interception Part                              */
/*                                                                   */
/*      Version 1.2                         from 17. March 1997      */
/*                                                                   */
/*********************************************************************/


#include <stdio.h>
#include <string.h>

#include "transprt.h"
#include "port.h"
#include "layer.h"
#include "logsting.h"


static DRV_HDR *cdecl(*old_get_dftab) (const char *drv_name);

static char const tcp_state[][12] = {
	"TCLOSED",
	"TLISTEN",
	"TSYN_SENT",
	"TSYN_RECV",
	"TESTABLISH",
	"TFIN_WAIT1",
	"TFIN_WAIT2",
	"TCLOSE_WAIT",
	"TCLOSING",
	"TLAST_ACK",
	"TTIME_WAIT"
};

static char func_code[] = "Driver \'%\', Code \'%\'";
static char const act_open[] = "   Active open :";
static char const pass_open[] = "   Passive open :";


static const char *const code_txt[] = { "HNDLR_SET", "HNDLR_FORCE", "HNDLR_REMOVE", "HNDLR_QUERY" };


static DRV_HDR *cdecl my_get_dftab(const char *drv_name)
{
	if (strcmp(drv_name, "LOGSTING : QUERY") == 0)
		return (DRV_HDR *) &log_this;

	if (strcmp(drv_name, "LOGSTING : REMOVE") == 0)
	{
		Supexec(deinstall);
		return (DRV_HDR *) 0x4f6b4f6bL;	/* 'OkOk' */
	}

	return old_get_dftab(drv_name);
}


static void *cdecl my_KRmalloc(int32 length)
{
	void *ret_val;

	offset++;

	if (cli_flags[0])
	{
		write_function("KRmalloc");
		if (generic[1])
			write_parameter("Length", INT32, &length, "");
	}
	ret_val = KRmalloc(length);

	if (cli_flags[0] && generic[1])
		write_parameter("returns", UINT32, &ret_val, "");

	--offset;

	return ret_val;
}


static void cdecl my_KRfree(void *mem_block)
{
	offset++;

	if (cli_flags[1])
	{
		write_function("KRfree");
		if (generic[1])
			write_parameter("Memory_Block", UINT32, &mem_block, "");
	}
	KRfree(mem_block);

	--offset;
}


static int32 cdecl my_KRgetfree(int16 flag)
{
	int32 ret_val;

	offset++;

	if (cli_flags[2])
	{
		write_function("KRgetfree");
		if (generic[1])
			write_parameter("Block_Flag", INT16, &flag, flag ? "Query_Largest" : "Query_Total");
	}
	ret_val = KRgetfree(flag);

	if (cli_flags[2] && generic[1])
		write_parameter("returns", INT32, &ret_val, "");

	--offset;

	return ret_val;
}


static void *cdecl my_KRrealloc(void *mem_block, int32 length)
{
	void *ret_val;

	offset++;

	if (cli_flags[3])
	{
		write_function("KRrealloc");
		if (generic[1])
		{
			write_parameter("Memory_Block", UINT32, &mem_block, "");
			write_parameter("New_Length", INT32, &length, "");
		}
	}
	ret_val = KRrealloc(mem_block, length);

	if (cli_flags[3] && generic[1])
		write_parameter("returns", UINT32, &ret_val, "");

	--offset;

	return ret_val;
}


static int16 cdecl my_TCP_open(uint32 rem_host, uint16 rem_port, uint16 tos, uint16 buff_size)
{
	int16 ret_val;

	offset++;

	if (cli_flags[7])
	{
		write_function("TCP_open");
		if (generic[1])
		{
			if (rem_port != TCP_ACTIVE && rem_port != TCP_PASSIVE)
			{
				write_parameter("Remote_Host", IP_ADDR, &rem_host, "");
				write_parameter("Remote_Port", UINT16, &rem_port, "");
			} else
			{
				write_log_text(rem_port == TCP_ACTIVE ? act_open : pass_open);
				write_parameter("CAB.Local_Host", IP_ADDR, &((CAB *) rem_host)->lhost, "");
				write_parameter("CAB.Local_Port", UINT16, &((CAB *) rem_host)->lport, "");
				write_parameter("CAB.Remote_Host", IP_ADDR, &((CAB *) rem_host)->rhost, "");
				write_parameter("CAB.Remote_Port", UINT16, &((CAB *) rem_host)->rport, "");
			}
			write_parameter("Type_Of_Service", UINT16, &tos, "");
			write_parameter("Buffer_Size", UINT16, &buff_size, "");
		}
	}
	ret_val = TCP_open(rem_host, rem_port, tos, buff_size);

	if (cli_flags[7] && generic[1])
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));

	--offset;

	return ret_val;
}


static int16 cdecl my_TCP_close(int16 connec, int16 mode, int16 *result)
{
	int16 ret_val;

	offset++;

	if (cli_flags[8])
	{
		write_function("TCP_close");
		if (generic[1])
		{
			write_parameter("Connection", INT16, &connec, "");
			write_parameter("Mode (Timeout)", INT16, &mode, "");
			if (generic[2] && mode < 0)
				write_parameter("Result_Address", UINT32, &result, "");
		}
	}
	ret_val = TCP_close(connec, mode, result);

	if (cli_flags[8] && generic[1])
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));

	--offset;

	return ret_val;
}


static int16 cdecl my_TCP_send(int16 connec, const void *buffer, int16 length)
{
	int16 ret_val;

	offset++;

	if (cli_flags[9])
	{
		write_function("TCP_send");
		if (generic[1])
		{
			write_parameter("Connection", INT16, &connec, "");
			if (generic[2])
				write_parameter("Buffer_Address", UINT32, &buffer, "");
			write_parameter("Buffer_Length", INT16, &length, "");
			if (generic[3])
				write_buffer(buffer, length);
		}
	}
	ret_val = TCP_send(connec, buffer, length);

	if (cli_flags[9] && generic[1])
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));

	--offset;

	return ret_val;
}


static int16 cdecl my_TCP_wait_state(int16 connec, int16 state, int16 timeout)
{
	int16 ret_val;

	offset++;

	if (cli_flags[10])
	{
		write_function("TCP_wait_state");
		if (generic[1])
		{
			write_parameter("Connection", INT16, &connec, "");
			write_parameter("State", INT16, &state, 0 <= state && state <= 10 ? tcp_state[state] : "");
			write_parameter("Timeout", INT16, &timeout, "");
		}
	}
	ret_val = TCP_wait_state(connec, state, timeout);

	if (cli_flags[10] && generic[1])
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));

	--offset;

	return ret_val;
}


static int16 cdecl my_TCP_ack_wait(int16 connec, int16 timeout)
{
	int16 ret_val;

	offset++;

	if (cli_flags[11])
	{
		write_function("TCP_ack_wait");
		if (generic[1])
		{
			write_parameter("Connection", INT16, &connec, "");
			write_parameter("Timeout", INT16, &timeout, "");
		}
	}
	ret_val = TCP_ack_wait(connec, timeout);

	if (cli_flags[11] && generic[1])
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));

	--offset;

	return ret_val;
}


static int16 cdecl my_TCP_info(int16 connec, TCPIB *tcpib)
{
	int16 ret_val;

	offset++;

	if (cli_flags[35])
	{
		write_function("TCP_info");
		if (generic[1])
		{
			write_parameter("Connection", INT16, &connec, "");
			if (generic[2])
				write_parameter("Information_Block", UINT32, &tcpib, "");
		}
	}
	tcpib->request |= TCPI_state;
	ret_val = TCP_info(connec, tcpib);

	if (cli_flags[35] && generic[1])
	{
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));
		/* if (ret_val == 0) */
		{
			write_parameter("State", INT16, &tcpib->state, tcpib->state <= 10 ? tcp_state[tcpib->state] : "");
		}
	}

	--offset;

	return ret_val;
}


static int16 cdecl my_UDP_open(uint32 rem_host, uint16 rem_port)
{
	int16 ret_val;

	offset++;

	if (cli_flags[12])
	{
		write_function("UDP_open");
		if (generic[1])
		{
			if (rem_port != UDP_EXTEND)
			{
				write_parameter("Remote_Host", IP_ADDR, &rem_host, "");
				write_parameter("Remote_Port", UINT16, &rem_port, "");
			} else
			{
				write_parameter("CAB.Local_Host", IP_ADDR, &((CAB *) rem_host)->lhost, "");
				write_parameter("CAB.Local_Port", UINT16, &((CAB *) rem_host)->lport, "");
				write_parameter("CAB.Remote_Host", IP_ADDR, &((CAB *) rem_host)->rhost, "");
				write_parameter("CAB.Remote_Port", UINT16, &((CAB *) rem_host)->rport, "");
			}
		}
	}
	ret_val = UDP_open(rem_host, rem_port);

	if (cli_flags[12] && generic[1])
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));

	--offset;

	return ret_val;
}


static int16 cdecl my_UDP_close(int16 connec)
{
	int16 ret_val;

	offset++;

	if (cli_flags[13])
	{
		write_function("UDP_close");
		if (generic[1])
			write_parameter("Connection", INT16, &connec, "");
	}
	ret_val = UDP_close(connec);

	if (cli_flags[13] && generic[1])
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));

	--offset;

	return ret_val;
}


static int16 cdecl my_UDP_send(int16 connec, const void *buffer, int16 length)
{
	int16 ret_val;

	offset++;

	if (cli_flags[14])
	{
		write_function("UDP_send");
		if (generic[1])
		{
			write_parameter("Connection", INT16, &connec, "");
			if (generic[2])
				write_parameter("Buffer_Address", UINT32, &buffer, "");
			write_parameter("Buffer_Length", INT16, &length, "");
			if (generic[3])
				write_buffer(buffer, length);
		}
	}
	ret_val = UDP_send(connec, buffer, length);

	if (cli_flags[14] && generic[1])
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));

	--offset;

	return ret_val;
}


static int16 cdecl my_UDP_info(int16 handle, UDPIB *buffer)
{
	int16 ret_val;

	offset++;

	if (cli_flags[37])
	{
		write_function("UDP_info");
		if (generic[1])
		{
			write_parameter("Connection", INT16, &handle, "");
			if (generic[2])
				write_parameter("Information_Block", UINT32, &buffer, "");
		}
	}
	buffer->request |= UDPI_state; /* WTF: modifying clients request here */
	ret_val = UDP_info(handle, buffer);
	if (cli_flags[35] && generic[1]) /* BUG: should be 37 */
	{
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));
		write_parameter("State", INT16, &buffer->state, ""); /* BUG: should be UINT16 */
	}
	--offset;
	return ret_val;
}


static int16 cdecl my_ICMP_send(uint32 rem_host, uint8 type, uint8 code, const void *data, uint16 length)
{
	int16 ret_val;

	offset++;

	if (cli_flags[32])
	{
		write_function("ICMP_send");
		if (generic[1])
		{
			write_parameter("Remote_Host", IP_ADDR, &rem_host, "");
			write_parameter("ICMP_Type", UINT8, &type, "");
			write_parameter("ICMP_Code", UINT8, &code, "");
			if (generic[2])
				write_parameter("Buffer_Address", UINT32, &data, "");
			write_parameter("Buffer_Length", UINT16, &length, "");
			if (generic[3])
				write_buffer(data, length);
		}
	}
	ret_val = ICMP_send(rem_host, type, code, data, length);

	if (cli_flags[32] && generic[1])
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));

	--offset;

	return ret_val;
}


static int16 cdecl my_ICMP_handler(int16 cdecl(*handler) (IP_DGRAM *), int16 code)
{
	int16 ret_val;

	offset++;

	if (cli_flags[33])
	{
		write_function("ICMP_handler");
		if (generic[1])
		{
			if (generic[2])
				write_parameter("Handler_Call", UINT32, &handler, "");
			write_parameter("Code", INT16, &code, code >= 0 ? code < 4 ? code_txt[code] : "" : "");
		}
	}
	ret_val = ICMP_handler(handler, code);

	if (cli_flags[33] && generic[1])
		write_parameter("returns", INT16, &ret_val, ret_val ? "TRUE" : "FALSE");

	--offset;

	return ret_val;
}


static void cdecl my_ICMP_discard(IP_DGRAM *dgram)
{
	offset++;

	if (cli_flags[34])
	{
		write_function("ICMP_discard");
		if (generic[1])
		{
			if (generic[2])
				write_parameter("Datagram", UINT32, &dgram, "");
			if (generic[3])
			{
				write_buffer(&dgram->hdr, (int)sizeof(IP_HDR));
				write_buffer(dgram->options, dgram->opt_length);
				write_buffer(dgram->pkt_data, dgram->pkt_length);
			}
		}
	}
	ICMP_discard(dgram);

	--offset;
}


static int16 cdecl my_CNkick(int16 connec)
{
	int16 ret_val;

	offset++;

	if (cli_flags[15])
	{
		write_function("CNkick");
		if (generic[1])
			write_parameter("Connection", INT16, &connec, "");
	}
	ret_val = CNkick(connec);

	if (cli_flags[15] && generic[1])
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));

	--offset;

	return ret_val;
}


static int16 cdecl my_CNbyte_count(int16 connec)
{
	int16 ret_val;

	offset++;

	if (cli_flags[16])
	{
		write_function("CNbyte_count");
		if (generic[1])
			write_parameter("Connection", INT16, &connec, "");
	}
	ret_val = CNbyte_count(connec);

	if (cli_flags[16] && generic[1])
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));

	--offset;

	return ret_val;
}


static int16 cdecl my_CNget_char(int16 connec)
{
	int16 ret_val;

	offset++;

	if (cli_flags[17])
	{
		write_function("CNget_char");
		if (generic[1])
			write_parameter("Connection", INT16, &connec, "");
	}
	ret_val = CNget_char(connec);

	if (cli_flags[17] && generic[1])
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));

	--offset;

	return ret_val;
}


static NDB *cdecl my_CNget_NDB(int16 connec)
{
	NDB *ret_val;

	offset++;

	if (cli_flags[18])
	{
		write_function("CNget_NDB");
		if (generic[1])
			write_parameter("Connection", INT16, &connec, "");
	}
	ret_val = CNget_NDB(connec);

	if (cli_flags[18] && generic[1])
	{
		if (generic[2])
			write_parameter("returns", UINT32, &ret_val, "");
		if (generic[3] && ret_val)
			write_buffer(ret_val->ndata, ret_val->len);
	}

	--offset;

	return ret_val;
}


static int16 cdecl my_CNget_block(int16 connec, void *block, int16 length)
{
	int16 ret_val;

	offset++;

	if (cli_flags[19])
	{
		write_function("CNget_block");
		if (generic[1])
		{
			write_parameter("Connection", INT16, &connec, "");
			if (generic[2])
				write_parameter("Block_Address", UINT32, &block, "");
			write_parameter("Block_Length", INT16, &length, "");
		}
	}
	ret_val = CNget_block(connec, block, length);

	if (cli_flags[19] && generic[1])
	{
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));
		if (ret_val >= 0 && generic[3])
			write_buffer(block, length);
	}

	--offset;

	return ret_val;
}


static int16 cdecl my_CNgets(int16 connec, char *buffer, int16 length, char delim)
{
	int16 ret_val;

	offset++;

	if (cli_flags[31])
	{
		write_function("CNgets");
		if (generic[1])
		{
			write_parameter("Connection", INT16, &connec, "");
			write_parameter("Block_Length", INT16, &length, "");
			write_parameter("Delimiter", UINT8, &delim, "");
		}
	}
	ret_val = CNgets(connec, buffer, length, delim);

	if (cli_flags[31] && generic[1])
	{
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));
		if (ret_val >= 0 && generic[3])
			write_buffer(buffer, ret_val);
	}

	--offset;

	return ret_val;
}


static CIB *cdecl my_CNget_info(int16 connec)
{
	CIB *ret_val;

	offset++;

	if (cli_flags[26])
	{
		write_function("CNget_info");
		if (generic[1])
			write_parameter("Connection", INT16, &connec, "");
	}
	ret_val = CNgetinfo(connec);

	if (cli_flags[26] && generic[1])
	{
		write_parameter("returns", UINT32, &ret_val, "");
		if (ret_val != NULL)
		{
			write_parameter("Protocol", UINT16, &ret_val->protocol, ret_val->protocol == P_TCP ? "TCP" : ret_val->protocol == P_UDP ? "UDP" : "");
			write_parameter("CAB.Local_Host", IP_ADDR, &ret_val->address.lhost, "");
			write_parameter("CAB.Local_Port", UINT16, &ret_val->address.lport, "");
			write_parameter("CAB.Remote_Host", IP_ADDR, &ret_val->address.rhost, "");
			write_parameter("CAB.Remote_Port", UINT16, &ret_val->address.rport, "");
			write_parameter("Net_Status", UINT16, &ret_val->status, "");
		}
	}

	--offset;

	return ret_val;
}


static int16 cdecl my_resolve(const char *domain, char **real_domain, uint32 *ip_list, int16 ip_num)
{
	int16 ret_val;
	int16 count;
	char num[20];

	offset++;

	if (cli_flags[21])
	{
		write_function("resolve");
		if (generic[1])
		{
			write_parameter("Domain_Name", STRING, domain, "");
			write_parameter("IP_List_Address", UINT32, &ip_list, "");
			write_parameter("IP_List_Length", INT16, &ip_num, "");
		}
	}
	ret_val = resolve(domain, real_domain, ip_list, ip_num);

	if (cli_flags[21] && generic[1])
	{
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));
		if (real_domain)
		{
			if (generic[2])
				write_parameter("Real_Buffer", UINT32, real_domain, "");
			if (*real_domain && generic[3])
				write_buffer(*real_domain, 32);
		}
		if (ip_list)
		{
			for (count = 0; count < ret_val; count++)
			{
				sprintf(num, "IP#%-2d", count);
				write_parameter(num, IP_ADDR, &ip_list[count], "");
			}
		}
	}

	--offset;

	return ret_val;
}


static int16 cdecl my_setvstr(const char *name, const char *value)
{
	int16 ret_val;

	offset++;

	if (cli_flags[29])
	{
		write_function("setvstr");
		if (generic[1])
		{
			write_parameter("Variable", STRING, name, "");
			write_parameter("Value", STRING, value, "");
		}
	}
	ret_val = setvstr(name, value);

	if (cli_flags[29] && generic[1])
		write_parameter("returns", INT16, &ret_val, ret_val ? "Okay" : "Error");

	--offset;

	return ret_val;
}


static const char *cdecl my_getvstr(const char *name)
{
	const char *ret_val;

	offset++;

	if (cli_flags[5])
	{
		write_function("getvstr");
		if (generic[1])
			write_parameter("Variable", STRING, name, "");
	}
	ret_val = getvstr(name);

	if (cli_flags[5] && generic[1])
		write_parameter("returns", UINT32, &ret_val, ret_val);

	--offset;

	return ret_val;
}


static int16 cdecl my_on_port(const char *port)
{
	int16 ret_val;

	offset++;

	if (cli_flags[27])
	{
		write_function("on_port");
		if (generic[1])
			write_parameter("Port_Name", STRING, port, "");
	}
	ret_val = on_port(port);

	if (cli_flags[27] && generic[1])
		write_parameter("returns", INT16, &ret_val, ret_val ? "Okay" : "Error");

	--offset;

	return ret_val;
}


static void cdecl my_off_port(const char *port)
{
	offset++;

	if (cli_flags[28])
	{
		write_function("off_port");
		if (generic[1])
			write_parameter("Port_Name", STRING, port, "");
	}
	off_port(port);

	--offset;
}


static int16 cdecl my_query_port(const char *port)
{
	int16 ret_val;

	offset++;

	if (cli_flags[30])
	{
		write_function("query_port");
		if (generic[1])
			write_parameter("Port_Name", STRING, port, "");
	}
	ret_val = query_port(port);

	if (cli_flags[30] && generic[1])
		write_parameter("returns", INT16, &ret_val, ret_val ? "Connected" : "No Connection");

	--offset;

	return ret_val;
}


static int16 cdecl my_cntrl_port(const char *port, uint32 argument, int16 code)
{
	int16 ret_val;

	offset++;

	if (cli_flags[36])
	{
		write_function("cntrl_port");
		if (generic[1])
		{
			write_parameter("Port_Name", STRING, port, "");
			write_parameter("Argument", UINT32, &argument, "");
			func_code[8] = code >> 8;
			func_code[18] = code & 0xff;
			write_parameter("Function_Code", STRING, code_txt, "");
		}
	}
	ret_val = cntrl_port(port, argument, code);

	if (cli_flags[36] && generic[1])
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));

	--offset;

	return ret_val;
}


static void cdecl my_ser_disable(void)
{
	offset++;

	if (cli_flags[22])
		write_function("ser_disable");

	ser_disable();

	--offset;
}


static void cdecl my_ser_enable(void)
{
	offset++;

	if (cli_flags[23])
		write_function("ser_enable");

	ser_enable();

	--offset;
}


static int16 cdecl my_set_flag(int16 flag)
{
	int16 ret_val;

	offset++;

	if (cli_flags[24])
	{
		write_function("set_flag");
		if (generic[1])
			write_parameter("Flag_No", INT16, &flag, "");
	}
	ret_val = set_flag(flag);

	if (cli_flags[24] && generic[1])
		write_parameter("returns", INT16, &ret_val, ret_val ? "Failed" : "Success");

	--offset;

	return ret_val;
}


static void cdecl my_clear_flag(int16 flag)
{
	offset++;

	if (cli_flags[25])
	{
		write_function("clear_flag");
		if (generic[1])
			write_parameter("Flag_No", INT16, &flag, "");
	}
	clear_flag(flag);

	--offset;
}


static const char *cdecl my_get_err_text(int16 error)
{
	const char *ret_val;

	offset++;

	if (cli_flags[4])
	{
		write_function("get_err_text");
		if (generic[1])
			write_parameter("Error_No", INT16, &error, get_error(error));
	}
	ret_val = get_err_text(error);

	if (cli_flags[4] && generic[1])
		write_parameter("returns", STRING, ret_val, "");

	--offset;

	return ret_val;
}


static int16 cdecl my_carrier_detect(void)
{
	int16 ret_val;

	offset++;

	if (cli_flags[6])
		write_function("carrier_detect");

	ret_val = carrier_detect();

	if (cli_flags[6] && generic[1])
	{
		write_parameter("returns", INT16, &ret_val, ret_val ? ret_val == 1 ? "Carrier" : "No Carrier" : "Unknown");
	}

	--offset;

	return ret_val;
}


static void cdecl my_housekeep(void)
{
	offset++;

	if (cli_flags[20])
		write_function("housekeep");

	housekeep();

	--offset;
}


static int16 cdecl my_IP_send(uint32 src, uint32 dest, uint8 tos, uint16 frg, uint8 ttl, uint8 prtcl, uint16 id, void *data, uint16 dlen, void *opt, uint16 olen)
{
	int16 ret_val;

	offset++;

	if (mod_flags[5])
	{
		write_function("IP_send");
		if (generic[1])
		{
			write_parameter("Local_IP", IP_ADDR, &src, "");
			write_parameter("Remote_IP", IP_ADDR, &dest, "");
			write_parameter("Type_Of_Service", UINT8, &tos, "");
			write_parameter("Fragment_Flag", UINT16, &frg, frg ? "Don't fragment" : "May be fragmented");
			write_parameter("Time_To_Live", UINT8, &ttl, "");
			write_parameter("Protocol", UINT8, &prtcl, prtcl == P_ICMP ? "ICMP" : prtcl == P_TCP ? "TCP" : prtcl == P_UDP ? "UDP" : "");
			write_parameter("Identifier", UINT16, &id, "");
			if (generic[2])
				write_parameter("Options_Address", UINT32, &opt, "");
			write_parameter("Options_Length", UINT16, &olen, "");
			if (generic[3])
				write_buffer(opt, olen);
			if (generic[2])
				write_parameter("Data_Address", UINT32, &data, "");
			write_parameter("Data_Length", UINT16, &dlen, "");
			if (generic[3])
				write_buffer(data, dlen);
		}
	}
	ret_val = IP_send(src, dest, tos, frg, ttl, prtcl, id, data, dlen, opt, olen);

	if (mod_flags[5] && generic[1])
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));

	--offset;

	return ret_val;
}


static IP_DGRAM *cdecl my_IP_fetch(int16 protocol)
{
	IP_DGRAM *ret_val;

	offset++;

	if (mod_flags[6])
	{
		write_function("IP_fetch");
		if (generic[1])
		{
			write_parameter("Protocol", UINT8, &protocol, 
				protocol == P_ICMP ? "ICMP" : 
				protocol == P_TCP ? "TCP" :
				protocol == P_UDP ? "UDP" : "");
		}
	}
	ret_val = IP_fetch(protocol);

	if (mod_flags[6] && generic[1])
	{
		write_parameter("returns", UINT32, &ret_val, (ret_val) ? "IP Datagram" : "No Datagram");
		if (generic[3])
		{
			write_buffer(&ret_val->hdr, (int)sizeof(IP_HDR));
			write_buffer(ret_val->options, ret_val->opt_length);
			write_buffer(ret_val->pkt_data, ret_val->pkt_length);
		}
	}

	--offset;

	return ret_val;
}


static int16 cdecl my_IP_handler(int16 protocol, int16 cdecl(*handler) (IP_DGRAM *), int16 code)
{
	int16 ret_val;

	offset++;

	if (mod_flags[7])
	{
		write_function("IP_handler");
		if (generic[1])
		{
			write_parameter("Protocol", UINT8, &protocol, protocol == P_ICMP ? "ICMP" : protocol == P_TCP ? "TCP" : protocol == P_UDP ? "UDP" : "");
			if (generic[2])
				write_parameter("Handler_Call", UINT32, &handler, "");
			write_parameter("Code", INT16, &code, code >= 0 ? code < 4 ? code_txt[code] : "" : "");
		}
	}
	ret_val = IP_handler(protocol, handler, code);

	if (mod_flags[7] && generic[1])
		write_parameter("returns", INT16, &ret_val, ret_val ? "TRUE" : "FALSE");

	--offset;

	return ret_val;
}


static void cdecl my_IP_discard(IP_DGRAM *dgram, int16 flag)
{
	offset++;

	if (mod_flags[8])
	{
		write_function("IP_discard");
		if (generic[1])
		{
			if (generic[2])
				write_parameter("Datagram", UINT32, &dgram, "");
			if (generic[3])
			{
				write_buffer(&dgram->hdr, (int)sizeof(IP_HDR));
				write_buffer(dgram->options, dgram->opt_length);
				write_buffer(dgram->pkt_data, dgram->pkt_length);
			}
			write_parameter("Data_Flag", INT16, &flag, flag ? "Discard All" : "Discard All But Data");
		}
	}
	IP_discard(dgram, flag);

	--offset;
}


static int16 cdecl my_PRTCL_announce(int16 protocol)
{
	int16 ret_val;

	offset++;

	if (mod_flags[9])
	{
		write_function("PRTCL_announce");
		if (generic[1])
			write_parameter("Protocol", UINT8, &protocol, protocol == P_ICMP ? "ICMP" : protocol == P_TCP ? "TCP" : protocol == P_UDP ? "UDP" : "");
	}
	ret_val = PRTCL_announce(protocol);

	if (mod_flags[9] && generic[1])
		write_parameter("returns", INT16, &ret_val, ret_val ? "Occupied" : "Free, Okay");

	--offset;

	return ret_val;
}


static int16 cdecl my_PRTCL_get_parameters(uint32 rem, uint32 *lcl, int16 *ttl, uint16 *mtu)
{
	int16 ret_val;

	offset++;

	if (mod_flags[10])
	{
		write_function("PRTCL_get_parameters");
		if (generic[1])
		{
			write_parameter("Remote_IP", IP_ADDR, &rem, "");
			if (generic[2])
			{
				write_parameter("Addr_Local_IP", IP_ADDR, &lcl, "");
				write_parameter("Addr_Time_To_Live", UINT32, &ttl, "");
				write_parameter("Addr_Max_Xmit_Unit", UINT32, &mtu, "");
			}
		}
	}
	ret_val = PRTCL_get_parameters(rem, lcl, ttl, mtu);

	if (mod_flags[10] && generic[1])
	{
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));
		if (lcl)
			write_parameter("Local_IP", UINT32, lcl, "");
		if (ttl)
			write_parameter("Time_To_Live", INT16, ttl, "");
		if (mtu)
			write_parameter("Max_Xmit_Unit", UINT16, mtu, "");
	}

	--offset;

	return ret_val;
}


static int16 cdecl my_PRTCL_request(void *anonymous, CN_FUNCS *functions)
{
	int16 ret_val;

	offset++;

	if (mod_flags[11])
	{
		write_function("PRTCL_request");
		if (generic[1])
		{
			if (generic[2])
				write_parameter("Anonymous_Data", UINT32, &anonymous, "");
			if (generic[2])
			{
				write_parameter("Function_Array", UINT32, &functions, "");
				write_parameter("  Call_CNkick      ", UINT32, &functions->CNkick, "");
				write_parameter("  Call_CNbyte_count", UINT32, &functions->CNbyte_count, "");
				write_parameter("  Call_CNget_char  ", UINT32, &functions->CNget_char, "");
				write_parameter("  Call_CNget_NDB   ", UINT32, &functions->CNget_NDB, "");
				write_parameter("  Call_CNget_block ", UINT32, &functions->CNget_block, "");
				write_parameter("  Call_CNgetinfo   ", UINT32, &functions->CNgetinfo, "");
				write_parameter("  Call_CNgets      ", UINT32, &functions->CNgets, "");
			}
		}
	}
	ret_val = PRTCL_request(anonymous, functions);

	if (mod_flags[11] && generic[1])
		write_parameter("returns", INT16, &ret_val, ret_val >= 0 ? "Handle" : "Error");

	--offset;

	return ret_val;
}


static void cdecl my_PRTCL_release(int16 handle)
{
	offset++;

	if (mod_flags[12])
	{
		write_function("PRTCL_release");
		if (generic[1])
			write_parameter("Handle", INT16, &handle, "");
	}
	PRTCL_release(handle);

	--offset;
}


static void *cdecl my_PRTCL_lookup(int16 handle, CN_FUNCS *functions)
{
	void *ret_val;

	offset++;

	if (mod_flags[13])
	{
		write_function("PRTCL_lookup");
		if (generic[1])
		{
			write_parameter("Handle", INT16, &handle, "");
			if (generic[2])
			{
				write_parameter("Function_Array", UINT32, &functions, "");
				write_parameter("  Call_CNkick      ", UINT32, &functions->CNkick, "");
				write_parameter("  Call_CNbyte_count", UINT32, &functions->CNbyte_count, "");
				write_parameter("  Call_CNget_char  ", UINT32, &functions->CNget_char, "");
				write_parameter("  Call_CNget_NDB   ", UINT32, &functions->CNget_NDB, "");
				write_parameter("  Call_CNget_block ", UINT32, &functions->CNget_block, "");
				write_parameter("  Call_CNgetinfo   ", UINT32, &functions->CNgetinfo, "");
				write_parameter("  Call_CNgets      ", UINT32, &functions->CNgets, "");
			}
		}
	}
	ret_val = PRTCL_lookup(handle, functions);

	if (mod_flags[13] && generic[1])
	{
		if (generic[2])
			write_parameter("returns", UINT32, &ret_val, "");
	}

	--offset;

	return ret_val;
}


static int16 cdecl my_TIMER_call(int16 cdecl(*handler) (IP_DGRAM *), int16 code)
{
	int16 ret_val;

	offset++;

	if (mod_flags[14])
	{
		write_function("TIMER_call");
		if (generic[1])
		{
			if (generic[2])
				write_parameter("Handler_Call", UINT32, &handler, "");
			write_parameter("Code", INT16, &code, code >= 0 ? code < 4 ? code_txt[code] : "" : "");
		}
	}
	ret_val = TIMER_call(handler, code);

	if (mod_flags[14] && generic[1])
		write_parameter("returns", INT16, &ret_val, ret_val ? "TRUE" : "FALSE");

	--offset;

	return ret_val;
}


static int32 cdecl my_TIMER_now(void)
{
	int32 ret_val;

	offset++;

	if (mod_flags[15])
	{
		write_function("TIMER_now");
	}
	ret_val = TIMER_now();

	if (mod_flags[15] && generic[1])
		write_parameter("returns", INT32, &ret_val, "milliseconds");

	--offset;

	return ret_val;
}


static int32 cdecl my_TIMER_elapsed(int32 moment)
{
	int32 ret_val;

	offset++;

	if (mod_flags[16])
	{
		write_function("TIMER_elapsed");
		if (generic[1])
			write_parameter("Since", INT32, &moment, "milliseconds (midnight)");
	}
	ret_val = TIMER_elapsed(moment);

	if (mod_flags[16] && generic[1])
		write_parameter("returns", INT32, &ret_val, "milliseconds");

	--offset;

	return ret_val;
}


static void cdecl my_set_dgram_ttl(IP_DGRAM *dgram)
{
	offset++;

	if (mod_flags[0])
	{
		write_function("set_dgram_ttl");
		if (generic[1])
		{
			if (generic[2])
				write_parameter("Datagram", UINT32, &dgram, "");
			if (generic[3])
			{
				write_buffer(&dgram->hdr, (int)sizeof(IP_HDR));
				write_buffer(dgram->options, dgram->opt_length);
				write_buffer(dgram->pkt_data, dgram->pkt_length);
			}
		}
	}
	set_dgram_ttl(dgram);

	--offset;
}


static int16 cdecl my_check_dgram_ttl(IP_DGRAM *dgram)
{
	int16 ret_val;

	offset++;

	if (mod_flags[1])
	{
		write_function("check_dgram_ttl");
		if (generic[1])
		{
			if (generic[2])
				write_parameter("Datagram", UINT32, &dgram, "");
			if (generic[3])
			{
				write_buffer(&dgram->hdr, (int)sizeof(IP_HDR));
				write_buffer(dgram->options, dgram->opt_length);
				write_buffer(dgram->pkt_data, dgram->pkt_length);
			}
		}
	}
	ret_val = check_dgram_ttl(dgram);

	if (mod_flags[1] && generic[1])
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));

	--offset;

	return ret_val;
}


static int32 cdecl my_set_sysvars(int16 active, int16 fraction)
{
	int32 ret_val;
	char line[20];

	offset++;

	if (mod_flags[3])
	{
		write_function("set_sysvars");
		if (generic[1])
		{
			write_parameter("Active", INT16, &active, active >= 0 ? active == 0 ? "FALSE" : "TRUE" : "NOP");
			sprintf(line, "%d ms", 5 * fraction);
			write_parameter("Fraction", INT16, &fraction, fraction == -1 ? "NOP" : line);
		}
	}
	ret_val = set_sysvars(active, fraction);

	if (mod_flags[3] && generic[1])
	{
		active = ret_val >> 16;
		write_parameter("Old_Active", INT16, &active, active == 0 ? "FALSE" : "TRUE");
		sprintf(line, "%d ms", 5 * (fraction = (int16) (ret_val & 0xffffL)));
		write_parameter("Fraction", INT16, &fraction, line);
	}

	--offset;

	return ret_val;
}


static void cdecl my_query_chains(PORT **port, DRIVER **driver, LAYER **layer)
{
	offset++;

	if (mod_flags[4])
	{
		write_function("query_chains");
		if (generic[1])
		{
			if (generic[2])
			{
				write_parameter("Addr_Ptr_Port", UINT32, &port, "");
				write_parameter("Addr_Ptr_Driver", UINT32, &driver, "");
				write_parameter("Addr_Ptr_Layer", UINT32, &layer, "");
			}
		}
	}
	query_chains(port, driver, layer);

	if (mod_flags[4] && generic[1])
	{
		if (generic[2])
		{
			write_parameter("Ptr_Port", UINT32, port, "");
			write_parameter("Ptr_Driver", UINT32, driver, "");
			write_parameter("Ptr_Layer", UINT32, layer, "");
		}
	}

	--offset;
}


static int32 cdecl my_protect_exec(void *parameter, int32 cdecl(*code) (void *))
{
	int32 ret_val;

	offset++;

	if (mod_flags[17])
	{
		write_function("protect_exec");
		if (generic[1])
		{
			if (generic[2])
			{
				write_parameter("Ptr_Parameter", UINT32, &parameter, "");
				write_parameter("Ptr_Code", UINT32, &code, "");
			}
		}
	}
	ret_val = protect_exec(parameter, code);

	if (mod_flags[17] && generic[1])
		write_parameter("returns", INT32, &ret_val, "");

	--offset;

	return ret_val;
}


static int16 cdecl my_load_routing_table(void)
{
	int16 ret_val;

	offset++;

	if (mod_flags[2])
	{
		write_function("load_routing_table");
	}
	ret_val = load_routing_table();

	if (mod_flags[2] && generic[1])
		write_parameter("returns", INT16, &ret_val, get_error(ret_val));

	--offset;

	return ret_val;
}


static int16 cdecl my_get_route_entry(int16 index, uint32 *subnet, uint32 *submask, PORT **port, uint32 *gateway)
{
	int16 ret_val;

	offset++;

	if (mod_flags[18])
	{
		write_function("get_route_entry");
		if (generic[1])
		{
			write_parameter("Table_Index", INT16, &index, "");
			if (generic[2])
			{
				write_parameter("Addr_SubNet", UINT32, &subnet, "");
				write_parameter("Addr_SubMask", UINT32, &submask, "");
				write_parameter("Addr_PortPtr", UINT32, &port, "");
				write_parameter("Addr_Gateway", UINT32, &gateway, "");
			}
		}
	}
	ret_val = get_route_entry(index, subnet, submask, port, gateway);

	if (mod_flags[18] && generic[1])
	{
		write_parameter("IP_SubNet", IP_ADDR, subnet, "");
		write_parameter("IP_SubMask", IP_ADDR, submask, "");
		if (generic[2])
		{
			write_parameter("Ptr_Port", UINT32, port, "");
		}
		write_parameter("IP_Gateway", IP_ADDR, gateway, "");
		write_parameter("returns", INT16, &ret_val, ret_val == -1 ? "Error" : "");
	}

	--offset;

	return ret_val;
}


static int16 cdecl my_set_route_entry(int16 index, uint32 subnet, uint32 submask, PORT *port, uint32 gateway)
{
	int16 ret_val;

	offset++;

	if (mod_flags[19])
	{
		write_function("set_route_entry");
		if (generic[1])
		{
			write_parameter("Table_Index", INT16, &index, index == -1 ? "New" : "");
			write_parameter("IP_SubNet", IP_ADDR, &subnet, "");
			write_parameter("IP_SubMask", IP_ADDR, &submask, "");
			if (generic[2])
			{
				write_parameter("Ptr_Port", UINT32, &port, "");
			}
			write_parameter("IP_Gateway", IP_ADDR, &gateway, "");
		}
	}
	ret_val = set_route_entry(index, subnet, submask, port, gateway);

	if (mod_flags[19] && generic[1])
	{
		write_parameter("returns", INT16, &ret_val, ret_val == -1 ? "Error" : "");
	}

	--offset;

	return ret_val;
}



void install_api(TPL *sting_tpl, STX *sting_stx, DRV_LIST *sting_drivers)
{
	sting_tpl->KRmalloc = my_KRmalloc;
	sting_tpl->KRfree = my_KRfree;
	sting_tpl->KRgetfree = my_KRgetfree;
	sting_tpl->KRrealloc = my_KRrealloc;
	sting_tpl->TCP_open = my_TCP_open;
	sting_tpl->TCP_close = my_TCP_close;
	sting_tpl->TCP_send = my_TCP_send;
	sting_tpl->TCP_wait_state = my_TCP_wait_state;
	sting_tpl->TCP_ack_wait = my_TCP_ack_wait;
	sting_tpl->TCP_info = my_TCP_info;
	sting_tpl->UDP_open = my_UDP_open;
	sting_tpl->UDP_close = my_UDP_close;
	sting_tpl->UDP_send = my_UDP_send;
	sting_tpl->UDP_info = my_UDP_info;
	sting_tpl->ICMP_send = my_ICMP_send;
	sting_tpl->ICMP_handler = my_ICMP_handler;
	sting_tpl->ICMP_discard = my_ICMP_discard;
	sting_tpl->CNkick = my_CNkick;
	sting_tpl->CNbyte_count = my_CNbyte_count;
	sting_tpl->CNget_char = my_CNget_char;
	sting_tpl->CNget_NDB = my_CNget_NDB;
	sting_tpl->CNget_block = my_CNget_block;
	sting_tpl->CNgets = my_CNgets;
	sting_tpl->CNgetinfo = my_CNget_info;
	sting_tpl->resolve = my_resolve;
	sting_tpl->setvstr = my_setvstr;
	sting_tpl->getvstr = my_getvstr;
	sting_tpl->on_port = my_on_port;
	sting_tpl->off_port = my_off_port;
	sting_tpl->query_port = my_query_port;
	sting_tpl->cntrl_port = my_cntrl_port;
	sting_tpl->ser_disable = my_ser_disable;
	sting_tpl->ser_enable = my_ser_enable;
	sting_tpl->set_flag = my_set_flag;
	sting_tpl->clear_flag = my_clear_flag;
	sting_tpl->get_err_text = my_get_err_text;
	sting_tpl->carrier_detect = my_carrier_detect;
	sting_tpl->housekeep = my_housekeep;

	sting_stx->IP_send = my_IP_send;
	sting_stx->IP_fetch = my_IP_fetch;
	sting_stx->IP_handler = my_IP_handler;
	sting_stx->IP_discard = my_IP_discard;
	sting_stx->PRTCL_announce = my_PRTCL_announce;
	sting_stx->PRTCL_get_parameters = my_PRTCL_get_parameters;
	sting_stx->PRTCL_request = my_PRTCL_request;
	sting_stx->PRTCL_release = my_PRTCL_release;
	sting_stx->PRTCL_lookup = my_PRTCL_lookup;
	sting_stx->TIMER_call = my_TIMER_call;
	sting_stx->TIMER_now = my_TIMER_now;
	sting_stx->TIMER_elapsed = my_TIMER_elapsed;
	sting_stx->set_dgram_ttl = my_set_dgram_ttl;
	sting_stx->check_dgram_ttl = my_check_dgram_ttl;
	sting_stx->set_sysvars = my_set_sysvars;
	sting_stx->query_chains = my_query_chains;
	sting_stx->protect_exec = my_protect_exec;
	sting_stx->load_routing_table = my_load_routing_table;
	sting_stx->get_route_entry = my_get_route_entry;
	sting_stx->set_route_entry = my_set_route_entry;

	old_get_dftab = sting_drivers->get_dftab;
	sting_drivers->get_dftab = my_get_dftab;
}


void uninstall_api(DRV_LIST *sting_drivers)
{
	sting_drivers->get_dftab = old_get_dftab;
}
