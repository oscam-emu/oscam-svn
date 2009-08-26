#ifndef __CAM_VIACCESS_H__
#  define __CAM_VIACCESS_H__

int cam_viaccess_card_init(uchar *, ushort);
int cam_viaccess_load_card_info();

int cam_viaccess_process_ecm(ECM_REQUEST *);
int cam_viaccess_process_emm(EMM_PACKET *);

#endif // __CAM_VIACCESS_H__
