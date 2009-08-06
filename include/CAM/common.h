#ifndef __CAM_COMMON_H__
#  define __CAM_COMMON_H__

#  include <ctapi.h>
#  include <ctbcs.h>

#  define ADDRLEN      4	// Address length in EMM commands
#  define MAX_PROV     16
#  define SCT_LEN(sct) (3+((sct[1]&0x0f)<<8)+sct[2])
#  define MAX_LEN      256

extern int reader_device_init(char *, int);
extern int reader_checkhealth(void);
extern int reader_ecm(ECM_REQUEST *);
extern int reader_emm(EMM_PACKET *);

extern uchar cta_cmd[], cta_res[];
extern ushort cta_lr;

extern ulong chk_provid(uchar *, ushort);
extern void guess_cardsystem(ECM_REQUEST *);
//extern void guess_irdeto(ECM_REQUEST *);

#endif // __CAM_COMMON_H__
