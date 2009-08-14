#ifndef __READER_COMMON_H__
#  define __READER_COMMON_H__

void reader_common_card_info();
int reader_common_device_init(char *, int);
int reader_common_check_health();

int reader_common_ecm2cam(ECM_REQUEST *);
int reader_common_emm2cam(EMM_PACKET *);
int reader_common_cmd2card(uchar *, ushort, uchar *, ushort, ushort *);

#endif // __READER_COMMON_H__
