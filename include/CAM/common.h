#ifndef __CAM_COMMON_H__
#  define __CAM_COMMON_H__

#  define SCT_LEN(sct) (3+((sct[1]&0x0f)<<8)+sct[2])

int cam_common_detect_card_system(uchar *, ushort);
void cam_common_card_info();

int cam_common_process_ecm(ECM_REQUEST *);
int cam_common_process_emm(EMM_PACKET *);

int cam_common_cmd2card(uchar *, ushort, uchar *, ushort, ushort *);

ulong chk_provid(uchar *, ushort caid);
void guess_irdeto(ECM_REQUEST *);
void guess_cardsystem(ECM_REQUEST *);

ulong chk_provid(uchar *, ushort);
void guess_cardsystem(ECM_REQUEST *);
//void guess_irdeto(ECM_REQUEST *);

#endif // __CAM_COMMON_H__
