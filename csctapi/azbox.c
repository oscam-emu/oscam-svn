#include "azbox.h"

#ifdef AZBOX
#include <string.h>

int xcas_sc_reset(int rfd, int mode)
{
  int n;
  unsigned char buf[512];

  bzero(buf, sizeof(buf));

  buf[0] = 3;
  buf[1] = mode & 0xff;

  cs_debug("azbox: performing reset...");

  if ((n = ioctl(rfd, SCARD_IOC_WARMRESET, &buf)) < 0) {
//    cs_log("azbox: card reset failed (fd = %d, mode = %d)", rfd, mode);
  }

//  cs_dump("azbox: card reset ok (%d), mode = %d", n, mode);
  return n;
}

int xcas_sc_open(void)
{
  int n, rfd = -1;
  unsigned char buf[512];

  bzero(buf, sizeof(buf));

  // init openxcas
  if (openxcas_open_with_smartcard("oscam") < 0) {
    cs_log("azbox: failed to open openxcas");
    return -1;
  }

  openxcas_debug_message_onoff(1);
  sleep(2);

  // open device
   if ((rfd = openxcas_get_smartcard_device(0)) < 0) {
     cs_log("azbox: failed to open scard dev");
     return -1;
   }

   cs_debug("azbox: scard fd = %d", rfd);
   
   xcas_sc_reset(rfd, 1);

   if ((n = ioctl (rfd, SCARD_IOC_CHECKCARD, &buf)) >= 3) {
     cs_log("azbox: failed to check card, status = %d", n);
     return -1;
   }

   bzero(buf, sizeof(buf));
   xcas_sc_get_atr(rfd, buf);

   cs_debug("azbox: checkcard = %d", n);
   
   return rfd;
}

int xcas_sc_read(int rfd, unsigned char *buf, int l)
{
  return read(rfd, buf, l);
}

int xcas_sc_write(int rfd, unsigned char *buf, int l)
{
  return write(rfd, buf, l);
}

int xcas_sc_get_atr(int rfd, unsigned char *atr)
{
  int n;

  atr[0] = 1;
  if ((n = ioctl (rfd, SCARD_IOC_CHECKCARD, &atr)) < 0) {
    cs_log("azbox: failed to get atr");
    return -1;
  }

  cs_log("azbox: atr len = %d: %s", n, cs_hexdump(0, atr, n));

  return n;
}

int xcas_sc_close(int rfd)
{
  rfd = -1;
  openxcas_release_smartcard_device(0);
}

#else
int xcas_sc_open(void) {return -1;};
int xcas_sc_read(int rfd, unsigned char *buf, int l) {return -1;};
int xcas_sc_write(int rfd, unsigned char *buf, int l) {return -1;};
int xcas_sc_get_atr(int rfd, unsigned char *atr) {return -1;};
int xcas_sc_close(int rfd) {return -1;};
#endif
