#ifndef __READER_COMMON_H__
#  define __READER_COMMON_H__

int reader_common_init(struct s_reader *);
void reader_common_card_info();
int reader_common_check_health();

int reader_common_ecm2cam(ECM_REQUEST *);
int reader_common_emm2cam(EMM_PACKET *);
int reader_common_cmd2card(uchar *, ushort, uchar *, ushort, ushort *);

#endif // __READER_COMMON_H__
