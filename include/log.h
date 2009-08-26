#ifndef __LOG_H__
#  define __LOG_H__

int cs_init_log(char *);
void cs_log(char *, ...);
void cs_debug(char *, ...);
void cs_ddump(uchar *, int, char *, ...);
void cs_close_log();
int cs_init_statistics(char *);
void cs_statistics(int);
void cs_dump(uchar *, int, char *, ...);
void cs_write_log(char *);

#endif // __LOG_H__
