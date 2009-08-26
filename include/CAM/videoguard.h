#ifndef __CAM_VIDEOGUARD_H__
#  define __CAM_VIDEOGUARD_H__

int cam_videoguard_card_init(uchar *, ushort);
int cam_videoguard_load_card_info();

int cam_videoguard_process_ecm(ECM_REQUEST *);
int cam_videoguard_process_emm(EMM_PACKET *);

#endif // __CAM_VIDEOGUARD_H__
