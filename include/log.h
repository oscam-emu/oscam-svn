#ifndef __LOG_H__
#  define __LOG_H__

int log_init(char *);

void log_normal(char *, ...);
void log_debug(char *, ...);
void log_dump(uchar *, int, char *, ...);
void log_ddump(uchar *, int, char *, ...);
void log_write(char *);
void log_close();

int log_init_statistics(char *);
void log_statistics(int);

#endif // __LOG_H__
