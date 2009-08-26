#ifndef __CAM_CONAX_H__
#  define __CAM_CONAX_H__

int cam_conax_card_init(uchar *, ushort);
int cam_conax_load_card_info();

int cam_conax_process_ecm(ECM_REQUEST *);
int cam_conax_process_emm(EMM_PACKET *);

#endif // __CAM_CONAX_H__
