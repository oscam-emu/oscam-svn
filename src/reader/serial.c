#include <globals.h>
#include <reader/serial.h>

/* CT-API */
#include <ctapi.h>
#include <defines.h>
#include <ctbcs.h>

#define CTAPI_CTN 1

char reader_serial_device[128];		// UGLY : to be removed
int reader_serial_irdeto_mode;		// UGLY : to be removed
int reader_serial_card_detect;		// UGLY : to be removed
int reader_serial_mhz;			// UGLY : to be removed

static ushort reader_serial_get_reader_type(struct s_reader *reader)
{
	ushort reader_type = RTYP_STD;
#ifdef TUXBOX
	struct stat sb;
#endif

	switch (reader->typ) {
		case R_MOUSE:
			reader_type = RTYP_STD;
#ifdef TUXBOX
			if (!stat(reader->device, &sb)) {
				if (S_ISCHR(sb.st_mode)) {
					int dev_major, dev_minor;
					dev_major = major(sb.st_rdev);
					dev_minor = minor(sb.st_rdev);

					if (cs_hw == CS_HW_DBOX2 && (dev_major == 4 || dev_major == 5)) {
						switch (dev_minor & 0x3F) {
							case 0:
								reader_type = RTYP_DB2COM1;
								break;
							case 1:
								reader_type = RTYP_DB2COM2;
								break;
						}
					}

					cs_debug("device is major: %d, minor: %d, reader_type=%d", dev_major, dev_minor, reader_type);
				}
			}
#endif
			break;

		case R_SMART:
			reader_type = RTYP_SMART;
			break;

		case R_INTERN:
			reader_type = RTYP_SCI;
			break;
	}

	return reader_type;
}

static int reader_serial_cmd2api(uchar dad, uchar *cmd, ushort cmd_size, uchar *result, ushort result_max_size, ushort *result_size, int dbg)
{
	char ret;
	uchar sad = 2;

	// Set result_size to the size of the result buffer (result_max_size)
	*result_size = result_max_size;

	// Save and Change cs_ptyp
	int cs_ptyp_orig = cs_ptyp;
	cs_ptyp = dbg;

//	cs_ddump(cmd, cmd_size, "send %d bytes to ctapi", cmd_size);

	// Call CT-API
	ret = CT_data(
		CTAPI_CTN,	/* Terminal Number */
		&dad,		/* Destination */
		&sad,		/* Source */
		cmd_size,	/* Length of command */
		cmd,		/* Command/Data Buffer */
		result_size,	/* Length of Response */
		result);	/* Response */

//	cs_ddump(result, *result_size, "received %d bytes from ctapi with ret=%d", *result_size, ret);

	// Restore cs_ptyp
	cs_ptyp = cs_ptyp_orig;

	return ret;
}

int reader_serial_cmd2card(uchar *cmd, ushort cmd_size, uchar *result, ushort result_max_size, ushort *result_size)
{
	return reader_serial_cmd2api(0, cmd, cmd_size, result, result_max_size, result_size, D_DEVICE);
}

static int reader_serial_cmd2reader(uchar *cmd, ushort cmd_size, uchar *result, ushort result_max_size, ushort *result_size)
{
	return reader_serial_cmd2api(1, cmd, cmd_size, result, result_max_size, result_size, D_DEVICE);
}

int reader_serial_activate_card(uchar *atr, ushort *atr_size)
{
	int i;
	char ret;
	uchar cmd[5];
	uchar result[260];
	ushort result_size;

	/* Reset CardTerminal */
	cmd[0] = CTBCS_INS_RESET;
	cmd[1] = CTBCS_P2_RESET_GET_ATR;
	cmd[2] = 0x00;

	ret = reader_serial_cmd2reader(cmd, 3, result, sizeof(result), &result_size);
	if (ret != OK) {
		cs_log("Error reset terminal: %d", ret);
		return 0;
	}

	/* Check if card is inserted */
	if (!reader_serial_card_is_inserted()) {
		return 0;
	}

	/* Try to get ATR from card */
	for (i = 1; i <= 5; i++) {
		reader_serial_irdeto_mode = i % 2;

		/* Request ICC */
		cmd[0] = CTBCS_CLA;
		cmd[1] = CTBCS_INS_REQUEST;
		cmd[2] = CTBCS_P1_INTERFACE1;
		cmd[3] = CTBCS_P2_REQUEST_GET_ATR;
		cmd[4] = 0x00;

		ret = reader_serial_cmd2reader(cmd, 5, result, sizeof(result), &result_size);
		if (ret == OK && result_size > 0) {
			break;
		} else {
			cs_log("Error activating card: %d", ret);
			cs_sleepms(500);

			/* Cannot get the ATR */
			if (i == 5) {
				return 0;
			}
		}
	}

	/* Store Answer to Reset */
	*atr_size = result_size - 2;
	memcpy(atr, result, *atr_size);
	cs_ri_log("ATR: %s", cs_hexdump(1, atr, *atr_size));

	return 1;
}

int reader_serial_card_is_inserted()
{
	uchar cmd[5];
	uchar result[260];
	ushort result_size;

	/* Get Status of CardTerminal */
	cmd[0] = CTBCS_CLA;
	cmd[1] = CTBCS_INS_STATUS;
	cmd[2] = CTBCS_P1_CT_KERNEL;
	cmd[3] = CTBCS_P2_STATUS_ICC;
	cmd[4] = 0x00;

	char ret = reader_serial_cmd2reader(cmd, 5, result, sizeof(result), &result_size);
	if (ret != OK) {
		cs_log("Error getting status of terminal: %d", ret);
	}

	return (ret == OK && result[0] == CTBCS_DATA_STATUS_CARD_CONNECT);
}

int reader_serial_init(struct s_reader *reader)
{
	char ret;

	// Set some extern variables to be used by CT-API
	snprintf(reader_serial_device, sizeof (reader_serial_device), "%s", reader->device);
	reader_serial_card_detect = reader->detect;
	reader_serial_mhz = reader->mhz;

	// Save and Change cs_ptyp
	int cs_ptyp_orig = cs_ptyp;
	cs_ptyp = D_DEVICE;

	// Lookup Port Number
	ushort reader_type = reader_serial_get_reader_type(reader);
	if ((ret = CT_init(CTAPI_CTN, PORT_COM1, reader_type)) != OK) {
		cs_log("Cannot open device: %s", reader->device);
	}
	cs_debug("CT_init on %s: %d", reader->device, ret);

	// Restore cs_ptyp
	cs_ptyp = cs_ptyp_orig;

	return ret == OK;
}
