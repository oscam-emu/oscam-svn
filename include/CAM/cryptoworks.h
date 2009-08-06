#ifndef __CAM_CRYPTOWORKS_H__
#  define __CAM_CRYPTOWORKS_H__

int cryptoworks_card_init(uchar *, int);
int cryptoworks_do_ecm(ECM_REQUEST *);
int cryptoworks_do_emm(EMM_PACKET *);
int cryptoworks_card_info();

#endif // __CAM_CRYPTOWORKS_H__
