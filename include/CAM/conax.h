#ifndef __CAM_CONAX_H__
#  define __CAM_CONAX_H__

int conax_card_init(uchar *, ushort);
int conax_do_ecm(ECM_REQUEST *);
int conax_do_emm(EMM_PACKET *);
int conax_card_info();

#endif // __CAM_CONAX_H__
