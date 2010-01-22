/*
    ifd_smartreader.h
    Header file for Argolis smartreader+.
*/
#if defined(HAVE_LIBUSB) && defined(USE_PTHREAD)
#ifndef __SMARTREADER__
#define __SMARTREADER__

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>

#include <pthread.h>
#include <memory.h>

#include "../globals.h"
#include "ftdi.h"
#include "atr.h"


#define LOBYTE(w) ((BYTE)((w) & 0xff))
#define HIBYTE(w) ((BYTE)((w) >> 8))


int SR_Init (struct s_reader *reader,int device_index);
int SR_GetStatus (struct s_reader *reader,int * in);
int SR_Reset (struct s_reader *reader, ATR ** atr);
int SR_Transmit (struct s_reader *reader, BYTE * buffer, unsigned size);
int SR_Receive (struct s_reader *reader, BYTE * buffer, unsigned size);
int SR_SetBaudrate (struct s_reader *reader);

// bool find_smartreader(int index, struct ftdi_context* ftdic,struct usb_device *dev);
static struct usb_device * find_smartreader(int index, struct ftdi_context* ftdic);
static void smart_flush(struct s_reader *reader);
static unsigned int smart_read(struct s_reader *reader, unsigned char* buff, size_t size, int timeout_sec);
static unsigned int smart_write(struct s_reader *reader, unsigned char* buff, size_t size, int udelay);
static void EnableSmartReader(struct s_reader *reader, int clock, unsigned short Fi, unsigned char Di, unsigned char Ni, unsigned char T,unsigned char inv);
static void ResetSmartReader(struct s_reader *reader);
static void* ReaderThread(void *p);
static bool smartreader_check_endpoint(struct usb_device *dev);

#endif // __SMARTREADER__
#endif // HAVE_LIBUSB && USE_PTHREAD
