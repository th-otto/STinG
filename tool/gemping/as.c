/*
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Andrew Brown.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "as.h"

#ifdef __PUREC__
char *mint_strerror(int errnum);
#else
#define mint_strerror(err) strerror(err)
#endif

#ifdef __PUREC__
#define strcasecmp(a,b)		stricmp(a,b)
#define strncasecmp(a,b,c)	strnicmp(a,b,c)
#endif

#define DEFAULT_AS_SERVER "whois.radb.net"
#undef AS_DEBUG_FILE

struct aslookup
{
	FILE *as_f;
#ifdef AS_DEBUG_FILE
	FILE *as_debug;
#endif
};

void *as_setup(const char *server)
{
	struct aslookup *asn;
	struct hostent *host;
	FILE *f;
	int s;
	struct sockaddr_in addr;
	
	s = -1;
	if (server == NULL)
		server = getenv("RA_SERVER");
	if (server == NULL)
		server = DEFAULT_AS_SERVER;

	host = gethostbyname(server);
	if (host == NULL)
	{
		fprintf(stderr, "%s: %s\n", server, hstrerror(h_errno));
		return NULL;
	}

	s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s < 0)
	{
		fprintf(stderr, "connect: %s\n", mint_strerror(errno));
		return NULL;
	}
	addr.sin_family = AF_INET;
	memcpy(&addr.sin_addr, host->h_addr, host->h_length);
	addr.sin_port = 43;
	if (connect(s, (struct sockaddr *)&addr, host->h_length) < 0)
	{
		fprintf(stderr, "connect: %s\n", mint_strerror(errno));
		return NULL;
	}

	f = fdopen(s, "r+");
	fprintf(f, "!!\n");
	fflush(f);

	asn = malloc(sizeof(struct aslookup));
	if (asn == NULL)
		fclose(f);
	else
		asn->as_f = f;

#ifdef AS_DEBUG_FILE
	if (asn)
	{
		asn->as_debug = fopen(AS_DEBUG_FILE, "w");
		if (asn->as_debug)
		{
			fprintf(asn->as_debug, ">> !!\n");
			fflush(asn->as_debug);
		}
	}
#endif

	return asn;
}


unsigned int as_lookup(void *_asn, char *addr, sa_family_t family)
{
	struct aslookup *asn = _asn;
	char buf[1024];
	unsigned int as;
	int rc;
	int dlen;
	int plen;

	as = 0;
	rc = dlen = 0;
	plen = (family == AF_INET6) ? 128 : 32;
	fprintf(asn->as_f, "!r%s/%d,l\n", addr, plen);
	fflush(asn->as_f);

#ifdef AS_DEBUG_FILE
	if (asn->as_debug)
	{
		fprintf(asn->as_debug, ">> !r%s/%d,l\n", addr, plen);
		fflush(asn->as_debug);
	}
#endif /* AS_DEBUG_FILE */

	while (fgets(buf, (int)sizeof(buf), asn->as_f) != NULL)
	{
		buf[sizeof(buf) - 1] = '\0';

#ifdef AS_DEBUG_FILE
		if (asn->as_debug)
		{
			fprintf(asn->as_debug, "<< %s", buf);
			fflush(asn->as_debug);
		}
#endif /* AS_DEBUG_FILE */

		if (rc == 0)
		{
			rc = buf[0];
			switch (rc)
			{
			case 'A':
				/* A - followed by # bytes of answer */
				sscanf(buf, "A%d\n", &dlen);
#ifdef AS_DEBUG_FILE
				if (asn->as_debug)
				{
					fprintf(asn->as_debug, "dlen: %d\n", dlen);
					fflush(asn->as_debug);
				}
#endif /* AS_DEBUG_FILE */
				break;
			case 'C':
			case 'D':
			case 'E':
			case 'F':
				/* C - no data returned */
				/* D - key not found */
				/* E - multiple copies of key */
				/* F - some other error */
				break;
			}
			if (rc == 'A')
				/* skip to next input line */
				continue;
		}

		if (dlen == 0)
			/* out of data, next char read is end code */
			rc = buf[0];
		if (rc != 'A')
			/* either an error off the bat, or a done code */
			break;

		/* data received, thank you */
		dlen -= (int)strlen(buf);

		/* origin line is the interesting bit */
		if (as == 0 && strncasecmp(buf, "origin:", 7) == 0)
		{
			sscanf(buf + 7, " AS%u", &as);
#ifdef AS_DEBUG_FILE
			if (asn->as_debug)
			{
				fprintf(asn->as_debug, "as: %d\n", as);
				fflush(asn->as_debug);
			}
#endif
		}
	}

	return as;
}


void as_shutdown(void *_asn)
{
	struct aslookup *asn = _asn;

	fprintf(asn->as_f, "!q\n");
	fclose(asn->as_f);

#ifdef AS_DEBUG_FILE
	if (asn->as_debug)
	{
		fprintf(asn->as_debug, ">> !q\n");
		fclose(asn->as_debug);
	}
#endif

	free(asn);
}
