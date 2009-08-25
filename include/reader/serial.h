#ifndef __READER_SERIAL_H__
#  define __READER_SERIAL_H__

int reader_serial_cmd2card(uchar *, ushort, uchar *, ushort, ushort *);

int reader_serial_activate_card(uchar *, ushort *);
int reader_serial_card_is_inserted();
int reader_serial_init(struct s_reader *);

#endif // __READER_SERIAL_H__
