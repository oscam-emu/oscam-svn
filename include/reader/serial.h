#ifndef __READER_SERIAL_H__
#  define __READER_SERIAL_H__

int reader_serial_doapi(uchar, uchar *, int, int);
int reader_serial_activate_card();
int reader_serial_card_inserted();
int reader_serial_device_init(char *, int);

#endif // __READER_SERIAL_H__
