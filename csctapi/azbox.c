#include "azbox.h"

#ifdef AZBOX
#include <string.h>

int xcas_sc_reset(int rfd)
{
  int n;
  unsigned char buf[128];

  bzero(buf, sizeof(buf));

  if (rfd < 0) return -1;

  buf[0] = 3;
  buf[1] = 1;

  cs_debug("azbox: performing reset...");

  if ((n = ioctl(rfd, SCARD_IOC_WARMRESET, &buf)) < 0) {
    cs_log("azbox: card reset failed (fd = %d, n = %d)", rfd, n);
  }

  cs_dump("azbox: card reset ok (fd = %d, n = %d)", rfd, n);
  return n;
}

int xcas_sc_check(int rfd)
{
  int n;
  unsigned char buf[128];

  bzero(buf, sizeof(buf));

  if (rfd < 0) return -1;

  n = ioctl (rfd, SCARD_IOC_CHECKCARD, &buf);

  cs_dump("azbox: card status = %d", n);

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
  sleep(1);

  // open device
   if ((rfd = openxcas_get_smartcard_device(0)) < 0) {
     cs_log("azbox: failed to open scard dev");
     return -1;
   }

   cs_debug("azbox: scard fd = %d", rfd);
   
   sleep(1);

   xcas_sc_reset(rfd);

   if ((n = xcas_sc_check(rfd)) >= 3) {
     cs_log("azbox: failed to check card, status = %d", n);
    // return -1;
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

  if (!atr) return -1;

  atr[0] = 1;
  if ((n = ioctl (rfd, SCARD_IOC_CHECKCARD, &atr)) <= 0) {
    cs_log("azbox: failed to get atr");
    return -1;
  }

  cs_log("azbox: atr len = %d: %s", n, cs_hexdump(0, atr, n));

  return n;
}

int xcas_sc_set_type(int rfd, xcas_sc_type type)
{
  int n;
  unsigned char buf[128];

  bzero(buf, sizeof(buf));

  if (rfd < 0) return -1;

  buf[2] = type;
  memcpy(buf + 5, "\x01\x01\x01\x01\x01", 5);

  if ((n = ioctl(rfd, SCARD_IOC_CHECKCARD, &buf)) < 0) {
    cs_log("azbox: failed to set type = %d", type);
    return -1;
  }

  cs_log("azbox: type set to %d", type);

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
int xcas_sc_reset(int rfd) {return -1;};
int xcas_sc_set_type(int rfd, xcas_sc_type type) {return -1;};
int xcas_sc_close(int rfd) {return -1;};
#endif
