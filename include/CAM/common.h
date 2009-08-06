#ifndef __CAM_COMMON_H__
#  define __CAM_COMMON_H__

#  define SCT_LEN(sct) (3+((sct[1]&0x0f)<<8)+sct[2])
#  define MAX_LEN      256

void cam_common_card_info();
int cam_common_get_cardsystem(void);
int cam_common_ecm(ECM_REQUEST *);
int cam_common_emm(EMM_PACKET *);
ulong chk_provid(uchar *, ushort caid);
void guess_irdeto(ECM_REQUEST *);
void guess_cardsystem(ECM_REQUEST *);

extern ulong chk_provid(uchar *, ushort);
extern void guess_cardsystem(ECM_REQUEST *);
//extern void guess_irdeto(ECM_REQUEST *);

#endif // __CAM_COMMON_H__
