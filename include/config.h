#ifndef __CONFIG_H__
#  define __CONFIG_H__

extern int init_config(void);
extern int init_userdb(void);
extern int init_readerdb(void);
extern int init_sidtab(void);
extern int init_srvid(void);
extern int search_boxkey(ushort, ulong, char *);
extern void init_len4caid(void);
extern int init_irdeto_guess_tab(void);

#  ifdef CS_ANTICASC
//extern void start_anticascader(void);
extern void init_ac(void);
#  endif

#endif // __CONFIG_H__
