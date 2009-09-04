#ifndef __SIMPLES_H__
#  define __SIMPLES_H__

#include <netdb.h>

char *trim(char *);
char *strtolower(char *);
char *strtoupper(char *);
int cs_atob(uchar *, char *, int);
ulong cs_atoi(char *, int, int);
int byte_atob(char *);
long word_atob(char *);
int key_atob(char *, uchar *);
int key_atob14(char *, uchar *);
char *key_btoa(char *, uchar *);
char *cs_hexdump(int, uchar *, int);
ulong b2i(int, uchar *);
ullong b2ll(int, uchar *);
uchar *i2b(int, ulong);
ulong a2i(char *, int);
int boundary(int, int);
void cs_ftime(struct timeb *);
void cs_sleepms(int);
int bytes_available(int);
int file_exists (const char *);

#endif // __SIMPLES_H__
