#ifndef __CAM_VIACCESS_H__
#  define __CAM_VIACCESS_H__

int viaccess_card_init(uchar *, int);
int viaccess_do_ecm(ECM_REQUEST *);
int viaccess_do_emm(EMM_PACKET *);
int viaccess_card_info();

#endif // __CAM_VIACCESS_H__
