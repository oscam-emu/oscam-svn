#ifndef __OSCAM_H__
#  define __OSCAM_H__

extern char *cs_platform(char *);
extern int recv_from_udpipe(uchar *, int);
extern char *username(int);
extern int idx_from_pid(pid_t);
extern int chk_bcaid(ECM_REQUEST *, CAIDTAB *);
extern void cs_exit(int sig);
extern int cs_fork(in_addr_t, in_port_t);
extern void wait4master(void);
extern int cs_auth_client(struct s_auth *, char *);
extern void cs_disconnect_client(void);
extern int check_ecmcache(ECM_REQUEST *, ulong);
extern int write_to_pipe(int, int, uchar *, int);
extern int read_from_pipe(int, uchar **, int);
extern int write_ecm_request(int, ECM_REQUEST *);
extern int write_ecm_answer(int, ECM_REQUEST *);
extern void log_emm_request(int);
extern void get_cw(ECM_REQUEST *);
extern void do_emm(EMM_PACKET *);
extern ECM_REQUEST *get_ecmtask(void);
extern void request_cw(ECM_REQUEST *, int, int);
extern int send_dcw(ECM_REQUEST *);
extern int process_input(uchar *, int, int);
extern int chk_srvid(ECM_REQUEST *, int);
extern int chk_sfilter(ECM_REQUEST *, PTAB *);
extern int chk_ufilters(ECM_REQUEST *);
extern int chk_rfilter(ECM_REQUEST *, struct s_reader *);
extern int chk_rsfilter(ECM_REQUEST *, int);
extern int chk_avail_reader(ECM_REQUEST *, struct s_reader *);
extern void set_signal_handler(int, int, void (*)(int));
extern void cs_log_config(void);

extern struct timeval *chk_pending(struct timeb tp_ctimeout);

#endif // __OSCAM_H__
