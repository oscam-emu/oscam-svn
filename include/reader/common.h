#ifndef __READER_COMMON_H__
#  define __READER_COMMON_H__

int reader_common_doapi(uchar, uchar *, int, int);
int reader_common_chkicc(uchar *, int);
int reader_common_cmd2api(uchar *, int);
int reader_common_cmd2icc(uchar *, int);

void reader_common_card_info();
int reader_common_device_init(char *, int);
int reader_common_checkhealth();
int reader_common_ecm(ECM_REQUEST *);
int reader_common_emm(EMM_PACKET *);

extern uchar cta_cmd[272], cta_res[260];
extern ushort cta_lr;

extern uchar atr[64];
extern ushort atr_size;

#endif // __READER_COMMON_H__
