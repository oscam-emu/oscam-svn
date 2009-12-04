   /*
    io_serial.c
    Serial port input/output functions

    This file is part of the Unix driver for Towitoko smartcard readers
    Copyright (C) 2000 2001 Carlos Prados <cprados@yahoo.com>

    This version is modified by doz21 to work in a special manner ;)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "defines.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#ifdef OS_HPUX
#include <sys/modem.h>
#endif
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef HAVE_POLL
#include <sys/poll.h>
#else
#include <sys/signal.h>
#include <sys/types.h>
#endif
#include <math.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <time.h>
#include "io_serial.h"
#include "mc_global.h"

#ifdef OS_LINUX
#include <linux/serial.h>
#endif


/*
 * Internal functions declaration
 */

static int IO_Serial_Bitrate_to_Speed(int bitrate);

static int IO_Serial_Bitrate_from_Speed(int speed);

static bool IO_Serial_WaitToRead (int hnd, unsigned delay_ms, unsigned timeout_ms);

static bool IO_Serial_WaitToWrite (IO_Serial *io, unsigned delay_ms, unsigned timeout_ms);

static void IO_Serial_DeviceName (unsigned com, bool usbserial, char * filename, unsigned length);

static bool IO_Serial_InitPnP (IO_Serial * io);

static void IO_Serial_Clear (IO_Serial * io);

static void IO_SR_Clear (SR_Config * srConfig);

static bool IO_Serial_Set_Smartreader_Config (IO_Serial * io);

static int _in_echo_read = 0;
int io_serial_need_dummy_char = 0;

int fdmc=(-1);

#if defined(TUXBOX) && defined(PPC)
void IO_Serial_Ioctl_Lock(IO_Serial * io, int flag)
{
  extern int *oscam_sem;
  if ((io->com!=RTYP_DB2COM1) && (io->com!=RTYP_DB2COM2)) return;
  if (!flag)
    *oscam_sem=0;
  else while (*oscam_sem!=io->com)
  {
    while (*oscam_sem)
    usleep((io->com)*2000);
    *oscam_sem=io->com;
    usleep(1000);
  }
}

static bool IO_Serial_DTR_RTS_dbox2(int mcport, int dtr, int set)
{
  int rc;
  unsigned short msr;
  unsigned int mbit;
  unsigned short rts_bits[2]={ 0x10, 0x800};
  unsigned short dtr_bits[2]={0x100,     0};

#ifdef DEBUG_IO
printf("IO: multicam.o %s %s\n", dtr ? "dtr" : "rts", set ? "set" : "clear"); fflush(stdout);
#endif
  if ((rc=ioctl(fdmc, GET_PCDAT, &msr))>=0)
  {
    if (dtr)		// DTR
    {
      if (dtr_bits[mcport])
      {
        if (set)
          msr&=(unsigned short)(~dtr_bits[mcport]);
        else
          msr|=dtr_bits[mcport];
        rc=ioctl(fdmc, SET_PCDAT, &msr);
      }
      else
        rc=0;		// Dummy, can't handle using multicam.o
    }
    else		// RTS
    {
      if (set)
        msr&=(unsigned short)(~rts_bits[mcport]);
      else
        msr|=rts_bits[mcport];
      rc=ioctl(fdmc, SET_PCDAT, &msr);
    }
  }
  return((rc<0) ? FALSE : TRUE);
}
#endif

bool IO_Serial_DTR_RTS(IO_Serial * io, int dtr, int set)
{
	unsigned int msr;
	unsigned int mbit;

#if defined(TUXBOX) && defined(PPC)
	if ((io->com==RTYP_DB2COM1) || (io->com==RTYP_DB2COM2))
		return(IO_Serial_DTR_RTS_dbox2(io->com==RTYP_DB2COM2, dtr, set));
#endif

	mbit=(dtr) ? TIOCM_DTR : TIOCM_RTS;
#if defined(TIOCMBIS) && defined(TIOBMBIC)
	if (ioctl (io->fd, set ? TIOCMBIS : TIOCMBIC, &mbit) < 0)
		return FALSE;
#else
	if (ioctl(io->fd, TIOCMGET, &msr) < 0)
		return FALSE;
	if (set)
		msr|=mbit;
	else
		msr&=~mbit;
	return((ioctl(io->fd, TIOCMSET, &msr)<0) ? FALSE : TRUE);
#endif
}

/*
 * Public functions definition
 */

IO_Serial * IO_Serial_New (int reader_type, int mhz, int cardmhz)
{
	IO_Serial *io;
	
	io = (IO_Serial *) malloc (sizeof (IO_Serial));
	
	if (io != NULL)
		IO_Serial_Clear (io);
	
	io->reader_type=reader_type;
	io->mhz=mhz;
	io->cardmhz=cardmhz;

	io->SmartReaderConf = (SR_Config *) malloc(sizeof (SR_Config));
	if (io != NULL)
		IO_SR_Clear (io->SmartReaderConf);
	
	return io;
}

bool IO_Serial_Init (IO_Serial * io, unsigned com, bool usbserial, bool pnp)
{
	char filename[IO_SERIAL_FILENAME_LENGTH];
	
	IO_Serial_DeviceName (com, usbserial, filename, IO_SERIAL_FILENAME_LENGTH);

	strncpy(io->filename,filename,IO_SERIAL_FILENAME_LENGTH);
	
#ifdef DEBUG_IO
	printf ("IO: Opening serial port %s\n", filename);
#endif
	
	if (com < 1)
		return FALSE;
	
	io->com = com;

#ifdef SCI_DEV
	if (com==RTYP_SCI)
#ifdef SH4
		io->fd = open (filename, O_RDWR|O_NONBLOCK|O_NOCTTY);
#else
		io->fd = open (filename, O_RDWR);
#endif
	else
#endif

#ifdef OS_MACOSX
		// on mac os x, make sure you use the /dev/cu.XXXX device, /dev/tty.XXXX will only work with O_NDELAY
		io->fd = open (filename,  O_RDWR | O_NOCTTY | O_NONBLOCK);
#else
		if (com==RTYP_SMART)
			io->fd = open (filename,  O_RDWR | O_NOCTTY | O_NONBLOCK);
		else
			io->fd = open (filename, O_RDWR | O_NOCTTY | O_SYNC);
#endif

	if (io->fd < 0)
		return FALSE;

#if defined(TUXBOX) && defined(PPC)
	if ((com==RTYP_DB2COM1) || (com==RTYP_DB2COM2))
		if ((fdmc = open(DEV_MULTICAM, O_RDWR)) < 0)
		{
			close(io->fd);
			return FALSE;
		}
#endif
	
	if (com!=RTYP_SCI)
		IO_Serial_InitPnP (io);
	
	io->usbserial=usbserial;
	
	if(io->com!=RTYP_SCI)
		IO_Serial_Flush(io);
		
	return TRUE;
}

bool IO_Serial_GetProperties (IO_Serial * io)
{
	struct termios currtio;
	speed_t i_speed, o_speed;
	unsigned int mctl;

#ifdef SCI_DEV
	if(io->com==RTYP_SCI)
		return FALSE;
#endif

	if (io->input_bitrate != 0 && io->output_bitrate != 0) //properties are already filled
	  return TRUE;
	
	if (tcgetattr (io->fd, &currtio) != 0)
		return FALSE;

	o_speed = cfgetospeed (&currtio);
	io->output_bitrate = IO_Serial_Bitrate_from_Speed(o_speed);

	i_speed = cfgetispeed (&currtio);
	io->input_bitrate = IO_Serial_Bitrate_from_Speed(i_speed);
	
	
	switch (currtio.c_cflag & CSIZE)
	{
		case CS5:
			io->bits = 5;
			break;
		case CS6:
			io->bits = 6;
			break;
		case CS7:
			io->bits = 7;
			break;
		case CS8:
			io->bits = 8;
			break;
	}
	
	if (((currtio.c_cflag) & PARENB) == PARENB)
	{
		if (((currtio.c_cflag) & PARODD) == PARODD)
			io->parity = IO_SERIAL_PARITY_ODD;
		else
			io->parity = IO_SERIAL_PARITY_EVEN;
	}
	else
	{
		io->parity = IO_SERIAL_PARITY_NONE;
	}
	
	if (((currtio.c_cflag) & CSTOPB) == CSTOPB)
		io->stopbits = 2;
	else
		io->stopbits = 1;
	
	if (ioctl (io->fd, TIOCMGET, &mctl) < 0)
		return FALSE;
	
	io->dtr = ((mctl & TIOCM_DTR) ? IO_SERIAL_HIGH : IO_SERIAL_LOW);
	io->rts = ((mctl & TIOCM_RTS) ? IO_SERIAL_HIGH : IO_SERIAL_LOW);
	
#ifdef DEBUG_IO
	printf("IO: Getting properties: %ld bps; %d bits/byte; %s parity; %d stopbits; dtr=%d; rts=%d\n", io->input_bitrate, io->bits, io->parity == IO_SERIAL_PARITY_EVEN ? "Even" : io->parity == IO_SERIAL_PARITY_ODD ? "Odd" : "None", io->stopbits, io->dtr, io->rts);
#endif
	
	return TRUE;
}

bool IO_Serial_SetProperties (IO_Serial * io)
{
	struct termios newtio;
	int mhz;
		
#ifdef SCI_DEV
	if(io->com==RTYP_SCI)
		return FALSE;
#endif
   
   //	printf("IO: Setting properties: com%d, %ld bps; %d bits/byte; %s parity; %d stopbits; dtr=%d; rts=%d\n", io->com, io->input_bitrate, io->bits, io->parity == IO_SERIAL_PARITY_EVEN ? "Even" : io->parity == IO_SERIAL_PARITY_ODD ? "Odd" : "None", io->stopbits, io->dtr, io->rts);
   memset (&newtio, 0, sizeof (newtio));
   /* Set the bitrate */
   

   mhz = io->mhz;

   if(io->reader_type==RTYP_SMART)
   {
#ifdef DEBUG_IO
      printf("IO: SMARTREADER .. switching %s to %2.2fMHz\n", io->filename,(float)mhz/100.0);
#endif
      if(!IO_Serial_Set_Smartreader_Config(io))
      {
#ifdef DEBUG_IO
         printf("IO: SMARTREADER .. ERROR switching %s to %2.2fMHz\n", io->filename,(float)mhz/100.0);
#endif
         return FALSE;
      }
	//return TRUE;
	// no need to continue as the smartreader is ready to be used
	// and we don't want any changes in the baudrate or anything else.
   }

	else
	{
#ifdef OS_LINUX
	   if (io->mhz == io->cardmhz)
#endif
		{ //no overclocking
			cfsetospeed(&newtio, IO_Serial_Bitrate_to_Speed(io->output_bitrate));
			cfsetispeed(&newtio, IO_Serial_Bitrate_to_Speed(io->input_bitrate));
		}
#ifdef OS_LINUX
   else 
		{ //over or underclocking
			/* these structures are only available on linux as fas as we know so limit this code to OS_LINUX */
			struct serial_struct nuts;
			ioctl(io->fd, TIOCGSERIAL, &nuts);
			int custom_baud = 9600 * io->mhz / io->cardmhz;
			nuts.custom_divisor = (nuts.baud_base + (custom_baud/2))/ custom_baud;
			cs_debug("custom baudrate: cardmhz=%d mhz=%d custom_baud=%d baud_base=%d divisor=%d -> effective baudrate %d", 
								  io->cardmhz, io->mhz, custom_baud, nuts.baud_base, nuts.custom_divisor, nuts.baud_base/nuts.custom_divisor);
			nuts.flags &= ~ASYNC_SPD_MASK;
			nuts.flags |= ASYNC_SPD_CUST;
			ioctl(io->fd, TIOCSSERIAL, &nuts);
			cfsetospeed(&newtio, IO_Serial_Bitrate_to_Speed(38400));
			cfsetispeed(&newtio, IO_Serial_Bitrate_to_Speed(38400));
	   }
#endif
    }    
   /* Set the character size */
   switch (io->bits)
   {
		case 5:
			newtio.c_cflag |= CS5;
			break;
		
		case 6:
			newtio.c_cflag |= CS6;
			break;
		
		case 7:
			newtio.c_cflag |= CS7;
			break;
		
		case 8:
			newtio.c_cflag |= CS8;
			break;
	}
	
	/* Set the parity */
	switch (io->parity)
	{
		case IO_SERIAL_PARITY_ODD:
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD;
			break;
		
		case IO_SERIAL_PARITY_EVEN:	
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD;
			break;
		
		case IO_SERIAL_PARITY_NONE:
			newtio.c_cflag &= ~PARENB;
			break;
	}
	
	/* Set the number of stop bits */
	switch (io->stopbits)
	{
		case 1:
			newtio.c_cflag &= (~CSTOPB);
			break;
		case 2:
			newtio.c_cflag |= CSTOPB;
			break;
	}
	
	/* Selects raw (non-canonical) input and output */
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	newtio.c_oflag &= ~OPOST;
#if 1
	newtio.c_iflag |= IGNPAR;
	/* Ignore parity errors!!! Windows driver does so why shouldn't I? */
#endif
	/* Enable receiber, hang on close, ignore control line */
	newtio.c_cflag |= CREAD | HUPCL | CLOCAL;
	
	/* Read 1 byte minimun, no timeout specified */
	newtio.c_cc[VMIN] = 1;
	newtio.c_cc[VTIME] = 0;

//	tcdrain(io->fd);
	if (tcsetattr (io->fd, TCSANOW, &newtio) < 0)
		return FALSE;
//	tcflush(io->fd, TCIOFLUSH);
//	if (tcsetattr (io->fd, TCSAFLUSH, &newtio) < 0)
//		return FALSE;

	IO_Serial_Ioctl_Lock(io, 1);
	IO_Serial_DTR_RTS(io, 0, io->rts == IO_SERIAL_HIGH);
	IO_Serial_DTR_RTS(io, 1, io->dtr == IO_SERIAL_HIGH);
	IO_Serial_Ioctl_Lock(io, 0);
	
#ifdef DEBUG_IO
	printf("IO: Setting properties: com%d, %ld bps; %d bits/byte; %s parity; %d stopbits; dtr=%d; rts=%d\n", io->com, io->input_bitrate, io->bits, io->parity == IO_SERIAL_PARITY_EVEN ? "Even" : io->parity == IO_SERIAL_PARITY_ODD ? "Odd" : "None", io->stopbits, io->dtr, io->rts);
#endif
	return TRUE;
}

void IO_Serial_Flush (IO_Serial * io)
{
	BYTE b;
	while(IO_Serial_Read(io, 1000, 1, &b));
}


void IO_Serial_GetPnPId (IO_Serial * io, BYTE * pnp_id, unsigned *length)
{
	(*length) = io->PnP_id_size;
	memcpy (pnp_id, io->PnP_id, io->PnP_id_size);
}

unsigned IO_Serial_GetCom (IO_Serial * io)
{
	return io->com;
}


bool IO_Serial_Read (IO_Serial * io, unsigned timeout, unsigned size, BYTE * data)
{
	BYTE c;
	int count = 0;
#ifdef SH4
	bool readed;
	struct timeval tv, tv_spent;
#endif
	
	if((io->com!=RTYP_SCI) && (io->wr>0))
	{
		BYTE buf[256];
		int n = io->wr;
		io->wr = 0;
	
		if(!IO_Serial_Read (io, timeout, n, buf))
		{
			return FALSE;
		}
	}
	
#ifdef DEBUG_IO
	printf ("IO: Receiving: ");
	fflush (stdout);
#endif
	for (count = 0; count < size * (_in_echo_read ? (1+io_serial_need_dummy_char) : 1); count++)
	{
#ifdef SH4
		gettimeofday(&tv,0);
		memcpy(&tv_spent,&tv,sizeof(struct timeval));
		readed=FALSE;
		while( (((tv_spent.tv_sec-tv.tv_sec)*1000) + ((tv_spent.tv_usec-tv.tv_usec)/1000L))<timeout )
 		{
 			if (read (io->fd, &c, 1) == 1)
 			{
 				readed=TRUE;
				break;
 			}
 			gettimeofday(&tv_spent,0);
		}
		if(!readed) return FALSE;
		
		data[_in_echo_read ? count/(1+io_serial_need_dummy_char) : count] = c;
#ifdef DEBUG_IO
		printf ("%X ", c);
		fflush (stdout);
#endif
#else
		if (IO_Serial_WaitToRead (io->fd, 0, timeout))
		{
			if (read (io->fd, &c, 1) != 1)
			{
#ifdef DEBUG_IO
				printf ("ERROR\n");
				fflush (stdout);
#endif
				return FALSE;
			}
			data[_in_echo_read ? count/(1+io_serial_need_dummy_char) : count] = c;
			
#ifdef DEBUG_IO
			printf ("%X ", c);
			fflush (stdout);
#endif
		}
		else
		{
#ifdef DEBUG_IO
			printf ("TIMEOUT\n");
			fflush (stdout);
#endif
			tcflush (io->fd, TCIFLUSH);
			return FALSE;
		}
#endif
	}
	
    _in_echo_read = 0;

#ifdef DEBUG_IO
	printf ("\n");
	fflush (stdout);
#endif
	
	return TRUE;
}




bool IO_Serial_Write (IO_Serial * io, unsigned delay, unsigned size, BYTE * data)
{
	unsigned count, to_send;
    BYTE data_w[512];
    int i_w;
#ifdef DEBUG_IO
	unsigned i;
	
	printf ("IO: Sending: ");
	fflush (stdout);
#endif
	/* Discard input data from previous commands */
//	tcflush (io->fd, TCIFLUSH);
	
	for (count = 0; count < size; count += to_send)
	{
//		if(io->com==RTYP_SCI)
//			to_send = 1;
//		else
			to_send = (delay? 1: size);
		
		if (IO_Serial_WaitToWrite (io, delay, 1000))
		{
            for (i_w=0; i_w < to_send; i_w++) {
            data_w [(1+io_serial_need_dummy_char)*i_w] = data [count + i_w];
            if (io_serial_need_dummy_char) {
              data_w [2*i_w+1] = 0x00;
              }
            }
            unsigned int u = write (io->fd, data_w, (1+io_serial_need_dummy_char)*to_send);
			// once the smargo is in native smartreader mode we don't get the echo of the sent data.
		   if(io->reader_type==RTYP_SMART)
	            _in_echo_read = 0;
		   else
	            _in_echo_read = 1;
            if (u != (1+io_serial_need_dummy_char)*to_send)
			{
#ifdef DEBUG_IO
				printf ("ERROR\n");
				fflush (stdout);
#endif
				if(io->com!=RTYP_SCI)
					io->wr += u;
				return FALSE;
			}
			
			if(io->com!=RTYP_SCI)
				io->wr += to_send;
			
#ifdef DEBUG_IO
			for (i=0; i<(1+io_serial_need_dummy_char)*to_send; i++)
				printf ("%X ", data_w[count + i]);
			fflush (stdout);
#endif
		}
		else
		{
#ifdef DEBUG_IO
			printf ("TIMEOUT\n");
			fflush (stdout);
#endif
//			tcflush (io->fd, TCIFLUSH);
			return FALSE;
		}
	}
	
#ifdef DEBUG_IO
	printf ("\n");
	fflush (stdout);
#endif
	
	return TRUE;
}

bool IO_Serial_Close (IO_Serial * io)
{
	char filename[IO_SERIAL_FILENAME_LENGTH];
	
	IO_Serial_DeviceName (io->com, io->usbserial, filename, IO_SERIAL_FILENAME_LENGTH);
	
#ifdef DEBUG_IO
	printf ("IO: Clossing serial port %s\n", filename);
#endif
	
#if defined(TUXBOX) && defined(PPC)
	close(fdmc);
#endif
	if (close (io->fd) != 0)
		return FALSE;
	
	IO_Serial_Clear (io);
	
	return TRUE;
}

/*
 * Internal functions definition
 */

static int IO_Serial_Bitrate_to_Speed(int bitrate)
{
	static const struct BaudRates { int real; speed_t apival; } BaudRateTab[] = {
		{   200, B200   }, {  300, B300  }, {  600, B600  },
		{   1200, B1200   }, {  2400, B2400  }, {  4800, B4800  },
		{   9600, B9600   }, {  19200, B19200  }, {  38400, B38400  },
		{  57600, B57600  }, { 115200, B115200 }, { 230400, B230400 }
		};

	int i;
	
	for(i=0; i<(int)(sizeof(BaudRateTab)/sizeof(struct BaudRates)); i++)
	{
		int b=BaudRateTab[i].real;
		int d=((b-bitrate)*10000)/b;
		if(abs(d)<=300)
		{
			return BaudRateTab[i].apival;
		}
	}
	return B0;
}

static int IO_Serial_Bitrate_from_Speed(int speed)
{

	switch (speed)
	{
#ifdef B0
		case B0:
			return 0;
#endif
#ifdef B50
		case B50:
			return 50;
#endif
#ifdef B75
		case B75:
			return 75;
#endif
#ifdef B110
		case B110:
			return 110;
#endif
#ifdef B134
		case B134:
			return 134;
#endif
#ifdef B150
		case B150:
			return 150;
#endif
#ifdef B200
		case B200:
			return 200;
#endif
#ifdef B300
		case B300:
			return 300;
#endif
#ifdef B600
		case B600:
			return 600;
#endif
#ifdef B1200
		case B1200:
			return 1200;
#endif
#ifdef B1800
		case B1800:
			return 1800;
#endif
#ifdef B2400
		case B2400:
			return 2400;
#endif
#ifdef B4800
		case B4800:
			return 4800;
#endif
#ifdef B9600
		case B9600:
			return 9600;
#endif
#ifdef B19200
		case B19200:
			return 19200;
#endif
#ifdef B38400
		case B38400:
			return 38400;
#endif
#ifdef B57600
		case B57600:
			return 57600;
#endif
#ifdef B115200
		case B115200:
			return 115200;
#endif
#ifdef B230400
		case B230400:
			return 230400;
#endif
		default:
			return 1200;
	}

	return 0;	/* Should never get here */
}



static bool IO_Serial_WaitToRead (int hnd, unsigned delay_ms, unsigned timeout_ms)
{
   fd_set rfds;
   fd_set erfds;
   struct timeval tv;
   int select_ret;
   int in_fd;
   
   if (delay_ms > 0)
   {
#ifdef HAVE_NANOSLEEP
      struct timespec req_ts;
      
      req_ts.tv_sec = delay_ms / 1000;
      req_ts.tv_nsec = (delay_ms % 1000) * 1000000L;
      nanosleep (&req_ts, NULL);
#else
      usleep (delay_ms * 1000L);
#endif
   }
   
   in_fd=hnd;
   
   FD_ZERO(&rfds);
   FD_SET(in_fd, &rfds);
   
   FD_ZERO(&erfds);
   FD_SET(in_fd, &erfds);
   
   tv.tv_sec = timeout_ms/1000;
   tv.tv_usec = (timeout_ms % 1000) * 1000L;
   select_ret = select(in_fd+1, &rfds, NULL,  &erfds, &tv);
   if(select_ret==-1)
   {
      printf("select_ret=%i\n" , select_ret);
      printf("errno =%d\n", errno);
      fflush(stdout);
      return (FALSE);
   }

   if (FD_ISSET(in_fd, &erfds))
   {
      printf("fd is in error fds\n");
      printf("errno =%d\n", errno);
      fflush(stdout);
      return (FALSE);
   }

   return(FD_ISSET(in_fd,&rfds));
}

static bool IO_Serial_WaitToWrite (IO_Serial *io, unsigned delay_ms, unsigned timeout_ms)
{
   fd_set wfds;
   fd_set ewfds;
   struct timeval tv;
   int select_ret;
   int out_fd;
   
#ifdef SCI_DEV
   if(io->com==RTYP_SCI)
      return TRUE;
#endif
		
   if (delay_ms > 0)
	{
#ifdef HAVE_NANOSLEEP
      struct timespec req_ts;
      
      req_ts.tv_sec = delay_ms / 1000;
      req_ts.tv_nsec = (delay_ms % 1000) * 1000000L;
      nanosleep (&req_ts, NULL);
#else
      usleep (delay_ms * 1000L);
#endif
   }

   out_fd=io->fd;
    
   FD_ZERO(&wfds);
   FD_SET(out_fd, &wfds);
   
   FD_ZERO(&ewfds);
   FD_SET(out_fd, &ewfds);
   
   tv.tv_sec = timeout_ms/1000L;
   tv.tv_usec = (timeout_ms % 1000) * 1000L;

   select_ret = select(out_fd+1, NULL, &wfds, &ewfds, &tv);

   if(select_ret==-1)
   {
      printf("select_ret=%d\n" , select_ret);
      printf("errno =%d\n", errno);
      fflush(stdout);
      return (FALSE);
   }

   if (FD_ISSET(out_fd, &ewfds))
   {
      printf("fd is in ewfds\n");
      printf("errno =%d\n", errno);
      fflush(stdout);
      return (FALSE);
   }

   return(FD_ISSET(out_fd,&wfds));
    
}

static void IO_Serial_Clear (IO_Serial * io)
{
	memset(io->filename,0,IO_SERIAL_FILENAME_LENGTH);
	io->fd = -1;
	io->com = 0;
	memset (io->PnP_id, 0, IO_SERIAL_PNPID_SIZE);
	io->PnP_id_size = 0;
	io->usbserial = FALSE;
	io->wr = 0;
	//modifyable properties:
	io->input_bitrate = 0;
	io->output_bitrate = 0;
	io->bits = 0;
	io->stopbits = 0;
	io->parity = 0;
	io->dtr = 0;
	io->rts = 0;
}

static void IO_SR_Clear (SR_Config * srConfig)
{
	// set smartreader+ default values
	srConfig->F=372;
	srConfig->D=1.0;
	srConfig->fs=3571200;
	srConfig->N=0;
	srConfig->T=0;
	srConfig->inv=0;
}

static void IO_Serial_DeviceName (unsigned com, bool usbserial, char * filename, unsigned length)
{
	extern char oscam_device[];
   snprintf (filename, length, "%s", oscam_device);
//	if(com==1)
//		snprintf (filename, length, "/dev/tts/%d", com - 1);
//	else
//		snprintf (filename, length, "/dev/sci%d", com - 2);
}

static bool IO_Serial_InitPnP (IO_Serial * io)
{
	int i = 0;
	io->input_bitrate = 1200;
	io->output_bitrate = 1200;
	io->parity = IO_SERIAL_PARITY_NONE;
	io->bits = 7;
	io->stopbits = 1;
	io->dtr = IO_SERIAL_HIGH;
//	io->rts = IO_SERIAL_HIGH;
	io->rts = IO_SERIAL_LOW;

	// set smartreader+ default values
	// use the frequency to get F for the smartreader
	// to make sure the ATR is sent at 9600.
	if(io->SmartReaderConf)
	{
		io->SmartReaderConf->F=(int)ceil((float)(io->mhz)*10000.0/9600);
		io->SmartReaderConf->D=1.0;
		io->SmartReaderConf->fs=(io->mhz)*10000; // freq in Hz
		io->SmartReaderConf->N=0;
		io->SmartReaderConf->T=0;
		io->SmartReaderConf->inv=0;
	}
	
	if (!IO_Serial_SetProperties (io))
		return FALSE;

	while ((i < IO_SERIAL_PNPID_SIZE) && IO_Serial_Read (io, 200, 1, &(io->PnP_id[i])))
      i++;

	io->PnP_id_size = i;
		return TRUE;
}


static bool IO_Serial_Set_Smartreader_Config(IO_Serial * io)
{
	SR_Config *sr_config;
	struct termios term;
	BYTE cmd[16];
	int fs;
	
	sr_config=io->SmartReaderConf;
	if(sr_config==NULL)
		return FALSE;

	fs/=1000; // convert to kHz.

#  ifdef DEBUG_IO
	printf("IO: Smartreader+ on %s: F=%d D=%f fs=%dKHz N=%d T=%d inv=%d\n",
			io->filename,
			sr_config->F,
			sr_config->D,
			fs,
			sr_config->N,
			sr_config->T,
			sr_config->inv);
#  endif

	// Set SmartReader+ in command mode.
	if(tcgetattr(io->fd,&term)==-1)
		{
#  ifdef DEBUG_IO
		printf("%s: tcgetattr failed: %s",io->filename,strerror(errno));
#endif
		return FALSE;
		}

	term.c_cflag&=~CSIZE;
	term.c_cflag|=CS5;
	if(tcsetattr(io->fd,TCSADRAIN,&term)==-1)
		{
#  ifdef DEBUG_IO
		printf("%s: tcsetattr failed: %s",io->filename,strerror(errno));
#endif
		return FALSE;
		}
	// Write SmartReader+ configuration commands.

	// how is (BYTE)D supposed to work for fractional values e.g. 0.125 ??
	cmd[0]=1;
	cmd[1]=(BYTE)(((sr_config->F)>>8) & 0xFF);
	cmd[2]=(BYTE)(sr_config->F & 0xFF);
	cmd[3]=(BYTE)((int)(sr_config->D)  & 0xFF);
	if(!IO_Serial_Write(io, 0, 4, cmd))
		return FALSE;

	cmd[0]=2;
	cmd[1]=(BYTE)((fs>>8) & 0xFF);
	cmd[2]=(BYTE)(fs & 0xFF);
	if(!IO_Serial_Write(io, 0, 3, cmd))
		return FALSE;

	cmd[0]=3;
	cmd[1]=(BYTE)(sr_config->N & 0xFF);
	if(!IO_Serial_Write(io, 0, 2, cmd))
		return FALSE;

	cmd[0]=4;
	cmd[1]=(BYTE)(sr_config->T & 0xFF);
	if(!IO_Serial_Write(io, 0, 2, cmd))
		return FALSE;

	cmd[0]=5;
	cmd[1]=(BYTE)(sr_config->inv & 0xFF);
	if(!IO_Serial_Write(io, 0, 2, cmd))
		return FALSE;

	// Send zero bits for 0.25 - 0.5 seconds.
	if(tcsendbreak(io->fd,0)==-1)
		{
#  ifdef DEBUG_IO
		printf("%s: tcsendbreak failed: %s\n",io->filename,strerror(errno));
#endif
		return FALSE;
		}
	
	// We're entering SmartReader+ mode; speed up serial communication.
	cfsetispeed(&term,B115200);
	cfsetospeed(&term,B115200);
	io->input_bitrate=115200;
	io->output_bitrate=115200;

	// Set SmartReader+ in DATA mode.
	term.c_cflag&=~CSIZE;
	term.c_cflag|=CS8;
	if(tcsetattr(io->fd,TCSADRAIN,&term)==-1)
		{
#  ifdef DEBUG_IO
		printf("%s: tcsetattr failed: %s\n",io->filename,strerror(errno));
#endif
		return FALSE;
		}
#ifdef DEBUG_IO
		printf("IO: Setting SmartReader+ config done\n");
#endif
	IO_Serial_Flush(io);
	return TRUE;
}

