#ifndef __READER_SERIAL_H__
#  define __READER_SERIAL_H__

int reader_serial_chkicc(uchar *, int);
int reader_serial_cmd2api(uchar *, int);
int reader_serial_cmd2icc(uchar *, int);

int reader_serial_activate_card();
int reader_serial_card_inserted();
int reader_serial_device_init(char *, int);

#endif // __READER_SERIAL_H__
