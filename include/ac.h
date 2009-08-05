#ifndef __AC_H__
#  define __AC_H__

#  ifdef CS_ANTICASC

extern struct s_acasc_shm *acasc;
extern FILE *fpa;
extern int use_ac_log;

extern void ac_init_stat(int);
extern int ac_init_log(char *);
extern void ac_do_stat(void);
extern void ac_init_client(struct s_auth *);
extern void ac_chk(ECM_REQUEST *, int);
#  endif

#endif // __AC_H__
