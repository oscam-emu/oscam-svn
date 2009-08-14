#ifndef __READER_SERIAL_H__
#  define __READER_SERIAL_H__

int reader_serial_cmd2card(uchar *, ushort, uchar *, ushort, ushort *);
int reader_serial_cmd2reader(uchar *, ushort, uchar *, ushort, ushort *);

int reader_serial_activate_card(uchar *, ushort *);
int reader_serial_card_inserted();
int reader_serial_device_init(char *, int);

#endif // __READER_SERIAL_H__
