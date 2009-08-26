#ifndef __MD5_H__
#  define __MD5_H__

#  define MD5_DIGEST_LENGTH 16
char *__md5_crypt(const char *, const char *);
unsigned char *MD5(const unsigned char *, unsigned long, unsigned char *);

#endif // __MD5_H__
