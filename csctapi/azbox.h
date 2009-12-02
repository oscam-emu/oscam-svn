
#ifndef AZBOX_H_
#define AZBOX_H_

#include "openxcas/openxcas_api.h"
#include "openxcas/openxcas_message.h"
#include "openxcas/openxcas_smartcard.h"
#include "azbox.h"

int xcas_sc_open(void);
int xcas_sc_read(int rfd, uchar *buf, int l);
int xcas_sc_write(int rfd, uchar *buf, int l);
int xcas_sc_get_atr(int rfd, uchar *atr);
int xcas_sc_close(int rfd);

#endif /* AZBOX_H_ */
