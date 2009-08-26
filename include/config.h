#ifndef __CONFIG_H__
#  define __CONFIG_H__

int init_config();
int init_userdb();
int init_readerdb();
int init_sidtab();
int init_srvid();
void init_cam_common_len4caid();

int search_boxkey(ushort, ulong, char *);

#  ifdef CS_ANTICASC
void init_ac();
#  endif

#endif // __CONFIG_H__
