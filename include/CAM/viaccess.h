#ifndef __CAM_VIACCESS_H__
#  define __CAM_VIACCESS_H__

extern int viaccess_card_init(uchar *, int);
extern int viaccess_do_ecm(ECM_REQUEST *);
extern int viaccess_do_emm(EMM_PACKET *);
extern int viaccess_card_info(void);

#endif // __CAM_VIACCESS_H__
