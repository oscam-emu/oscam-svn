
#include "azbox.h"

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

  sleep(2);

  // open device
   if ((rfd = openxcas_get_smartcard_device(0)) < 0) {
     cs_log("azbox: failed to open scard dev");
     return -1;
   }

   // check if card is present
   if ((n = ioctl (rfd, SCARD_IOC_CHECKCARD, &buf)) >= 3) {
     cs_log("azbox: failed to check card, status = %d", n);
     return -1;
   }

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
  if (!(n = ioctl (rfd, SCARD_IOC_CHECKCARD, &atr))) {
    cs_log("azbox: failed to get atr");
    return -1;
  }

  cs_log("azbox: atr len = %d: %s", n, cs_hexdump(0, buf, n));

  return n;
}

int xcas_sc_close(int rfd)
{
  rfd = -1;
  openxcas_release_smartcard_device(0);
}
