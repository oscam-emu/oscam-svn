#ifndef __CONFIG_H__
#  define __CONFIG_H__

int config_init();
int config_init_userdb();
int config_init_readerdb();
int config_init_sidtab();
int config_init_srvid();

#  ifdef CS_ANTICASC
void config_init_ac();
#  endif

void config_init_cam_common_len4caid();
int config_search_boxkey(ushort, ulong, char *);

#endif // __CONFIG_H__
