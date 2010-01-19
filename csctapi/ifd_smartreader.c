#ifdef HAVE_LIBUSB
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
    
    if(!find_smartreader(device_index, &ftdic))
        return ERROR;
    
    //The smartreader has different endpoint addresses
    //compared to a real FT232 device, so change them here,
    //also a good way to compare a real FT232 with a smartreader
    //if you enumarate usb devices
    ftdic.in_ep = 0x1;
    ftdic.out_ep = 0x82;

    //open the first smartreader if found by find_smartreader
    if ((ret = ftdi_usb_open(&ftdic, 0x0403, 0x6001)) < 0) {
        cs_log("unable to open ftdi device: %d (%s)\n", ret, ftdi_get_error_string(&ftdic));
        return ERROR;
    }

    //Set the FTDI latency timer to 1ms
    ret = ftdi_set_latency_timer(&ftdic, 1);

    //Set databits to 8o2
    ret = ftdi_set_line_property(&ftdic, BITS_8, STOP_BIT_2, ODD);

    //Set the DTR HIGH and RTS LOW
    ftdi_setdtr_rts(&ftdic, 1, 1);

    //Disable flow control
    ftdi_setflowctrl(&ftdic, 0);

    ret = pthread_create(&rt, NULL, ReaderThread, (void *)&ftdic);
    if (ret) {
        cs_log("ERROR; return code from pthread_create() is %d\n", ret);
        return ERROR;
    }


	return OK;
}


int SR_GetStatus (int * in)
{
	int state;

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
    if(data[0]==0x03) {
        sr_config.inv=1;
        EnableSmartReader(&ftdic, 3571200, 372, 1, 0, sr_config.inv);
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
    EnableSmartReader(&ftdic, sr_config.fs, sr_config.F, sr_config.D, sr_config.N, sr_config.inv);

	return OK;
}


bool find_smartreader(int index, struct ftdi_context* ftdic)
{
    int ret, i;
    bool dev_found;
    struct ftdi_device_list *devlist, *curdev;
    char manufacturer[128], description[128];
    
    if (ftdi_init(ftdic) < 0) {
        fprintf(stderr, "ftdi_init failed\n");
        return ERROR;
    }

    if ((ret = ftdi_usb_find_all(ftdic, &devlist, 0x0403, 0x6001)) < 0)
    {
        fprintf(stderr, "ftdi_usb_find_all failed: %d (%s)\n", ret, ftdi_get_error_string(ftdic));
        return ERROR;
    }

    printf("Number of FTDI devices found: %d\n", ret);

    i = 0;
    dev_found=FALSE;
    for (curdev = devlist; curdev != NULL; i++)
    {
        printf("Checking device: %d\n", i);
        if ((ret = ftdi_usb_get_strings(ftdic, curdev->dev, manufacturer, 128, description, 128, NULL, 0)) < 0)
        {
            fprintf(stderr, "ftdi_usb_get_strings failed: %d (%s)\n", ret, ftdi_get_error_string(ftdic));
            return ERROR;
        }
        printf("Manufacturer: %s, Description: %s\n\n", manufacturer, description);
        if (i==index)
            {
            printf("Found device index %d\n",i);
            dev_found=TRUE;
            break;
            }
        curdev = curdev->next;
    }

    ftdi_list_free(&devlist);
    
    if(!dev_found)
        {
        printf("Device not found\n");
        
        ftdi_deinit(ftdic);
        return FALSE;
        }

    return TRUE;
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
    }

    
    return total_read;
}

int smart_write(struct ftdi_context* ftdic, unsigned char* buff, size_t size, int udelay)
{

    int ret = 0;
    int idx;
    printf("write: ");
    hexdump(buff, size, 0);

    struct timeval start, stop, taken;
    gettimeofday(&start, NULL);

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

    gettimeofday(&stop, NULL);
    timersub(&stop, &start, &taken);
    printf(" took %u.000%u seconds\n", (unsigned int) taken.tv_sec, (unsigned int) taken.tv_usec);
    return ret;
}

void EnableSmartReader(struct ftdi_context* ftdic, int clock, unsigned short Fi, unsigned char Di, unsigned char Ni, unsigned char inv) {

    int ret = 0;
    unsigned char buff[4];
    int delay=50000;
    
    pthread_mutex_lock(&g_usb_mutex);
    ret = ftdi_set_baudrate(ftdic, 9600);
    ret = ftdi_set_line_property(ftdic, (enum ftdi_bits_type) 5, STOP_BIT_2, NONE);
    pthread_mutex_unlock(&g_usb_mutex);

    unsigned char FiDi[] = {0x01, HIBYTE(Fi), LOBYTE(Fi), Di};
    ret = smart_write(ftdic, FiDi, sizeof (FiDi),-1);
    usleep(delay);

    unsigned short freqk = (unsigned short) (clock / 1000);
    unsigned char Freq[] = {0x02, HIBYTE(freqk), LOBYTE(freqk)};
    ret = smart_write(ftdic, Freq, sizeof (Freq),-1);
    usleep(delay);

    unsigned char N[] = {0x03, Ni};
    ret = smart_write(ftdic, N, sizeof (N),-1);
    usleep(delay);

    unsigned char Prot[] = {0x04, 0x00};
    ret = smart_write(ftdic, Prot, sizeof (Prot),-1);
    usleep(delay);

    unsigned char Invert[] = {0x05, inv};
    ret = smart_write(ftdic, Invert, sizeof (Invert),-1);
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
    EnableSmartReader(ftdic, 3571200, 372, 1, 0, 0);
    // set smartreader+ default values 
    sr_config.F=372; 
    sr_config.D=1.0; 
    sr_config.fs=3571200; 
    sr_config.N=0; 
    sr_config.T=0; 
    sr_config.inv=0; 

}

void* ReaderThread(void *p)
{

    struct ftdi_context* ftdic = (struct ftdi_context*)p;
    bool running = TRUE;
    int ret;
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
        pthread_yield_np();

        if(ret>2){  //FTDI always sends modem status bytes as first 2 chars with the 232BM
            pthread_mutex_lock(&g_read_mutex);

            int copy_size = sizeof(g_read_buffer) - g_read_buffer_size > ret-2 ?ret-2: sizeof(g_read_buffer) - g_read_buffer_size;
            memcpy(g_read_buffer+g_read_buffer_size,local_buffer+2,copy_size);
            g_read_buffer_size += copy_size;            
            //printf("Transferred %u bytes to read buffer - current buffer = ",copy_size); hexdump(g_read_buffer,g_read_buffer_size); printf("\n");
            pthread_mutex_unlock(&g_read_mutex);
        }else{
            //sleep for 50ms since there was nothing to read last time
            usleep(50000);
        }
    }

    pthread_exit(NULL);
}


#endif
