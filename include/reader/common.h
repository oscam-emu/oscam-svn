#ifndef __READER_COMMON_H__
#  define __READER_COMMON_H__

extern int reader_device_init(char *, int);
extern int reader_checkhealth(void);
extern int reader_ecm(ECM_REQUEST *);
extern int reader_emm(EMM_PACKET *);

extern uchar cta_cmd[272], cta_res[260];
extern ushort cta_lr;

extern uchar atr[64];
extern ushort atr_size;

#endif // __READER_COMMON_H__
