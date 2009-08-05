#ifndef __LOG_H__
#  define __LOG_H__

extern int cs_init_log(char *);
extern void cs_log(char *, ...);
extern void cs_debug(char *, ...);
extern void cs_ddump(uchar *, int, char *, ...);
extern void cs_close_log(void);
extern int cs_init_statistics(char *);
extern void cs_statistics(int);
extern void cs_dump(uchar *, int, char *, ...);

#endif // __LOG_H__
