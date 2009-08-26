#ifndef __READER_COMMON_H__
#  define __READER_COMMON_H__

#  define CARD_INSERTED  1
#  define CARD_NEED_INIT 2
#  define CARD_FAILURE   4  

int reader_common_init(struct s_reader *);
void reader_common_card_info(struct s_reader *reader);
int reader_common_check_health(struct s_reader *reader);

int reader_common_ecm2cam(struct s_reader *reader, ECM_REQUEST *);
int reader_common_emm2cam(struct s_reader *reader, EMM_PACKET *);
int reader_common_cmd2card(struct s_reader *reader, uchar *, ushort, uchar *, ushort, ushort *);

#endif // __READER_COMMON_H__
