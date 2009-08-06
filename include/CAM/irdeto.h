#ifndef __CAM_IRDETO_H__
#  define __CAM_IRDETO_H__

#  define ADDRLEN      4        // Address length in EMM commands

extern int irdeto_card_init(uchar *, int);
extern int irdeto_do_ecm(ECM_REQUEST *);
extern int irdeto_do_emm(EMM_PACKET *);
extern int irdeto_card_info(void);

#endif // __CAM_IRDETO_H__
