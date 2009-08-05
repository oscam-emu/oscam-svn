#ifndef __CAM_CRYPTOWORKS_H__
#  define __CAM_CRYPTOWORKS_H__

extern int cryptoworks_card_init(uchar *, int);
extern int cryptoworks_do_ecm(ECM_REQUEST *);
extern int cryptoworks_do_emm(EMM_PACKET *);
extern int cryptoworks_card_info(void);

#endif // __CAM_CRYPTOWORKS_H__
