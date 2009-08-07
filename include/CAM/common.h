#ifndef __CAM_COMMON_H__
#  define __CAM_COMMON_H__

#  define SCT_LEN(sct) (3+((sct[1]&0x0f)<<8)+sct[2])

#  define CMD_LEN 5
#  define MAX_LEN 256

#define cam_common_read_cmd(cmd, data) (cam_common_send_cmd(cmd, data, 0) == 0)
#define cam_common_write_cmd(cmd, data) (cam_common_send_cmd(cmd, data, 1) == 0)

void cam_common_card_info();
int cam_common_get_cardsystem(uchar *, int);

int cam_common_ecm(ECM_REQUEST *);
int cam_common_emm(EMM_PACKET *);
int cam_common_send_ins(const uchar *, const uchar *, const int);

ulong chk_provid(uchar *, ushort caid);
void guess_irdeto(ECM_REQUEST *);
void guess_cardsystem(ECM_REQUEST *);

ulong chk_provid(uchar *, ushort);
void guess_cardsystem(ECM_REQUEST *);
//void guess_irdeto(ECM_REQUEST *);

#endif // __CAM_COMMON_H__
