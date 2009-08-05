#ifndef __READER_H__
#  define __READER_H__

extern void cs_ri_brk(int);
extern void cs_ri_log(char *, ...);
extern void start_cardreader(void);
extern void reader_card_info(void);

#endif // __READER_H__
