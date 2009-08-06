#ifndef __CAM_SECA_H__
#  define __CAM_SECA_H__

int seca_card_init(uchar *, int);
int seca_do_ecm(ECM_REQUEST *);
int seca_do_emm(EMM_PACKET *);
int seca_card_info();

#endif // __CAM_SECA_H__
