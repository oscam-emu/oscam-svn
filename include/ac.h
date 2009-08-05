#ifndef __AC_H__
#  define __AC_H__

#  ifdef CS_ANTICASC
struct s_acasc_shm {
	ushort count:15;
	ushort deny:1;
};

struct s_acasc {
	ushort stat[10];
	uchar idx;		// current active index in stat[]
};

struct s_cpmap {
	ushort caid;
	ulong provid;
	ushort sid;
	ushort chid;
	ushort dwtime;
	struct s_cpmap *next;
};

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
