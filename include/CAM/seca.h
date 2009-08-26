#ifndef __CAM_SECA_H__
#  define __CAM_SECA_H__

int cam_seca_card_init(uchar *, ushort);
int cam_seca_load_card_info();

int cam_seca_process_ecm(ECM_REQUEST *);
int cam_seca_process_emm(EMM_PACKET *);

#endif // __CAM_SECA_H__
