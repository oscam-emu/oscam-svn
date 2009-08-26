#ifndef __CAM_CRYPTOWORKS_H__
#  define __CAM_CRYPTOWORKS_H__

int cam_cryptoworks_card_init(uchar *, ushort);
int cam_cryptoworks_load_card_info();

int cam_cryptoworks_process_ecm(ECM_REQUEST *);
int cam_cryptoworks_process_emm(EMM_PACKET *);

#endif // __CAM_CRYPTOWORKS_H__
