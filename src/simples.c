#include "globals.h"
#include "simples.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

#ifdef NO_FTIME
#  include <sys/time.h>
#endif

static int get_hex_value(char c)
{
	if ((c >= '0') && (c <= '9'))
		return (c - '0');
	if ((c >= 'A') && (c <= 'F'))
		return (c - 'A' + 10);
	if ((c >= 'a') && (c <= 'f'))
		return (c - 'a' + 10);

	return -1;
}

char *trim(char *txt)
{
	register int l;
	register char *p1, *p2;

	if (*txt == ' ') {
		for (p1 = p2 = txt; (*p1 == ' ') || (*p1 == '\t') || (*p1 == '\n') || (*p1 == '\r'); p1++);
		while (*p1)
			*p2++ = *p1++;
		*p2 = '\0';
	}
	if ((l = strlen(txt)) > 0)
		for (p1 = txt + l - 1; (*p1 == ' ') || (*p1 == '\t') || (*p1 == '\n') || (*p1 == '\r'); *p1-- = '\0');

	return txt;
}

char *strtolower(char *txt)
{
	char *p;

	for (p = txt; *p; p++)
		*p = tolower(*p);

	return txt;
}

char *strtoupper(char *txt)
{
	char *p;

	for (p = txt; *p; p++)
		*p = toupper(*p);

	return txt;
}

int cs_atob(uchar * buf, char *asc, int n)
{
	int i, rc;

	for (i = 0; i < n; i++) {
		if ((rc = (get_hex_value(asc[i << 1]) << 4) | get_hex_value(asc[(i << 1) + 1])) & 0x100)
			return (-1);
		buf[i] = rc;
	}

	return n;
}

ulong cs_atoi(char *asc, int l, int val_on_err)
{
	int i, n = 0;
	ulong rc = 0;

	for (i = ((l - 1) << 1), errno = 0; (i >= 0) && (n < 4); i -= 2) {
		int b;

		b = (get_hex_value(asc[i]) << 4) | get_hex_value(asc[i + 1]);
		if (b < 0) {
			errno = EINVAL;
			rc = (val_on_err) ? 0xFFFFFFFF : 0;
			break;
		}
		rc |= b << (n << 3);
		n++;
	}

	return rc;
}

int byte_atob(char *asc)
{
	int rc;

	if (strlen(trim(asc)) != 2) {
		rc = -1;
	} else if ((rc = (get_hex_value(asc[0]) << 4) | get_hex_value(asc[1])) & 0x100) {
		rc = -1;
	}

	return rc;
}

long word_atob(char *asc)
{
	long rc;

	if (strlen(trim(asc)) != 4) {
		rc = -1;
	} else {
		rc = get_hex_value(asc[0]) << 12 | get_hex_value(asc[1]) << 8 | get_hex_value(asc[2]) << 4 | get_hex_value(asc[3]);
		if (rc & 0x10000) {
			rc = -1;
		}
	}

	return rc;
}

int key_atob(char *asc, uchar * bin)
{
	int i, n1, n2, rc;

	for (i = rc = 0; i < 32; i += 2) {
		if ((n1 = get_hex_value(asc[i])) < 0)
			rc = -1;
		if ((n2 = get_hex_value(asc[i + 1])) < 0)
			rc = -1;
		bin[i >> 1] = (n1 << 4) + (n2 & 0xff);
	}

	return rc;
}

int key_atob14(char *asc, uchar * bin)
{
	int i, n1, n2, rc;

	for (i = rc = 0; i < 28; i += 2) {
		if ((n1 = get_hex_value(asc[i])) < 0)
			rc = -1;
		if ((n2 = get_hex_value(asc[i + 1])) < 0)
			rc = -1;
		bin[i >> 1] = (n1 << 4) + (n2 & 0xff);
	}

	return rc;
}

char *key_btoa(char *asc, uchar * bin)
{
	int i;			//, n1, n2, rc;
	static char buf[33];

	if (!asc) {
		asc = buf;
	}
	for (i = 0; i < 16; i++) {
		sprintf(asc + (i << 1), "%02X", bin[i]);
	}

	return asc;
}

char *cs_hexdump(int m, uchar * buf, int n)
{
	int i = 0;
	static char dump[520];

	dump[i] = '\0';
	m = (m) ? 3 : 2;
	if (m * n >= sizeof (dump))
		n = (sizeof (dump) / m) - 1;
	while (i < n)
		sprintf(dump + (m * i++), "%02X%s", *buf++, (m > 2) ? " " : "");
	if (m == 3) dump[(3 * i) - 1] = '\0';

	return dump;
}

ulong b2i(int n, uchar * b)
{
	switch (n) {
		case 2:
			return 0xFFFF & ((b[0] << 8) | b[1]);
		case 3:
			return 0xFFFFFF & ((b[0] << 16) | (b[1] << 8) | b[2]);
		case 4:
			return 0xFFFFFFFF & ((b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3]);
	}

	return 0;
}

ullong b2ll(int n, uchar * b)
{
	int i;
	ullong k = 0;

	for (i = 0; i < n; k += b[i++])
		k <<= 8;

	return k;
}

uchar *i2b(int n, ulong i)
{
	static uchar b[4];

	switch (n) {
		case 2:
			b[0] = (i >> 8) & 0xff;
			b[1] = (i) & 0xff;
			break;
		case 3:
			b[0] = (i >> 16) & 0xff;
			b[1] = (i >> 8) & 0xff;
			b[2] = (i) & 0xff;
		case 4:
			b[0] = (i >> 24) & 0xff;
			b[1] = (i >> 16) & 0xff;
			b[2] = (i >> 8) & 0xff;
			b[3] = (i) & 0xff;
			break;
	}

	return b;
}

ulong a2i(char *asc, int bytes)
{
	int i, n;
	ulong rc;

	for (rc = i = 0, n = strlen(trim(asc)) - 1; i < (abs(bytes) << 1); n--, i++) {
		if (n >= 0) {
			int rcl;

			if ((rcl = get_hex_value(asc[n])) < 0) {
				errno = EINVAL;
				return (0x1F1F1F);
			}
			rc |= (rcl << (i << 2));
		} else if (bytes < 0) {
			rc |= (0xf << (i << 2));
		}
	}
	errno = 0;

	return rc;
}

int boundary(int exp, int n)
{
	return ((((n - 1) >> exp) + 1) << exp);
}

void cs_ftime(struct timeb *tp)
{
#ifdef NO_FTIME
	struct timeval tv;

	gettimeofday(&tv, (struct timezone *) 0);
	tp->time = tv.tv_sec;
	tp->millitm = tv.tv_usec / 1000;
#else
	ftime(tp);
#endif
}

void cs_sleepms(int msec)
{
	struct timeval tv;

	tv.tv_sec = msec / 1000;
	tv.tv_usec = (msec % 1000) * 1000;
	select(0, 0, 0, 0, &tv);
}

int file_exists (const char *filepath) {
	FILE *file = fopen(filepath, "r");

	if (file != NULL) {
		fclose(file);
		return 1;
	}

	return 0;
}
