#ifndef __CAM_VIDEOGUARD_H__
#  define __CAM_VIDEOGUARD_H__

extern int videoguard_card_init(uchar *, int);
extern int videoguard_do_ecm(ECM_REQUEST *);
extern int videoguard_do_emm(EMM_PACKET *);
extern int videoguard_card_info(void);

#endif // __CAM_VIDEOGUARD_H__
