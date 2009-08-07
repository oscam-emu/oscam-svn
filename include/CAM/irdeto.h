#ifndef __CAM_IRDETO_H__
#  define __CAM_IRDETO_H__

#  define ADDRLEN      4        // Address length in EMM commands

int irdeto_card_init(uchar *, ushort);
int irdeto_do_ecm(ECM_REQUEST *);
int irdeto_do_emm(EMM_PACKET *);
int irdeto_card_info();

#endif // __CAM_IRDETO_H__
