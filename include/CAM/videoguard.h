#ifndef __CAM_VIDEOGUARD_H__
#  define __CAM_VIDEOGUARD_H__

int videoguard_card_init(uchar *, ushort);
int videoguard_do_ecm(ECM_REQUEST *);
int videoguard_do_emm(EMM_PACKET *);
int videoguard_card_info();

#endif // __CAM_VIDEOGUARD_H__
