#ifdef __GNUC__
# define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "icmp.h"
#include <mint/mintbind.h>
#include "mintsock.h"
#undef ENOSYS
#define ENOSYS 32

#undef __set_errno
#define __set_errno(e) (errno = (e))

#ifndef howmany
# define howmany(x, y)	(((x)+((y)-1))/(y))
#endif

#define MAGIC_ONLY 0

#if !MAGIC_ONLY
static short __libc_newsockets = 1;
#endif

#if !MAGIC_ONLY
static const char *h_errlist[] =
{
	"Resolver Error 0 (no error)",
	"Unknown host",				/* 1 HOST_NOT_FOUND */
	"Host name lookup failure",		/* 2 TRY_AGAIN */
	"Unknown server error",			/* 3 NO_RECOVERY */
	"No address associated with name",	/* 4 NO_ADDRESS */
};
#define h_nerr (int)(sizeof h_errlist / sizeof h_errlist[0])


const char *hstrerror(int err)
{
	if (err < 0)
		return "Resolver internal error";
	else if (err < h_nerr)
		return h_errlist[err];
	
	return "Unknown resolver error";
}
#endif


int accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		unsigned long addrlen32;

		if (addrlen)
		{
			addrlen32 = *addrlen;
			r = (int)Faccept(fd, addr, &addrlen32);
		} else
		{
			r = (int)Faccept(fd, addr, addrlen);
		}
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int)r);
				return -1;
			}
			if (addrlen)
				*addrlen = addrlen32;
			return (int)r;
		}
		__libc_newsockets = 0;
	}
#endif

	{
		struct accept_cmd cmd;
		short addrlen16;
		
		if (addrlen)
			addrlen16 = (short) *addrlen;
		
		cmd.cmd = ACCEPT_CMD;
		cmd.addr = addr;
		cmd.addrlen = &addrlen16;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		
		if (addrlen)
			*addrlen = addrlen16;
		
		if (r < 0)
		{
			__set_errno(-(int)r);
			return -1;
		}
	}
	return (int)r;
}


int bind(int fd, const struct sockaddr *addr, socklen_t addrlen)
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		r = (int)Fbind(fd, addr, addrlen);
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int) r);
				return -1;
			}
			return 0;
		}
		__libc_newsockets = 0;
	}
#endif

	{
		struct bind_cmd cmd;
		
		cmd.addr = (struct sockaddr *)addr;
		cmd.addrlen = (short) addrlen;
		cmd.cmd = BIND_CMD;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		if (r < 0)
		{
			__set_errno(-(int) r);
			return -1;
		}
	}
	return 0;
}


int connect(int fd, const struct sockaddr *addr, socklen_t addrlen)
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		r = (int)Fconnect(fd, addr, addrlen);
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int)r);
				return -1;
			}
			return 0;
		}
		__libc_newsockets = 0;
	}
#endif

	{
		struct connect_cmd cmd;
		
		cmd.addr = (struct sockaddr *)addr;
		cmd.addrlen = (short) addrlen;
		cmd.cmd = CONNECT_CMD;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		if (r < 0)
		{
			__set_errno(-(int)r);
			return -1;
		}
	}
	return 0;
}


int socket(int domain, int type, int proto)
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		r = (int)Fsocket(domain, type, proto);
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-r);
				return -1;
			}
			return r;
		}
		__libc_newsockets = 0;
	}
#endif

	{
		struct socket_cmd cmd;
		int sockfd;

		sockfd = (int)Fopen(SOCKDEV, O_RDWR);
		if (sockfd < 0)
		{
			__set_errno (-sockfd);
			return -1;
		}
		
		cmd.cmd = SOCKET_CMD;
		cmd.domain = domain;
		cmd.type = type;
		cmd.protocol = proto;
		
		r = (int)Fcntl(sockfd, (long) &cmd, SOCKETCALL);
		if (r < 0)
		{
			__set_errno(-r);
			Fclose(sockfd);
			return -1;
		}
		
		return sockfd;
	}
}


#ifdef __MINT__
int listen(int fd, unsigned int backlog)
#else
int listen(int fd, int backlog)
#endif
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		r = (int)Flisten(fd, backlog);
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int) r);
				return -1;
			}
			return 0;
		}
		__libc_newsockets = 0;
	}
#endif
	
	{
		struct listen_cmd cmd;
		
		cmd.cmd = LISTEN_CMD;
		cmd.backlog = backlog;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		if (r < 0)
		{
			__set_errno(-(int)r);
			return -1;
		}
	}
	return 0;
}


int sendto(int fd, const void *buf, size_t buflen, int flags, const struct sockaddr *addr, socklen_t addrlen)
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		r = (int)Fsendto(fd, buf, buflen, flags, addr, addrlen);
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int)r);
				return -1;
			}
			return (int)r;
		}
		__libc_newsockets = 0;
	}
#endif

	{
		struct sendto_cmd cmd;
		
		cmd.cmd = addr ? SENDTO_CMD : SEND_CMD;
		cmd.buf = buf;
		cmd.buflen = buflen;
		cmd.flags = flags;
		cmd.addr = addr;
		cmd.addrlen = (short) addrlen;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		if (r < 0)
		{
			__set_errno(-(int)r);
			return -1;
		}
	}
	return (int)r;
}


int send(int fd, const void *buf, size_t buflen, int flags)
{
	return sendto(fd, buf, buflen, flags, NULL, 0);
}


int recvfrom(int fd, void *buf, size_t buflen, int flags, struct sockaddr *addr, socklen_t *addrlen)
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		unsigned long addrlen32;

		if (addrlen)
		{
			addrlen32 = *addrlen;
			r = (int)Frecvfrom(fd, buf, buflen, flags, addr, &addrlen32);
		} else
		{
			r = (int)Frecvfrom(fd, buf, buflen, flags, addr, addrlen);
		}
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int)r);
				return -1;
			}
			if (addrlen)
				*addrlen = addrlen32;
			return (int)r;
		}
		__libc_newsockets = 0;
	}
#endif

	{
		struct recvfrom_cmd cmd;
		short addrlen16;
		
		if (addrlen)
			addrlen16 = (short) *addrlen;
		
		cmd.cmd = addr ? RECVFROM_CMD : RECV_CMD;
		cmd.buf = buf;
		cmd.buflen = buflen;
		cmd.flags = flags;
		cmd.addr = addr;
		cmd.addrlen = &addrlen16;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		
		if (addrlen)
			*addrlen = addrlen16;
		
		if (r < 0)
		{
			__set_errno(-(int)r);
			return -1;
		}
	}
	return (int)r;
}


int recv(int fd, void *buf, size_t buflen, int flags)
{
	return recvfrom(fd, buf, buflen, flags, NULL, 0);
}


int poll(struct pollfd *fds, unsigned long int nfds, int32_t __timeout)
{
	long int retval;
	unsigned long timeout = (unsigned long) __timeout;

	if (__timeout < 0)
	{
		timeout = ~0;
	}

	retval = Fpoll(fds, nfds, timeout);
	if (retval != -ENOSYS)
	{
		if (retval < 0)
		{
			__set_errno(-(int)retval);
			return -1;
		}
	} else
	{
		/* We must emulate the call via Fselect ().  First task is to
		   set up the file descriptor masks.    */
		long rfds = 0;
		long wfds = 0;
		long xfds = 0;
		unsigned long int i;
		struct pollfd *pfds = fds;

		for (i = 0; i < nfds; i++)
		{
			pfds[i].revents = 0;

			/* Older than 1.19 can't do more than 32 file descriptors.
			 * And we'd only get here if we're a very old kernel anyway.
			 */
			if (pfds[i].fd >= 32)
			{
				pfds[i].revents = POLLNVAL;
				continue;
			}
#define LEGAL_FLAGS (POLLIN | POLLPRI | POLLOUT | POLLRDNORM | POLLWRNORM | POLLRDBAND | POLLWRBAND)

			if ((pfds[i].events | LEGAL_FLAGS) != LEGAL_FLAGS)
			{
				pfds[i].revents = POLLNVAL;
				continue;
			}

			if (pfds[i].events & (POLLIN | POLLRDNORM))
				rfds |= (1L << (pfds[i].fd));
			if (pfds[i].events & POLLPRI)
				xfds |= (1L << (pfds[i].fd));
			if (pfds[i].events & (POLLOUT | POLLWRNORM))
				wfds |= (1L << (pfds[i].fd));
		}

		if (__timeout < 0)
		{
			retval = Fselect(0L, &rfds, &wfds, &xfds);
		} else if (timeout == 0)
		{
			retval = Fselect(1L, &rfds, &wfds, &xfds);
		} else if (timeout < USHRT_MAX)
		{
			/* The manpage Fselect(2) says that timeout is
			   signed.  But it is really unsigned.  */
			retval = Fselect(timeout, &rfds, &wfds, &xfds);
		} else
		{
			/* Thanks to the former kernel hackers we have
			   to loop in order to simulate longer timeouts
			   than USHRT_MAX.  */
			unsigned long saved_rfds;
			unsigned long saved_wfds;
			unsigned long saved_xfds;
			unsigned short int this_timeout;
			int last_round = 0;

			saved_rfds = rfds;
			saved_wfds = wfds;
			saved_xfds = xfds;

			do
			{
				if ((unsigned long) timeout > USHRT_MAX)
					this_timeout = USHRT_MAX;
				else
				{
					this_timeout = timeout;
					last_round = 1;
				}

				retval = Fselect(this_timeout, &rfds, &wfds, &xfds);

				if (retval != 0)
					break;

				timeout -= this_timeout;

				/* I don't know whether we can rely on the
				   masks not being clobbered on timeout.  */
				rfds = saved_rfds;
				wfds = saved_wfds;
				xfds = saved_xfds;
			} while (!last_round);
		}

		/* Now fill in the results in struct pollfd.    */
		for (i = 0; i < nfds; i++)
		{
			/* Older than 1.19 can't do more than 32 file descriptors. */
			if (pfds[i].fd >= 32)
				continue;
			if (rfds & (1L << (pfds[i].fd)))
				pfds[i].revents |= (pfds[i].events & (POLLIN | POLLRDNORM));
			if (wfds & (1L << (pfds[i].fd)))
				pfds[i].revents |= (pfds[i].events & (POLLOUT | POLLWRNORM));
			if (xfds & (1L << (pfds[i].fd)))
				pfds[i].revents |= (pfds[i].events & POLLPRI);
		}

		if (retval < 0)
		{
			__set_errno(-(int)retval);
			return -1;
		}
	}

	return (int)retval;
}


/* Check the first NFDS descriptors each in READFDS (if not NULL) for read
   readiness, in WRITEFDS (if not NULL) for write readiness, and in EXCEPTFDS
   (if not NULL) for exceptional conditions.  If TIMEOUT is not NULL, time out
   after waiting the interval specified therein.  Returns the number of ready
   descriptors, or -1 for errors.  

   The function is currently emulated by means of poll().  This is 
   sub-optimal as long as NFDS is less or equal 32 because then we 
   waste quite some time by copying the file descriptor masks into
   struct poll.  Better poll will only be called when the native
   Fselect is not able to handle the call.  For the time being I want
   to test poll() and therefore ignore this problem.  
   
   If poll() has set POLLERR, POLLHUP, POLLNVAL or POLLMSG for any of the 
   polled descriptors, we simply mark that descriptor as ready for reading,
   writing or urgent reading so that the caller will get informed.  */
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	struct pollfd pfds[FD_SETSIZE];
	int i;
	long retval;
	long msec_timeout;
	int saved_errno = errno;

	if (nfds < 0 || nfds > FD_SETSIZE)
	{
		__set_errno(EINVAL);
		return -1;
#if 0
	} else if (nfds <= 32)
	{
		/* Implement a native call to Fselect.  */
#endif
	}

	memset(pfds, 0, nfds * sizeof *pfds);

	/* Three loops are more efficient than one here.  */
	if (readfds != NULL)
	{
		for (i = 0; i < nfds; i++)
			if (FD_ISSET(i, readfds))
			{
				pfds[i].fd = i;
				pfds[i].events |= POLLIN;
			}
	}
	
	if (exceptfds != NULL)
	{
		for (i = 0; i < nfds; i++)
			if (FD_ISSET(i, exceptfds))
			{
				pfds[i].fd = i;
				pfds[i].events |= POLLPRI;
			}
	}

	if (writefds != NULL)
	{
		for (i = 0; i < nfds; i++)
			if (FD_ISSET(i, writefds))
			{
				pfds[i].fd = i;
				pfds[i].events |= POLLOUT;
			}
	}

	if (timeout == NULL)
	{
		msec_timeout = -1;
	} else
	{
		msec_timeout = timeout->tv_sec * 1000;
		msec_timeout += (timeout->tv_usec + 999) / 1000;
	}

	retval = poll(pfds, nfds, msec_timeout);

	if (retval < 0)
	{
		return (int)retval;
	} else
	{
		unsigned int sz = (unsigned int)howmany(nfds, NFDBITS) * (unsigned int)sizeof(fd_mask);

		if (readfds)
			memset(readfds, 0, sz);
		if (exceptfds)
			memset(exceptfds, 0, sz);
		if (writefds)
			memset(writefds, 0, sz);

		if (retval)
			for (i = 0; i < nfds; i++)
			{
				if (pfds[i].revents & (POLLIN | POLLRDNORM | POLLRDBAND))
					if (readfds != NULL)
						FD_SET(pfds[i].fd, readfds);
				if (pfds[i].revents & POLLPRI)
					if (exceptfds != NULL)
						FD_SET(pfds[i].fd, exceptfds);
				if (pfds[i].revents & (POLLOUT | POLLWRNORM | POLLWRBAND))
					if (writefds != NULL)
						FD_SET(pfds[i].fd, writefds);
				if (pfds[i].revents & (POLLERR | POLLNVAL | POLLHUP | POLLMSG))
				{
					if (readfds != NULL && FD_ISSET(pfds[i].fd, readfds))
						FD_SET(pfds[i].fd, readfds);
					if (exceptfds != NULL && FD_ISSET(pfds[i].fd, exceptfds))
						FD_SET(pfds[i].fd, exceptfds);
					if (writefds != NULL && FD_ISSET(pfds[i].fd, writefds))
						FD_SET(pfds[i].fd, writefds);
				}
			}
	}

	__set_errno(saved_errno);
	return (int)retval;
}


int usleep(__useconds_t dt)
{
	clock_t t;
	clock_t tt;

	tt = ((clock_t) dt) / (((clock_t) 1000000UL) / CLOCKS_PER_SEC);
	t = clock();
	while ((clock() - t) < tt)
		;
	return 0;
}


/*
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
in_addr_t inet_aton(const char *cp, struct in_addr *addr)
{
	in_addr_t val;
	int base;
	int n;
	unsigned char c;
	unsigned long parts[4];
	unsigned long *pp = parts;

	c = *cp;
	for (;;)
	{
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		if (!isdigit(c))
			return 0;
		base = 10;
		if (c == '0')
		{
			c = *++cp;
			if (c == 'x' || c == 'X')
				base = 16, c = *++cp;
			else
				base = 8;
		}
		val = 0;
		for (;;)
		{
			if (isascii(c) && isdigit(c))
			{
				val = (val * base) + (c - '0');
				c = *++cp;
			} else if (base == 16 && isascii(c) && isxdigit(c))
			{
				val = (val << 4) |
					(c + 10 - (islower(c) ? 'a' : 'A'));
				c = *++cp;
			} else
				break;
		}
		if (c == '.')
		{
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16 bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3)
				return 0;
			*pp++ = val;
			c = *++cp;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (c != '\0' && (!isascii(c) || !isspace(c)))
		return 0;
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = (int)(pp - parts + 1);
	switch (n)
	{
	case 0:
		return 0;		/* initial nondigit */

	case 1:				/* a -- 32 bits */
		break;

	case 2:				/* a.b -- 8.24 bits */
		if (parts[0] > 0xff || val > 0xffffffUL)
			return 0;
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		if (parts[0] > 0xff || parts[1] > 0xff || val > 0xffffUL)
			return 0;
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if (parts[0] > 0xff || parts[1] > 0xff || parts[2] > 0xff || val > 0xff)
			return 0;
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
	if (addr)
		addr->s_addr = htonl(val);

	return 1;
}


/*
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 */
in_addr_t inet_addr(const char *cp)
{
	struct in_addr val;

	if (inet_aton(cp, &val))
		return val.s_addr;
	return INADDR_NONE;
}


char *inet_ntoa(struct in_addr in)
{
	static char b[18];
	unsigned char *p;

	p = (unsigned char *)&in.s_addr;
	(void)sprintf(b, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
	return b;
}


uid_t getuid(void)
{
	uid_t id = Pgetuid();
	if (id == (uid_t)-ENOSYS)
		id = 0;
	return id;
}


int setuid(uid_t id)
{
	long r = Psetuid(id);
	if (r == 0 || r == -ENOSYS)
		return 0;
	return -1;
}


gid_t getgid(void)
{
	gid_t id = Pgetgid();
	if (id == (gid_t)-ENOSYS)
		id = 0;
	return id;
}


int setgid(gid_t id)
{
	long r = Psetgid(id);
	if (r == 0 || r == -ENOSYS)
		return 0;
	return -1;
}


int shutdown(int fd, int how)
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		r = (int)Fshutdown(fd, how);
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-r);
				return -1;
			}
			return 0;
		}
		__libc_newsockets = 0;
	}
#endif
	
	{
		struct shutdown_cmd cmd;

		cmd.cmd = SHUTDOWN_CMD;
		cmd.how = how;

		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		if (r < 0)
		{
			__set_errno(-r);
			return -1;
		}
	}
	return 0;
}


int setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		r = (int)Fsetsockopt(fd, level, optname, optval, optlen);
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int)r);
				return -1;
			}
			return 0;
		}
		__libc_newsockets = 0;
	}
#endif

	{
		struct setsockopt_cmd cmd;
		
		cmd.cmd = SETSOCKOPT_CMD;
		cmd.level = level;
		cmd.optname = optname;
		cmd.optval = (void *)optval;
		cmd.optlen = optlen;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		if (r < 0)
		{
			__set_errno(-(int)r);
			return -1;
		}
	}
	return 0;
}


int getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen)
{
	int r;
	unsigned long optlen32;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{

		if (optlen)
		{
			optlen32 = *optlen;
			r = (int)Fgetsockopt(fd, level, optname, optval, &optlen32);
		} else
		{
			r = (int)Fgetsockopt(fd, level, optname, optval, optlen);
		}
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int)r);
				return -1;
			}
			if (optlen)
				*optlen = optlen32;
			return 0;
		}
		__libc_newsockets = 0;
	}
#endif
	
	{
		struct getsockopt_cmd cmd;
		
		if (optlen)
			optlen32 = *optlen;
		
		cmd.cmd = GETSOCKOPT_CMD;
		cmd.level = level;
		cmd.optname = optname;
		cmd.optval = optval;
		cmd.optlen = (void *) &optlen32;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		
		if (optlen)
			*optlen = optlen32;
		
		if (r < 0)
		{
			__set_errno(-(int)r);
			return -1;
		}
		return 0;
	}
}


int sendmsg(int fd, const struct msghdr *msg, int flags)
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		r = (int)Fsendmsg(fd, msg, flags);
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int)r);
				return -1;
			}
			return (int)r;
		}
		__libc_newsockets = 0;
	}
#endif

	{
		struct sendmsg_cmd cmd;
		
		cmd.msg = msg;
		cmd.cmd = SENDMSG_CMD;
		cmd.flags = flags;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		if (r < 0)
		{
			__set_errno(-(int) r);
			return -1;
		}
	}
	return (int)r;
}


int recvmsg(int fd, struct msghdr *msg, int flags)
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		r = (int)Frecvmsg(fd, msg, flags);
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int)r);
				return -1;
			}
			return (int)r;
		}
		__libc_newsockets = 0;
	}
#endif

	{
		struct recvmsg_cmd cmd;
		
		cmd.cmd = RECVMSG_CMD;
		cmd.msg = msg;
		cmd.flags = flags;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		
		if (r < 0)
		{
			__set_errno(-(int)r);
			return -1;
		}
	}
	return (int)r;
}


int getpeername(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		unsigned long addrlen32;

		if (addrlen)
		{
			addrlen32 = *addrlen;
			r = (int)Fgetpeername(fd, addr, &addrlen32);
		} else
		{
			r = (int)Fgetpeername(fd, addr, addrlen);
		}
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int)r);
				return -1;
			}
			if (addrlen)
				*addrlen = addrlen32;
			return 0;
		}
		__libc_newsockets = 0;
	}
#endif
	
	{
		struct getpeername_cmd cmd;
		short addrlen16;
		
		if (addrlen)
			addrlen16 = (short) *addrlen;
		
		cmd.cmd = GETPEERNAME_CMD;
		cmd.addr = addr;
		cmd.addrlen = &addrlen16;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		
		if (addrlen)
			*addrlen = addrlen16;
		if (r < 0)
		{
			__set_errno(-(int) r);
			return -1;
		}
	}
	return 0;
}


int getsockname(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		unsigned long addrlen32;

		addrlen32 = *addrlen;
		r = (int)Fgetsockname(fd, addr, &addrlen32);
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int)r);
				return -1;
			}
			*addrlen = addrlen32;
			return 0;
		}
		__libc_newsockets = 0;
	}
#endif

	{
		struct getsockname_cmd cmd;
		short addrlen16;
		
		if (addrlen)
			addrlen16 = (short) *addrlen;
		
		cmd.cmd = GETSOCKNAME_CMD;
		cmd.addr = addr;
		cmd.addrlen = &addrlen16;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		
		if (addrlen)
			*addrlen = addrlen16;

		if (r < 0)
		{
			__set_errno(-(int) r);
			return -1;
		}
	}
	return 0;
}


int socketpair(int domain, int type, int proto, int fds[2])
{
#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		short _fds[2];
		int r;

		r = (int)Fsocketpair(domain, type, proto, _fds);
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int)r);
				return -1;
			}
			fds[0] = _fds[0];
			fds[1] = _fds[1];
			return 0;
		}
		__libc_newsockets = 0;
	}
#endif

	{
		struct socketpair_cmd cmd;
		int sockfd1, sockfd2;
		
		sockfd1 = (int) Fopen(SOCKDEV, 2);
		if (sockfd1 < 0)
		{
			__set_errno(-sockfd1);
			return -1;
		}
		
		cmd.cmd = SOCKETPAIR_CMD;
		cmd.domain = domain;
		cmd.type = type;
		cmd.protocol = proto;
		
		sockfd2 = (int) Fcntl(sockfd1, (long) &cmd, SOCKETCALL);
		if (sockfd2 < 0)
		{
			__set_errno(-sockfd2);
			Fclose(sockfd1);
			return -1;
		}
		
		fds[0] = sockfd1;
		fds[1] = sockfd2;
	}

	return 0;
}


int gettimeofday(struct timeval *tp, struct timezone *tzp)
{
	unsigned short tos_time;
	unsigned short tos_date;
	struct tm now;
	static int have_Tgettimeofday = 1;

	(void)tzp;
	if (have_Tgettimeofday != 0)
	{
		long retval;

		/*
		 * MiNTs timezone structure is different than ours
		 */
		struct __mint_timezone minttz;
		retval = Tgettimeofday(tp, &minttz);
		if (retval == -ENOSYS)
		{
			have_Tgettimeofday = 0;
		} else if (retval < 0)
		{
			__set_errno(-(int)retval);
			return -1;
		} else
		{
			if (tzp != NULL)
			{
				tzp->tz_minuteswest = (int)minttz.tz_minuteswest;
				tzp->tz_dsttime = (int)minttz.tz_dsttime;
			}
			return 0;
		}
	}									/* have_Tgettimeofday != 0 */

	/* Don't use `else' here.  The have_Tgettimeofday flag may have
	 * changed and we want to fall back to the emulation.
	 */
	tos_time = Tgettime();
	tos_date = Tgetdate();

	now.tm_sec = (tos_time & 0x1f) * 2;
	now.tm_min = (tos_time >> 5) & 0x3f;
	now.tm_hour = tos_time >> 11;
	now.tm_mday = tos_date & 0x1f;
	now.tm_mon = ((tos_date >> 5) & 0xf) - 1;
	now.tm_year = (tos_date >> 9) + 80;
	now.tm_isdst = -1;					/* Dunno.  */

	if (tp != NULL)
	{
		tp->tv_sec = mktime(&now);
		tp->tv_usec = (clock() * (1000000UL / CLOCKS_PER_SEC)) % 2000000UL;
		if (tp->tv_usec >= 1000000L)
		{
			tp->tv_usec -= 1000000L;
			tp->tv_sec++;
		}
	}

	return 0;
}


int settimeofday(const struct timeval *tp, const struct timezone *tzp)
{
	int retval = 0;
	static int have_Tsettimeofday = 1;

	if (have_Tsettimeofday != 0)
	{
		/*
		 * MiNTs timezone structure is different than ours
		 */
		struct __mint_timezone minttz;
		struct __mint_timezone *minttzp = NULL;

		if (tzp != NULL)
		{
			minttz.tz_minuteswest = tzp->tz_minuteswest;
			minttz.tz_dsttime = tzp->tz_dsttime;
			minttzp = &minttz;
		}
		retval = (int)Tsettimeofday(tp, minttzp);
		if (retval == -ENOSYS)
		{
			have_Tsettimeofday = 0;
		} else if (retval < 0)
		{
			if (retval == -EACCES)
				retval = -EPERM;
			__set_errno(-retval);
			return -1;
		} else
		{
			return 0;
		}
	}

	/* Fall through to emulation. */
	if (tp != NULL)
	{
		struct tm then;
		short tos_time;
		short tos_date;

#if 0
		if (tzp != NULL)
		{
#ifdef __PUREC__
			timezone = -tzp->tz_minuteswest * 60;
#else
			timezone = tzp->tz_minuteswest * 60;
#endif
		}
#endif

		/* The T[gs]et(date|time) system calls always expect local
		 * times.
		 */
		then = *localtime((time_t *) &tp->tv_sec);

		if (then.tm_year < 80)
		{
			__set_errno(EINVAL);
			return -1;
		}
		
		tos_time = then.tm_sec / 2;
		/* I'm not sure if TOS can handle leapseconds.  Better take care.  */
		if (tos_time > 29)
			tos_time = 29;
		tos_time |= ((then.tm_min << 5) | (then.tm_hour << 11));
		tos_date = (then.tm_mday | ((then.tm_mon + 1) << 5) | ((then.tm_year - 80) << 9));

		retval = (int)Tsettime(tos_time);
		if (retval < 0)
		{
			if (retval == -1)
				retval = -EINVAL;
			__set_errno(-retval);
			return -1;
		}
		retval = (int)Tsetdate(tos_date);
		if (retval < 0)
		{
			if (retval == -1)
				retval = -EINVAL;
			__set_errno(-retval);
			return -1;
		}
	}

	return 0;
}


#ifdef __PUREC__
#define _FIOREAD	0x01
#define _FIOWRITE	0x02
#define _FIOUNBUF	0x04
#define _FIOBUF		0x08
#define _FIOEOF		0x10
#define _FIOERR		0x20
#define _FIODIRTY	0x40
#define _FIOBIN		0x80

FILE *fdopen(int fd, const char *mode)
{
	int i;
	int f = 0;
	int b;
	FILE *fp = NULL;
	extern FILE _FilTab[FOPEN_MAX];

	while (*mode)
	{
		switch (*mode++)
		{
		case 'r':
			f |= _FIOREAD | _FIOBUF;
			break;
		case 'w':
			f |= _FIOWRITE | _FIOBUF;
			break;
		case 'a':
			f |= _FIOWRITE | _FIOBUF;
			break;
		case '+':
			f |= (_FIOREAD | _FIOWRITE | _FIOBUF);
			break;
		case 'b':
			f |= _FIOBIN | _FIOBUF;
			break;
		case 't':
			f &= ~_FIOBIN | _FIOBUF;
			break;
		default:
			__set_errno(EINVAL);
			return NULL;
		}
	}

	if (fd == 0)
		return stdin;
	if (fd == 1)
		return stdout;
	if (fd == 2)
		return stderr;

	for (i = 0; !fp && i < FOPEN_MAX; ++i)
		if (!(_FilTab[i].Flags & (_FIOREAD|_FIOWRITE)))   /* empty slot? */
			fp = &_FilTab[i];

	if (!fp)
	{
		__set_errno(EMFILE);
		return NULL;
	}

	if ((i = (f & (_FIOREAD|_FIOWRITE))) == 0)
	{
		__set_errno(EINVAL);
		return NULL;
	}

	if (isatty(fd))
	{
		b  = _IONBF;
		f |= _FIOUNBUF;
	} else
	{
		b  = _IOFBF;
	}
	fp->Handle = fd;			/* file handle */
	fp->Flags = f;			/* file status flags */
	fp->Mode = b;
	fp->ungetFlag = 0;
	if ((fp->BufStart = malloc(BUFSIZ)) == NULL)
	{
		return NULL;
	}
	fp->BufPtr = fp->BufLvl = fp->BufStart;
	fp->BufEnd = (char *)(fp->BufStart) + BUFSIZ;

	return fp;
}
#endif


/*
 * Converts the current herrno error value into an EAI_* error code.
 * That error code is normally returned by getnameinfo() or getaddrinfo().
 */
static int gai_error_from_herrno(void)
{
	switch (h_errno)
	{
	case HOST_NOT_FOUND:
		return EAI_NONAME;

	case NO_ADDRESS:
#if NO_ADDRESS != NO_DATA
	case NO_DATA:
#endif
		return EAI_NODATA;

	case NO_RECOVERY:
		return EAI_FAIL;

	case TRY_AGAIN:
		return EAI_AGAIN;
	}
	return EAI_SYSTEM;
}


/*
 * Internal function that builds an addrinfo struct.
 */
static struct addrinfo *makeaddrinfo(
	int af, int type, int proto,
	const struct sockaddr *addr, size_t addrlen, const char *canonname)
{
	struct addrinfo *res;
	size_t size;
	
	size = sizeof(*res) + addrlen;
	if (canonname != NULL)
		size += strlen(canonname) + 1;
	res = (struct addrinfo *) malloc(size);
	if (res != NULL)
	{
		res->ai_flags = 0;
		res->ai_family = af;
		res->ai_socktype = type;
		res->ai_protocol = proto;
		res->ai_addrlen = addrlen;
		res->ai_canonname = NULL;
		res->ai_next = NULL;
		res->ai_addr = (struct sockaddr *)(res + 1);
		
		memcpy(res->ai_addr, addr, addrlen);

		if (canonname != NULL)
		{
			res->ai_canonname = (char *)res->ai_addr + sizeof(struct addrinfo) + addrlen;
			strcpy(res->ai_canonname, canonname);
		}
	}
	
	return res;
}


static struct addrinfo *makeipv4info(int type, int proto, uint32_t ip, uint16_t port, const char *name)
{
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
#ifdef HAVE_STRUCT_SOCKADDR_SIN_LEN
	addr.sin_len = sizeof(addr);
#endif
	addr.sin_port = port;
	addr.sin_addr.s_addr = ip;

	return makeaddrinfo(AF_INET, type, proto, (struct sockaddr *) &addr, sizeof(addr), name);
}


/*
 * getaddrinfo() non-thread-safe IPv4-only implementation
 * Address-family-independent hostname to address resolution.
 *
 * This is meant for IPv6-unaware systems that do probably not provide
 * getaddrinfo(), but still have old function gethostbyname().
 *
 * Only UDP and TCP over IPv4 are supported here.
 */
int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res)
{
	struct addrinfo *info;
	uint32_t ip;
	uint16_t port;
	int protocol = 0;
	int flags = 0;
	const char *name = NULL;

	*res = NULL;

	if (hints != NULL)
	{
		flags = hints->ai_flags;

		if (flags & ~(AI_PASSIVE|AI_CANONNAME|AI_NUMERICHOST))
			return EAI_BADFLAGS;
		/* only accept AF_INET and AF_UNSPEC */
		if (hints->ai_family && (hints->ai_family != AF_INET))
			return EAI_FAMILY;

		/* protocol sanity check */
		switch (hints->ai_socktype)
		{
		case SOCK_STREAM:
			protocol = IPPROTO_TCP;
			break;

		case SOCK_DGRAM:
			protocol = IPPROTO_UDP;
			break;

#ifdef SOCK_RAW
		case SOCK_RAW:
#endif
		case 0:
			break;

		default:
			return EAI_SOCKTYPE;
		}
		if (hints->ai_protocol && protocol && protocol != hints->ai_protocol)
			return EAI_SERVICE;
	}

	/* default values */
	if (node == NULL)
	{
		if (flags & AI_PASSIVE)
			ip = htonl(INADDR_ANY);
		else
			ip = htonl(INADDR_LOOPBACK);
	} else if ((ip = inet_addr(node)) == INADDR_NONE)
	{
		struct hostent *entry = NULL;

		/* hostname resolution */
		if (!(flags & AI_NUMERICHOST))
			entry = gethostbyname(node);

		if (entry == NULL)
			return gai_error_from_herrno();

		if (entry->h_length != 4 || entry->h_addrtype != AF_INET)
			return EAI_FAMILY;

		ip = *((uint32_t *) entry->h_addr);
		if (flags & AI_CANONNAME)
			name = entry->h_name;
	}

	if ((flags & AI_CANONNAME) && name == NULL)
		name = node;

	/* service resolution */
	if (service == NULL)
	{
		port = 0;
	} else
	{
		unsigned long d;
		char *end;
		struct servent *sp;

		d = strtoul(service, &end, 0);
		if (!end[0])
		{
			if (d > 65535UL)
				return EAI_SERVICE;
			port = htons((unsigned short) d);
		} else
		{
			sp = getservbyname(service, NULL);
			if (sp == NULL)
				return EAI_SERVICE;
			port = sp->s_port;
		}
	}

	/* building results... */
	if (protocol == 0 || protocol == IPPROTO_UDP)
	{
		info = makeipv4info(SOCK_DGRAM, IPPROTO_UDP, ip, port, name);
		if (info == NULL)
		{
			__set_errno(ENOMEM);
			return EAI_SYSTEM;
		}
		if (flags & AI_PASSIVE)
			info->ai_flags |= AI_PASSIVE;
		*res = info;
	}
	if (protocol == 0 || protocol == IPPROTO_TCP)
	{
		info = makeipv4info(SOCK_STREAM, IPPROTO_TCP, ip, port, name);
		if (info == NULL)
		{
			__set_errno(ENOMEM);
			return EAI_SYSTEM;
		}
		info->ai_next = *res;
		if (flags & AI_PASSIVE)
			info->ai_flags |= AI_PASSIVE;
		*res = info;
	}

	return 0;
}
