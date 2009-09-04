#ifndef __AC_H__
#  define __AC_H__

#  ifdef CS_ANTICASC

#  include <stdio.h>

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

void ac_init_stat(int);
int ac_init_log(char *);
void ac_do_stat();
void ac_init_client(struct s_auth *);
void ac_chk(ECM_REQUEST *, int);
#  endif

#endif // __AC_H__
