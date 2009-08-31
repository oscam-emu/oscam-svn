#ifndef __CAM_VIDEOGUARD_H__
#  define __CAM_VIDEOGUARD_H__

int cam_videoguard_detect(uchar *, ushort);
int cam_videoguard_load_card();

int cam_videoguard_process_ecm(ECM_REQUEST *);
int cam_videoguard_process_emm(EMM_PACKET *);

#endif // __CAM_VIDEOGUARD_H__
