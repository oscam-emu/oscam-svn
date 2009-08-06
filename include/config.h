#ifndef __CONFIG_H__
#  define __CONFIG_H__

int init_config();
int init_userdb();
int init_readerdb();
int init_sidtab();
int init_srvid();
int search_boxkey(ushort, ulong, char *);
void init_len4caid();
int init_irdeto_guess_tab();

#  ifdef CS_ANTICASC
//void start_anticascader();
void init_ac();
#  endif

#endif // __CONFIG_H__
