#ifndef __READER_COMMON_H__
#  define __READER_COMMON_H__

void reader_common_card_info();
int reader_common_device_init(char *, int);
int reader_common_check_health();

int reader_common_send_ecm(ECM_REQUEST *);
int reader_common_send_emm(EMM_PACKET *);
int reader_common_send_cmd(uchar *, int);

extern uchar cta_cmd[272], cta_res[260];
extern ushort cta_lr;

extern uchar atr[64];
extern ushort atr_size;

#endif // __READER_COMMON_H__
