#ifndef __SIMPLES_H__
#  define __SIMPLES_H__

extern void aes_set_key(char *);
extern void aes_encrypt_idx(int, uchar *, int);
extern void aes_decrypt(uchar *, int);

#  define aes_encrypt(b, n) aes_encrypt_idx(cs_idx, b, n)

extern char *remote_txt(void);
extern char *trim(char *);
extern char *strtolower(char *);
extern int gethexval(char);
extern int cs_atob(uchar *, char *, int);
extern ulong cs_atoi(char *, int, int);
extern int byte_atob(char *);
extern long word_atob(char *);
extern int key_atob(char *, uchar *);
extern int key_atob4(char *, uchar *);
extern char *key_btoa(char *, uchar *);
extern char *cs_hexdump(int, uchar *, int);
extern in_addr_t cs_inet_order(in_addr_t);
extern char *cs_inet_ntoa(in_addr_t);
extern in_addr_t cs_inet_addr(char *txt);
extern ulong b2i(int, uchar *);
extern ullong b2ll(int, uchar *);
extern uchar *i2b(int, ulong);
extern ulong a2i(char *, int);
extern int boundary(int, int);
extern void cs_ftime(struct timeb *);
extern void cs_sleepms(int);
extern int bytes_available(int);
extern void cs_setpriority(int);
extern struct s_auth *find_user(char *);

#endif // __SIMPLES_H__
