#ifndef __CAM_SECA_H__
#  define __CAM_SECA_H__

extern int seca_card_init(uchar *, int);
extern int seca_do_ecm(ECM_REQUEST *);
extern int seca_do_emm(EMM_PACKET *);
extern int seca_card_info(void);

#endif // __CAM_SECA_H__
