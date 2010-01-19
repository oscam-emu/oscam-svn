/*
    ifd_smartreader.h
    Header file for Argolis smartreader+.
*/
#ifdef HAVE_LIBUSB
#ifdef USE_PTHREAD

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>

#include <pthread.h>
#include <memory.h>

#include "ftdi.h"
#include "atr.h"


#define LOBYTE(w) ((BYTE)((w) & 0xff))
#define HIBYTE(w) ((BYTE)((w) >> 8))

typedef struct  {
    int F;
    float D;
    int fs;
    int N;
    int T;
    int inv;
} SR_CONFIG;

unsigned char g_read_buffer[4096];
unsigned short g_read_buffer_size = 0;
pthread_mutex_t g_read_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_usb_mutex = PTHREAD_MUTEX_INITIALIZER;
struct ftdi_context ftdic;
pthread_t rt;

SR_CONFIG sr_config;


int SR_Init (int device_index);
int SR_Reset (ATR ** atr);
int SR_Transmit (BYTE * buffer, unsigned size);
int SR_Receive (BYTE * buffer, unsigned size);
int SR_SetBaudrate (int mhz);

bool find_smartreader(int index, struct ftdi_context* ftdic);
void smart_flush(struct ftdi_context* ftdic);
int smart_read(struct ftdi_context* ftdic, unsigned char* buff, size_t size, int timeout_sec);
int smart_write(struct ftdi_context* ftdic, unsigned char* buff, size_t size, int udelay);
void EnableSmartReader(struct ftdi_context* ftdic, int clock, unsigned short Fi, unsigned char Di, unsigned char Ni, unsigned char inv);
void ResetSmartReader(struct ftdi_context* ftdic);
void* ReaderThread(void *p);

#endf // USE_PTHREAD
#endif //HAVE_LIBUSB
