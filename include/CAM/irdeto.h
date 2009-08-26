#ifndef __CAM_IRDETO_H__
#  define __CAM_IRDETO_H__

int cam_irdeto_card_init(uchar *, ushort);
int cam_irdeto_load_card_info();

int cam_irdeto_process_ecm(ECM_REQUEST *);
int cam_irdeto_process_emm(EMM_PACKET *);

#endif // __CAM_IRDETO_H__
