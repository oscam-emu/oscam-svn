#if defined(HAVE_LIBUSB) && defined(USE_PTHREAD)
/*
		ifd_smartreader.c
		This module provides IFD handling functions for for Argolis smartreader+.
*/

#include <stdio.h>
#include <time.h>
#include <string.h>
#include"ifd_smartreader.h"

#define OK 		1 
#define ERROR 0

int SR_Init (int device_index)
{
    
    int ret;
    smartreader_usb_dev=NULL;
    if(!(smartreader_usb_dev=find_smartreader(device_index, &ftdic)))
        return ERROR;
    //The smartreader has different endpoint addresses
    //compared to a real FT232 device, so change them here,
    //also a good way to compare a real FT232 with a smartreader
    //if you enumarate usb devices
    ftdic.in_ep = 0x1;
    ftdic.out_ep = 0x82;

    
    //open the smartreader device if found by find_smartreader
    if ((ret = ftdi_usb_open_dev(&ftdic,smartreader_usb_dev)) < 0) {
        cs_log("unable to open ftdi device: %d (%s)", ret, ftdi_get_error_string(&ftdic));
        return ERROR;
    }

#ifdef DEBUG_IO
    cs_log("IO:SR: Setting smartreader latency timer to 1ms");
#endif
    //Set the FTDI latency timer to 1ms
    ret = ftdi_set_latency_timer(&ftdic, 1);

    //Set databits to 8o2
    ret = ftdi_set_line_property(&ftdic, BITS_8, STOP_BIT_2, ODD);

    //Set the DTR HIGH and RTS LOW
    ftdi_setdtr_rts(&ftdic, 1, 1);

    //Disable flow control
    ftdi_setflowctrl(&ftdic, 0);

    // star the reading thread
    g_read_buffer_size = 0;
    modem_status = 0 ;
    pthread_mutex_init(&g_read_mutex,NULL);
    pthread_mutex_init(&g_usb_mutex,NULL);
    ret = pthread_create(&rt, NULL, ReaderThread, (void *)&ftdic);
    if (ret) {
        cs_log("ERROR; return code from pthread_create() is %d", ret);
        return ERROR;
    }

	return OK;
}


int SR_GetStatus (int * in)
{
	int state;

    pthread_mutex_lock(&g_read_mutex);
    state =(modem_status & 0x80) == 0x80 ? 0 : 2;
    pthread_mutex_unlock(&g_read_mutex);

    
	//state = 0 no card, 1 = not ready, 2 = ready
	if (state)
		*in = 1; //CARD, even if not ready report card is in, or it will never get activated
	else
		*in = 0; //NOCARD
	return OK;
}

int SR_Reset (ATR ** atr)
{
    unsigned char data[ATR_MAX_SIZE] = {0};
    int ret;
    
    //Reset smartreader to defaults
    ResetSmartReader(&ftdic);

    //Reset smartcard
    pthread_mutex_lock(&g_usb_mutex);
    ftdi_setdtr_rts(&ftdic, 1, 1);
    usleep(200000);
    ftdi_setdtr_rts(&ftdic, 1, 0);
    pthread_mutex_unlock(&g_usb_mutex);

    //Read the ATR
    ret = smart_read(&ftdic, data, 32,1);
#ifdef DEBUG_IO
    cs_log("IO:SR: get ATR ret = %d" , ret);
#endif
    // did we get any data for the ATR ?
    if(!ret)
        return ERROR;

    if(data[0]==0x03) {
        sr_config.inv=1;
        EnableSmartReader(&ftdic, 3571200, 372, 1, 0, 0, sr_config.inv);
    }
    // parse atr
	(*atr) = ATR_New ();
	if(ATR_InitFromArray ((*atr), data, ret) == ATR_OK)
	{
		struct timespec req_ts;
		req_ts.tv_sec = 0;
		req_ts.tv_nsec = 50000000;
		nanosleep (&req_ts, NULL);
		return OK;
	}
	else
	{
		ATR_Delete (*atr);
		(*atr) = NULL;
		return ERROR;
	}
    
}

int SR_Transmit (BYTE * sent, unsigned size)
{ 
    int ret;
    ret = smart_write(&ftdic, sent, size, 0);
    if (ret!=size)
        return ERROR;
        
	return OK;
}

int SR_Receive (BYTE * data, unsigned size)
{ 
    int ret;
    ret = smart_read(&ftdic, data, size,1);
    if (ret!=size)
        return ERROR;

	return OK;
}	

int SR_SetBaudrate (int mhz)
{

    sr_config.fs=mhz*1000; //freq in KHz
    EnableSmartReader(&ftdic, sr_config.fs, sr_config.F, (BYTE)sr_config.D, sr_config.N, sr_config.T, sr_config.inv);
    //baud rate not really used in native mode since
    //it's handled by the card, so just set to maximum 3Mb/s
    pthread_mutex_lock(&g_usb_mutex);
    ftdi_set_baudrate(&ftdic, 3000000);
    pthread_mutex_unlock(&g_usb_mutex);

	return OK;
}


struct usb_device * find_smartreader(int index, struct ftdi_context* ftdic)
{
    int ret, i;
    bool dev_found;
    struct usb_bus *bus;
    struct usb_device *dev;

    if (ftdi_init(ftdic) < 0) {
        cs_log("ftdi_init failed");
        return NULL;
    }
    usb_init();
    if (usb_find_busses() < 0) {
        cs_log("usb_find_busses() failed");
        return NULL;
    }
    if (usb_find_devices() < 0) {
        cs_log("usb_find_devices() failed");
        return NULL;
    }


    i = -1;
    dev_found=FALSE;
    for (bus = usb_get_busses(); bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
            if ( (dev->descriptor.idVendor != 0x0403) || (dev->descriptor.idProduct != 0x6001))
                    continue;
#ifdef DEBUG_IO
            cs_log("IO:SR: Checking FTDI device: %d", i+1);
#endif
            if(smartreader_check_endpoint(dev)) {
                i++;
                if (i==index) {
                    dev_found=TRUE;
                    break;
                }
            }
        }
    if(dev_found)
        break;    
    }
#ifdef DEBUG_IO
    cs_log("IO:SR: Found smartreader device %d",index);
#endif

    if(!dev_found) {
        cs_log("IO:SR: Smartreader device number %d not found",index);
        ftdi_deinit(ftdic);
        return NULL;
        }

    return dev;
}

void smart_flush(struct ftdi_context* ftdic)
{

    pthread_mutex_lock(&g_usb_mutex);
    ftdi_usb_purge_buffers(ftdic);
    pthread_mutex_unlock(&g_usb_mutex);

    pthread_mutex_lock(&g_read_mutex);
    g_read_buffer_size = 0;
    pthread_mutex_unlock(&g_read_mutex);
}

int smart_read(struct ftdi_context* ftdic, unsigned char* buff, size_t size, int timeout_sec)
{

    int ret = 0;
    int total_read = 0;
    struct timeval start, now, dif = {0};
    gettimeofday(&start,NULL);


    while(total_read < size && dif.tv_sec < timeout_sec) {

        pthread_mutex_lock(&g_read_mutex);
        if(g_read_buffer_size > 0) {
        
            ret = g_read_buffer_size>size-total_read ? size-total_read : g_read_buffer_size;
            memcpy(buff+total_read,g_read_buffer,ret);
            g_read_buffer_size -= ret;
            total_read+=ret;
        }
        pthread_mutex_unlock(&g_read_mutex);
       
        gettimeofday(&now,NULL);
        timersub(&now, &start, &dif);
        usleep(500);

    }

    
    return total_read;
}

int smart_write(struct ftdi_context* ftdic, unsigned char* buff, size_t size, int udelay)
{

    int ret = 0;
    int idx;

    if (udelay == 0) {
        pthread_mutex_lock(&g_usb_mutex);
        ret = ftdi_write_data(ftdic, buff, size);
        pthread_mutex_unlock(&g_usb_mutex);
    }
    else {
        for (idx = 0; idx < size; idx++) {
            pthread_mutex_lock(&g_usb_mutex);
            if ((ret = ftdi_write_data(ftdic, &buff[idx], 1)) < 0){
                pthread_mutex_unlock(&g_usb_mutex);
                break;
            }
            pthread_mutex_unlock(&g_usb_mutex);
            usleep(udelay);
        }
    }

    return ret;
}

void EnableSmartReader(struct ftdi_context* ftdic, int clock, unsigned short Fi, unsigned char Di, unsigned char Ni, unsigned char T, unsigned char inv) {

    int ret = 0;
    unsigned char buff[4];
    int delay=50000;

    
    pthread_mutex_lock(&g_usb_mutex);
    ret = ftdi_set_baudrate(ftdic, 9600);
    ret = ftdi_set_line_property(ftdic, (enum ftdi_bits_type) 5, STOP_BIT_2, NONE);
    pthread_mutex_unlock(&g_usb_mutex);
#ifdef DEBUG_IO
    cs_log("IO:SR: sending F=%04X to smartreader",Fi);
    cs_log("IO:SR: sending D=%02X to smartreader",Di);
#endif

    unsigned char FiDi[] = {0x01, HIBYTE(Fi), LOBYTE(Fi), Di};
    ret = smart_write(ftdic, FiDi, sizeof (FiDi),0);
    usleep(delay);

    unsigned short freqk = (unsigned short) (clock / 1000);
#ifdef DEBUG_IO
    cs_log("IO:SR: sending Freq=%d to smartreader",freqk);
#endif
    unsigned char Freq[] = {0x02, HIBYTE(freqk), LOBYTE(freqk)};
    ret = smart_write(ftdic, Freq, sizeof (Freq),0);
    usleep(delay);

#ifdef DEBUG_IO
    cs_log("IO:SR: sending N=%02X to smartreader",Ni);
#endif
    unsigned char N[] = {0x03, Ni};
    ret = smart_write(ftdic, N, sizeof (N),0);
    usleep(delay);

#ifdef DEBUG_IO
    cs_log("IO:SR: sending T=%02X to smartreader",T);
#endif
    unsigned char Prot[] = {0x04, T};
    ret = smart_write(ftdic, Prot, sizeof (Prot),0);
    usleep(delay);

#ifdef DEBUG_IO
    cs_log("IO:SR: sending inv=%02X to smartreader",inv);
#endif
    unsigned char Invert[] = {0x05, inv};
    ret = smart_write(ftdic, Invert, sizeof (Invert),0);
    usleep(delay);

    pthread_mutex_lock(&g_usb_mutex);
    ret = ftdi_set_line_property2(ftdic, BITS_8, STOP_BIT_2, ODD, BREAK_ON);
    pthread_mutex_unlock(&g_usb_mutex);
   
    usleep(50000);

    pthread_mutex_lock(&g_usb_mutex);
    ret = ftdi_set_line_property2(ftdic, BITS_8, STOP_BIT_2, ODD, BREAK_OFF);
    pthread_mutex_unlock(&g_usb_mutex);

    smart_flush(ftdic);
}

void ResetSmartReader(struct ftdi_context* ftdic) 
{

    smart_flush(ftdic);
    // set smartreader+ default values 
    sr_config.F=372; 
    sr_config.D=1.0; 
    sr_config.fs=3571200; 
    sr_config.N=0; 
    sr_config.T=0; 
    sr_config.inv=0; 

    EnableSmartReader(ftdic, sr_config.fs, sr_config.F, (BYTE)sr_config.D, sr_config.N, sr_config.T, sr_config.inv);

}

void* ReaderThread(void *p)
{

    struct ftdi_context* ftdic = (struct ftdi_context*)p;
    bool running = TRUE;
    int ret;
    int copy_size;
    unsigned char local_buffer[64];  //64 is max transfer size of FTDI bulk pipe

    while(running){

        if(g_read_buffer_size == sizeof(g_read_buffer)){
            //if out read buffer is full then delay
            //slightly and go around again
            struct timespec req = {0,200000000}; //20ms
            nanosleep(&req,NULL);
            continue;
        }

        pthread_mutex_lock(&g_usb_mutex);
        ret = usb_bulk_read(ftdic->usb_dev,ftdic->out_ep,(char*)local_buffer,64,1000);
        pthread_mutex_unlock(&g_usb_mutex);
        sched_yield();


        if(ret>2) {  //FTDI always sends modem status bytes as first 2 chars with the 232BM
            pthread_mutex_lock(&g_read_mutex);
            modem_status=local_buffer[0];

            copy_size = sizeof(g_read_buffer) - g_read_buffer_size > ret-2 ?ret-2: sizeof(g_read_buffer) - g_read_buffer_size;
            memcpy(g_read_buffer+g_read_buffer_size,local_buffer+2,copy_size);
            g_read_buffer_size += copy_size;            
            pthread_mutex_unlock(&g_read_mutex);
        } 
        else {
            if(ret==2) {
                pthread_mutex_lock(&g_read_mutex);
                modem_status=local_buffer[0];
                pthread_mutex_unlock(&g_read_mutex);
            }
            //sleep for 50ms since there was nothing to read last time
            usleep(50000);
        }
    }

    pthread_exit(NULL);
}


bool smartreader_check_endpoint(struct usb_device *dev)
{
    int nb_interfaces;
    int i,j,k,l;
    u_int8_t tmpEndpointAddress;
    int nb_endpoint_ok;

    if (!dev->config) {
#ifdef DEBUG_IO
        cs_log("IO:SR:  Couldn't retrieve descriptors");
#endif
        return FALSE;
    }
        
    nb_interfaces=dev->config->bNumInterfaces;
    // smartreader only has 1 interface
    if(nb_interfaces!=1) {
#ifdef DEBUG_IO
        cs_log("IO:SR:  Couldn't retrieve interfaces");
#endif
        return FALSE;
    }

    nb_endpoint_ok=0;
    for (i = 0; i < dev->descriptor.bNumConfigurations; i++)
        for (j = 0; j < dev->config[i].bNumInterfaces; j++)
            for (k = 0; k < dev->config[i].interface[j].num_altsetting; k++)
                for (l = 0; l < dev->config[i].interface[j].altsetting[k].bNumEndpoints; l++) {
                    tmpEndpointAddress=dev->config[i].interface[j].altsetting[k].endpoint[l].bEndpointAddress;
#ifdef DEBUG_IO
                    cs_log("IO:SR:  checking endpoint address %02X",tmpEndpointAddress);
#endif
                    if((tmpEndpointAddress== 0x1) || (tmpEndpointAddress== 0x82))
                        nb_endpoint_ok++;
                }

    if(nb_endpoint_ok!=2)
        return FALSE;
    return TRUE;
}

#endif // HAVE_LIBUSB && USE_PTHREAD
