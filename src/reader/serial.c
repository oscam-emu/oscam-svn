#include <globals.h>
#include <reader/serial.h>

#include <ctapi.h>
#include <ctbcs.h>

char reader_serial_device[128];		// UGLY : to be removed
int reader_serial_irdeto_mode;		// UGLY : to be removed
int reader_serial_card_detect;		// UGLY : to be removed
int reader_serial_mhz;			// UGLY : to be removed

static int reader_serial_device_type(char *device, int typ)
{
	int rc = PORT_STD;

#ifdef TUXBOX
	struct stat sb;
#endif

	switch (reader[ridx].typ) {
		case R_MOUSE:
		case R_SMART:
			rc = PORT_STD;
#ifdef TUXBOX
			if (!stat(device, &sb)) {
				if (S_ISCHR(sb.st_mode)) {
					int dev_major, dev_minor;

					dev_major = major(sb.st_rdev);
					dev_minor = minor(sb.st_rdev);
					if ((cs_hw == CS_HW_DBOX2) && ((dev_major == 4) || (dev_major == 5)))
						switch (dev_minor & 0x3F) {
							case 0:
								rc = PORT_DB2COM1;
								break;
							case 1:
								rc = PORT_DB2COM2;
								break;
						}
					cs_debug("device is major: %d, minor: %d, typ=%d", dev_major, dev_minor, rc);
				}
			}
#endif
			break;
		case R_INTERN:
			rc = PORT_SCI;
			break;
	}

	return rc;
}

static int reader_serial_do_api(uchar dad, uchar *cmd, ushort cmd_size, uchar *result, ushort result_max_size, ushort *result_size, int dbg)
{
	int rc;
	unsigned short ctn = 1;
	unsigned char sad = 2;

	// Set result_size to the size of the result buffer (result_max_size)
	*result_size = result_max_size;

	// Save and Change cs_ptyp
	int cs_ptyp_orig = cs_ptyp;
	cs_ptyp = dbg;

//	cs_ddump(cmd, cmd_size, "send %d bytes to ctapi", cmd_size);

	// Call CSCTAPI
	rc = CT_data(
		ctn,		/* Terminal Number */
		&dad,		/* Destination */
		&sad,		/* Source */
		cmd_size,	/* Length of command */
		cmd,		/* Command/Data Buffer */
		result_size,	/* Length of Response */
		result);	/* Response */

//	cs_ddump(result, *result_size, "received %d bytes from ctapi with rc=%d", *result_size, rc);

	// Restore cs_ptyp
	cs_ptyp = cs_ptyp_orig;

	return rc;
}

int reader_serial_cmd2card(uchar *cmd, ushort cmd_size, uchar *result, ushort result_max_size, ushort *result_size)
{
	return reader_serial_do_api(0, cmd, cmd_size, result, result_max_size, result_size, D_DEVICE);
}

static int reader_serial_cmd2reader(uchar *cmd, ushort cmd_size, uchar *result, ushort result_max_size, ushort *result_size)
{
	return reader_serial_do_api(1, cmd, cmd_size, result, result_max_size, result_size, D_DEVICE);
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

	/* Get Status of CardTerminal */
	cmd[0] = CTBCS_CLA;
	cmd[1] = CTBCS_INS_STATUS;
	cmd[2] = CTBCS_P1_CT_KERNEL;
	cmd[3] = CTBCS_P2_STATUS_ICC;
	cmd[4] = 0x00;

	ret = reader_serial_cmd2reader(cmd, 5, result, sizeof(result), &result_size);
	if (ret != OK) {
		cs_log("Error getting status of terminal: %d", ret);
		return 0;
	}

	/* Check status of CardTerminal */
	if (result[0] != CTBCS_DATA_STATUS_CARD_CONNECT) {
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

int reader_serial_card_inserted()
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

	int ret = reader_serial_cmd2reader(cmd, 5, result, sizeof(result), &result_size);
	return (ret == OK && result[0] == CTBCS_DATA_STATUS_CARD_CONNECT);
}

int reader_serial_device_init(char *device, int typ)
{
	int rc;

	reader_serial_card_detect = reader[ridx].detect;
	reader_serial_mhz = reader[ridx].mhz;
	int cs_ptyp_orig = cs_ptyp;
	cs_ptyp = D_DEVICE;
	snprintf(reader_serial_device, sizeof (reader_serial_device), "%s", device);
	if ((rc = CT_init(1, reader_serial_device_type(device, typ), reader[ridx].typ)) != OK)
		cs_log("Cannot open device: %s", device);
	cs_debug("ct_init on %s: %d", device, rc);
	cs_ptyp = cs_ptyp_orig;

	return (rc != OK) ? 2 : 0;
}
