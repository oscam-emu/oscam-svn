#ifndef __OSCAM_H__
#  define __OSCAM_H__

char *oscam_platform(char *);

void oscam_set_priority(int);

int oscam_fork(in_addr_t, in_port_t);
void oscam_wait4master();
void oscam_exit(int);

char *oscam_username(int);
int oscam_auth_client(struct s_auth *, char *);
void oscam_disconnect_client();

int oscam_chk_bcaid(ECM_REQUEST *, CAIDTAB *);

int oscam_idx_from_pid(pid_t);
int oscam_recv_from_udpipe(uchar *, int);

int oscam_write_to_pipe(int, int, uchar *, int);
int oscam_read_from_pipe(int, char **, int);

int oscam_write_ecm_answer(int, ECM_REQUEST *);

ECM_REQUEST *oscam_get_ecmtask();
int oscam_send_dcw(ECM_REQUEST *);
int oscam_process_input(uchar *, int, int);

void oscam_resolve();

void oscam_process_ecm(ECM_REQUEST *);
void oscam_process_emm(EMM_PACKET *);

#endif // __OSCAM_H__
