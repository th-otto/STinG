/*
 * Copyright (c) 1997, 1998, 1999, 2000
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the Computer Systems
 *	Engineering Group at Lawrence Berkeley Laboratory.
 * 4. Neither the name of the University nor of the Laboratory may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>					/* concession to AIX */

#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ifaddrl.h"

#if defined(__TOS__) || defined(__MINT__)
#include <mint/mintbind.h>
#define ioctl(fd, cmd, arg) Fcntl(fd, arg, cmd)
#endif

#ifdef __PUREC__
char *mint_strerror(int errnum);
#else
#define mint_strerror(err) strerror(err)
#endif


/* Not all systems have IFF_LOOPBACK */
#ifdef IFF_LOOPBACK
#define ISLOOPBACK(p) ((p)->ifr_flags & IFF_LOOPBACK)
#else
#define ISLOOPBACK(p) ((strcmp((p)->ifr_name, "lo0") == 0) || (strcmp((p)->ifr_name, "lo") == 0))
#endif

#define MAX_IPADDR 32

/*
 * Return the interface list
 */
int ifaddrlist(struct ifaddrlist **ipaddrp, char *errbuf)
{
	int fd;
	int nipaddr;
	struct ifreq *ifrp;
	struct ifreq *ifend;
	struct ifreq *ifnext;
	struct sockaddr_in *sin;
	struct ifaddrlist *al;
	struct ifconf ifc;
	struct ifreq ibuf[MAX_IPADDR];
	struct ifreq ifr;
	char device[IF_NAMESIZE + 1];
	static struct ifaddrlist ifaddrlist[MAX_IPADDR];

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		sprintf(errbuf, "socket: %s", mint_strerror(errno));
		return -1;
	}
	ifc.ifc_len = sizeof(ibuf);
	ifc.ifc_buf = (void *) ibuf;

	if (ioctl(fd, SIOCGIFCONF, (char *) &ifc) < 0 ||
		ifc.ifc_len < (int) sizeof(struct ifreq))
	{
		if (errno == EINVAL)
			sprintf(errbuf, "SIOCGIFCONF: ifreq struct too small (%d bytes)", (int)sizeof(ibuf));
		else
			sprintf(errbuf, "SIOCGIFCONF: %s", mint_strerror(errno));
		close(fd);
		return -1;
	}
	ifrp = ibuf;
	ifend = (struct ifreq *) ((char *) ibuf + ifc.ifc_len);

	al = ifaddrlist;
	nipaddr = 0;
	for (; ifrp < ifend; ifrp = ifnext)
	{
#ifdef HAVE_SOCKADDR_SA_LEN
		n = ifrp->ifr_addr.sa_len + sizeof(ifrp->ifr_name);
		if (n < sizeof(*ifrp))
			ifnext = ifrp + 1;
		else
			ifnext = (struct ifreq *) ((char *) ifrp + n);
		if (ifrp->ifr_addr.sa_family != AF_INET)
			continue;
#else
		ifnext = ifrp + 1;
#endif
		/*
		 * Need a template to preserve address info that is
		 * used below to locate the next entry.  (Otherwise,
		 * SIOCGIFFLAGS stomps over it because the requests
		 * are returned in a union.)
		 */
		memcpy(ifr.ifr_name, ifrp->ifr_name, IF_NAMESIZE);
		if (ioctl(fd, SIOCGIFFLAGS, (char *) &ifr) < 0)
		{
			if (errno == ENXIO)
				continue;
			sprintf(errbuf, "SIOCGIFFLAGS: %.*s: %s", (int) sizeof(ifr.ifr_name), ifr.ifr_name, mint_strerror(errno));
			close(fd);
			return -1;
		}

		/* Must be up */
		if ((ifr.ifr_flags & IFF_UP) == 0)
			continue;

		strncpy(device, ifr.ifr_name, IF_NAMESIZE);
		device[sizeof(device) - 1] = '\0';
#ifdef sun
		/* Ignore sun virtual interfaces */
		if (strchr(device, ':') != NULL)
			continue;
#endif
		ifr.ifr_dstaddr.sa_family = AF_INET;
		if (ioctl(fd, SIOCGIFADDR, (char *) &ifr) < 0)
		{
			sprintf(errbuf, "SIOCGIFADDR: %s: %s", device, mint_strerror(errno));
			close(fd);
			return -1;
		}

		if (nipaddr >= MAX_IPADDR)
		{
			sprintf(errbuf, "Too many interfaces (%d)", MAX_IPADDR);
			close(fd);
			return -1;
		}

		sin = (struct sockaddr_in *) &ifr.ifr_addr;
		al->addr = sin->sin_addr.s_addr;
		ifr.ifr_netmask.sa_family = AF_INET;
		if (ioctl(fd, SIOCGIFNETMASK, (char *) &ifr) < 0)
		{
			sprintf(errbuf, "SIOCGIFNETMASK: %s: %s", device, mint_strerror(errno));
			close(fd);
			return -1;
		}

		sin = (struct sockaddr_in *) &ifr.ifr_netmask;
		al->mask = sin->sin_addr.s_addr;
		al->device = strdup(device);
		++al;
		++nipaddr;
	}
	close(fd);

	*ipaddrp = ifaddrlist;
	return nipaddr;
}
