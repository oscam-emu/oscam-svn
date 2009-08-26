#ifndef __OSCAM_H__
#  define __OSCAM_H__

char *cs_platform(char *);
int recv_from_udpipe(uchar *, int);
char *username(int);
int idx_from_pid(pid_t);
int chk_bcaid(ECM_REQUEST *, CAIDTAB *);
void cs_exit(int);
int cs_fork(in_addr_t, in_port_t);
void wait4master();
int cs_auth_client(struct s_auth *, char *);
void cs_disconnect_client();
int check_ecmcache(ECM_REQUEST *, ulong);
int write_to_pipe(int, int, uchar *, int);
int read_from_pipe(int, char **, int);
int write_ecm_request(int, ECM_REQUEST *);
int write_ecm_answer(int, ECM_REQUEST *);
void log_emm_request(int);
void get_cw(ECM_REQUEST *);
void do_emm(EMM_PACKET *);
ECM_REQUEST *get_ecmtask();
void request_cw(ECM_REQUEST *, int, int);
int send_dcw(ECM_REQUEST *);
int process_input(uchar *, int, int);
int chk_srvid(ECM_REQUEST *, int);
int chk_sfilter(ECM_REQUEST *, PTAB *);
int chk_ufilters(ECM_REQUEST *);
int chk_rfilter(ECM_REQUEST *, struct s_reader *);
int chk_rsfilter(ECM_REQUEST *, int);
int chk_avail_reader(ECM_REQUEST *, struct s_reader *);
void set_signal_handler(int, int, void (*)(int));
void cs_log_config();
struct timeval *chk_pending(struct timeb tp_ctimeout);
void store_logentry(char *);
void cs_resolve();

#endif // __OSCAM_H__
