#ifndef __OSCAM_H__
#  define __OSCAM_H__

char *oscam_platform(char *);
int oscam_recv_from_udpipe(uchar *, int);
char *oscam_username(int);
int oscam_idx_from_pid(pid_t);
int oscam_chk_bcaid(ECM_REQUEST *, CAIDTAB *);
void oscam_exit(int);
int oscam_fork(in_addr_t, in_port_t);
void oscam_wait4master();
int oscam_auth_client(struct s_auth *, char *);
void oscam_disconnect_client();
int oscam_check_ecmcache(ECM_REQUEST *, ulong);
int oscam_write_to_pipe(int, int, uchar *, int);
int oscam_read_from_pipe(int, char **, int);
int oscam_write_ecm_request(int, ECM_REQUEST *);
int oscam_write_ecm_answer(int, ECM_REQUEST *);
ECM_REQUEST *oscam_get_ecmtask();
void oscam_request_cw(ECM_REQUEST *, int, int);
int oscam_send_dcw(ECM_REQUEST *);
int oscam_process_input(uchar *, int, int);
void oscam_set_signal_handler(int, int, void (*)(int));
void oscam_set_priority(int);
struct timeval *oscam_chk_pending(struct timeb tp_ctimeout);
void oscam_resolve();

void oscam_process_ecm(ECM_REQUEST *);
void oscam_process_emm(EMM_PACKET *);

void oscam_log_emm_request(int);
void oscam_log_config();
void oscam_store_logentry(char *);

#endif // __OSCAM_H__
