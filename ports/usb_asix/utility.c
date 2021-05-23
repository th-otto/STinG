/*
 * utility.c: library replacement functions
 *
 * Copyright Roger Burrows (June 2018)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * IMPORTANT: you must compile with default short ints because the
 * STinG & USB APIs expect this ...
 */
#include <limits.h>
#if INT_MAX != 32767
#error you must compile with short ints!
#endif

#ifdef __PUREC__
#include <tos.h>
#else
#include <osbind.h>
#endif
#include <stdarg.h>
#include <string.h>


int memcmp(const void *s1, const void *s2, size_t n)
{
	unsigned char *p = (unsigned char *) s1;
	unsigned char *q = (unsigned char *) s2;
	int r;

	while (n--)
	{
		r = *p++ - *q++;
		if (r)
			return r;
	}

	return 0;
}

void *memcpy(void *dest, const void *source, size_t n)
{
	char *dst = (char *) dest;
	const char *src = (const char *) source;

	while (n--)
		*dst++ = *src++;

	return dest;
}


void *memset(void *s, int c, size_t n)
{
	char *p = (char *) s;

	while (n--)
		*p++ = c;

	return s;
}


char *strcat(char *dst, const char *src)
{
	char *p;

	for (p = dst; *p; p++)
		;
	while ((*p++ = *src++) != 0)
		;

	return dst;
}

int strcmp(const char *s1, const char *s2)
{
	const unsigned char *p = (const unsigned char *) s1;
	const unsigned char *q = (const unsigned char *) s2;
	int r;

	while (*p || *q)
	{
		r = *p++ - *q++;
		if (r)
			return r;
	}

	return 0;
}


char *strcpy(char *dest, const char *src)
{
	char *p = dest;

	while (*src)
		*p++ = *src++;

	return dest;
}


size_t strlen(const char *s)
{
	char *p = (char *) s;
	size_t n = 0;

	while (*p++)
		n++;

	return n;
}

#ifdef ENABLE_DEBUG
/*
 * flags used in processing format string
 */
#define PR_LJ   0x01					/* left justify */
#define PR_CA   0x02					/* use A-F instead of a-f for hex */
#define PR_SG   0x04					/* signed numeric conversion (%d vs. %u) */
#define PR_32   0x08					/* long (32-bit) numeric conversion */
#define PR_16   0x10					/* short (16-bit) numeric conversion */
#define PR_WS   0x20					/* PR_SG set and num was < 0 */
#define PR_LZ   0x40					/* pad left with '0' instead of ' ' */

/* largest number handled is 2^32-1, lowest radix handled is 8.
2^32-1 in base 8 has 11 digits (add 5 for trailing NUL and for slop) */
#define PR_BUFLEN   16

typedef int (*fnptr_t)(unsigned c, void **helper);

/*****************************************************************************
name:   do_printf
action: minimal subfunction for ?printf, calls function
    'fn' with arg 'ptr' for each character to be output
returns:total number of characters output
*****************************************************************************/
static int do_printf(const char *fmt, va_list args, fnptr_t fn, void *ptr)
{
	unsigned int flags;
	unsigned int actual_wd;
	unsigned int count;
	unsigned int given_wd;
	unsigned char *where;
	unsigned char buf[PR_BUFLEN];
	unsigned char state;
	unsigned char radix;
	long num;

	state = flags = count = given_wd = 0;
/* begin scanning format specifier list */
	for (; *fmt; fmt++)
	{
		switch (state)
		{
/* STATE 0: AWAITING % */
		case 0:
			if (*fmt != '%')			/* not %... */
			{
				fn(*fmt, &ptr);			/* ...just echo it */
				count++;
				break;
			}
/* found %, get next char and advance state to check if next char is a flag */
			state++;
			fmt++;
			if (*fmt == '\0')
				break;
			/* FALL THROUGH */
/* STATE 1: AWAITING FLAGS (%-0) */
		case 1:
			if (*fmt == '%')			/* %% */
			{
				fn(*fmt, &ptr);
				count++;
				state = flags = given_wd = 0;
				break;
			}
			if (*fmt == '-')
			{
				if (flags & PR_LJ)		/* %-- is illegal */
					state = flags = given_wd = 0;
				else
					flags |= PR_LJ;
				break;
			}
			/* for now, just ignore some flag characters */
			if ((*fmt == '+') || (*fmt == '#') || (*fmt == ' '))
			{
				fmt++;
				if (*fmt == '\0')
					break;
			}
/* not a flag char: advance state to check if it's field width */
			state++;
/* check now for '%0...' */
			if (*fmt == '0')
			{
				flags |= PR_LZ;
				fmt++;
				if (*fmt == '\0')
					break;
			}
			/* FALL THROUGH */
/* STATE 2: AWAITING (NUMERIC) FIELD WIDTH */
		case 2:
			if (*fmt >= '0' && *fmt <= '9')
			{
				given_wd = 10 * given_wd + (*fmt - '0');
				break;
			}
/* not field width: advance state to check if it's a modifier */
			state++;
			/* FALL THROUGH */
/* STATE 3: AWAITING MODIFIER CHARS (lh) */
		case 3:
			if (*fmt == 'l')
			{
				flags |= PR_32;
				break;
			}
			if (*fmt == 'h')
			{
				flags |= PR_16;
				break;
			}
/* not modifier: advance state to check if it's a conversion char */
			state++;
			/* FALL THROUGH */
/* STATE 4: AWAITING CONVERSION CHARS (Xxpndiuocs) */
		case 4:
			where = buf + PR_BUFLEN - 1;
			*where = '\0';
			switch (*fmt)
			{
			case 'X':
				flags |= PR_CA;
				/* FALL THROUGH */
			case 'x':
			case 'p':
			case 'n':
				radix = 16;
				if (*fmt == 'p')
				{
					fn('0', &ptr);		/* insert '0x' */
					fn('x', &ptr);
					count += 2;
					flags |= PR_32;		/* pointers are always 32-bit on 68k */
				}
				goto DO_NUM;
			case 'd':
			case 'i':
				flags |= PR_SG;
				/* FALL THROUGH */
			case 'u':
				radix = 10;
				goto DO_NUM;
			case 'o':
				radix = 8;
/* load the value to be printed. l=long=32 bits: */
			  DO_NUM:if (flags & PR_32)
				{
					if (flags & PR_SG)
						num = va_arg(args, long);

					else
						num = va_arg(args, unsigned long);
				}
/* h or nothing: sizeof(int) bits (signed or unsigned) */
				else
				{
					if (flags & PR_SG)
						num = va_arg(args, int);

					else
						num = va_arg(args, unsigned int);
				}
/* take care of sign */
				if (flags & PR_SG)
				{
					if (num < 0)
					{
						flags |= PR_WS;
						num = -num;
					}
				}
/* convert binary to octal/decimal/hex ASCII
OK, I found my mistake. The math here is _always_ unsigned */
				do
				{
					unsigned short temp;

					temp = (unsigned long) num % radix;
					where--;
					if (temp < 10)
						*where = temp + '0';
					else if (flags & PR_CA)
						*where = temp - 10 + 'A';
					else
						*where = temp - 10 + 'a';
					num = (unsigned long) num / radix;
				}
				while (num != 0);
				goto EMIT;
			case 'c':
/* disallow pad-left-with-zeroes for %c */
				flags &= ~PR_LZ;
				where--;
				*where = (unsigned char) va_arg(args, int);

				actual_wd = 1;
				goto EMIT2;
			case 's':
/* disallow pad-left-with-zeroes for %s */
				flags &= ~PR_LZ;
				where = va_arg(args, unsigned char *);

				if (!where)
					where = (unsigned char *) "(null)";
			  EMIT:
				actual_wd = strlen((const char *) where);
				if (flags & PR_WS)
					actual_wd++;
/* if we pad left with ZEROES, do the sign now */
				if ((flags & (PR_WS | PR_LZ)) == (PR_WS | PR_LZ))
				{
					fn('-', &ptr);
					count++;
				}
/* pad on left with spaces or zeroes (for right justify) */
			  EMIT2:if ((flags & PR_LJ) == 0)
				{
					while (given_wd > actual_wd)
					{
						fn(flags & PR_LZ ? '0' : ' ', &ptr);
						count++;
						given_wd--;
					}
				}
/* if we pad left with SPACES, do the sign now */
				if ((flags & (PR_WS | PR_LZ)) == PR_WS)
				{
					fn('-', &ptr);
					count++;
				}
/* emit string/char/converted number */
				while (*where != '\0')
				{
					fn(*where++, &ptr);
					count++;
				}
/* pad on right with spaces (for left justify) */
				if (given_wd < actual_wd)
					given_wd = 0;
				else
					given_wd -= actual_wd;
				for (; given_wd; given_wd--)
				{
					fn(' ', &ptr);
					count++;
				}
				break;
			default:
				break;
			}
		default:
			state = flags = given_wd = 0;
			break;
		}
		if (*fmt == '\0')				/* misformed format string */
			break;
	}
	return count;
}

/*****************************************************************************
SPRINTF
*****************************************************************************/
static int vsprintf_help(unsigned c, void **ptr)
{
	char *dst;

	dst = *ptr;
	*dst++ = (char) c;
	*ptr = dst;
	return 0;
}

/*****************************************************************************
*****************************************************************************/
int vsprintf(char *buf, const char *fmt, va_list args)
{
	int rv;

	rv = do_printf(fmt, args, vsprintf_help, (void *) buf);
	buf[rv] = '\0';
	return rv;
}

/*****************************************************************************
*****************************************************************************/
int sprintf(char *buf, const char *fmt, ...)
{
	va_list args;
	int rv;

	va_start(args, fmt);
	rv = vsprintf(buf, fmt, args);
	va_end(args);
	return rv;
}

/*****************************************************************************
PRINTF
You must write your own my_putchar()
*****************************************************************************/
static void my_putchar(unsigned c)
{
	if (c == '\n')
		(void) Bconout(2, '\r');
	(void) Bconout(2, c);
}

int vprintf_help(unsigned c, void **ptr)
{
	my_putchar(c);
	return 0;
}

/*****************************************************************************
*****************************************************************************/
int vprintf(const char *fmt, va_list args)
{
	return do_printf(fmt, args, vprintf_help, NULL);
}

/*****************************************************************************
*****************************************************************************/
int printf(const char *fmt, ...)
{
	va_list args;
	int rv;

	va_start(args, fmt);
	rv = vprintf(fmt, args);
	va_end(args);
	return rv;
}
#endif
