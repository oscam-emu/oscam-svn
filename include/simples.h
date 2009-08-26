#ifndef __SIMPLES_H__
#  define __SIMPLES_H__

void aes_set_key(char *);
void aes_encrypt_idx(int, uchar *, int);
void aes_decrypt(uchar *, int);

#  define aes_encrypt(b, n) aes_encrypt_idx(cs_idx, b, n)

char *remote_txt();
char *trim(char *);
char *strtolower(char *);
int gethexval(char);
int cs_atob(uchar *, char *, int);
ulong cs_atoi(char *, int, int);
int byte_atob(char *);
long word_atob(char *);
int key_atob(char *, uchar *);
int key_atob14(char *, uchar *);
char *key_btoa(char *, uchar *);
char *cs_hexdump(int, uchar *, int);
in_addr_t cs_inet_order(in_addr_t);
char *cs_inet_ntoa(in_addr_t);
in_addr_t cs_inet_addr(char *txt);
ulong b2i(int, uchar *);
ullong b2ll(int, uchar *);
uchar *i2b(int, ulong);
ulong a2i(char *, int);
int boundary(int, int);
void cs_ftime(struct timeb *);
void cs_sleepms(int);
int bytes_available(int);
void cs_setpriority(int);
struct s_auth *find_user(char *);
int file_exists (const char *);

#endif // __SIMPLES_H__
