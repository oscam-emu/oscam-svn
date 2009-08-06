#include <globals.h>
#include <reader/common.h>
#include <reader/serial.h>

#include <ctapi.h>
#include <ctbcs.h>

static int cs_ptyp_orig;	// reinit=1
int reader_irdeto_mode;		// UGLY : to be removed

char oscam_device[128];
int oscam_card_detect;
int mhz;

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


int reader_serial_doapi(uchar dad, uchar * buf, int l, int dbg)
{
	int rc;
	uchar sad;

//	oscam_card_inserted=4;
	sad = 2;
	cta_lr = sizeof (cta_res) - 1;
	cs_ptyp_orig = cs_ptyp;
	cs_ptyp = dbg;
//	cs_ddump(buf, l, "send %d bytes to ctapi", l);
	rc = CT_data(1, &dad, &sad, l, buf, &cta_lr, cta_res);
//	cs_ddump(cta_res, cta_lr, "received %d bytes from ctapi with rc=%d", cta_lr, rc);
	cs_ptyp = cs_ptyp_orig;

	return rc;
}

int reader_serial_activate_card()
{
	int i;
	char ret;

	cta_cmd[0] = CTBCS_INS_RESET;
	cta_cmd[1] = CTBCS_P2_RESET_GET_ATR;
	cta_cmd[2] = 0x00;

	ret = reader_cmd2api(cta_cmd, 3);
	if (ret != OK) {
		cs_log("Error reset terminal: %d", ret);
		return (0);
	}

	cta_cmd[0] = CTBCS_CLA;
	cta_cmd[1] = CTBCS_INS_STATUS;
	cta_cmd[2] = CTBCS_P1_CT_KERNEL;
	cta_cmd[3] = CTBCS_P2_STATUS_ICC;
	cta_cmd[4] = 0x00;

//	ret=reader_cmd2api(cmd, 11); warum 11 ??????
	ret = reader_cmd2api(cta_cmd, 5);
	if (ret != OK) {
		cs_log("Error getting status of terminal: %d", ret);
		return (0);
	}
	if (cta_res[0] != CTBCS_DATA_STATUS_CARD_CONNECT)
		return (0);

	/* Activate card */
//	for (i = 0; i < 5 && ((ret!=OK) || (cta_res[cta_lr-2]!=0x90)) ; i++)
	for (i = 0; i < 5; i++) {
		reader_irdeto_mode = i % 2 == 1;
		cta_cmd[0] = CTBCS_CLA;
		cta_cmd[1] = CTBCS_INS_REQUEST;
		cta_cmd[2] = CTBCS_P1_INTERFACE1;
		cta_cmd[3] = CTBCS_P2_REQUEST_GET_ATR;
		cta_cmd[4] = 0x00;

		ret = reader_cmd2api(cta_cmd, 5);
		if ((ret == OK) || (cta_res[cta_lr - 2] == 0x90)) {
			i = 100;
			break;
		}
		cs_log("Error activating card: %d", ret);
		cs_sleepms(500);
	}
	if (i < 100)
		return (0);

	/* Store ATR */
	atr_size = cta_lr - 2;
	memcpy(atr, cta_res, atr_size);
#ifdef CS_RDR_INIT_HIST
	reader[ridx].init_history_pos = 0;
	memset(reader[ridx].init_history, 0, sizeof (reader[ridx].init_history));
#endif
	cs_ri_log("ATR: %s", cs_hexdump(1, atr, atr_size));
	sleep(1);
	return (1);
}

int reader_serial_card_inserted(void)
{
	cta_cmd[0] = CTBCS_CLA;
	cta_cmd[1] = CTBCS_INS_STATUS;
	cta_cmd[2] = CTBCS_P1_INTERFACE1;
	cta_cmd[3] = CTBCS_P2_STATUS_ICC;
	cta_cmd[4] = 0x00;

	return reader_chkicc(cta_cmd, 5) ? 0 : cta_res[0];
}

int reader_serial_device_init(char *device, int typ)
{
	int rc;

	oscam_card_detect = reader[ridx].detect;
	mhz = reader[ridx].mhz;
	cs_ptyp_orig = cs_ptyp;
	cs_ptyp = D_DEVICE;
	snprintf(oscam_device, sizeof (oscam_device), "%s", device);
	if ((rc = CT_init(1, reader_serial_device_type(device, typ), reader[ridx].typ)) != OK)
		cs_log("Cannot open device: %s", device);
	cs_debug("ct_init on %s: %d", device, rc);
	cs_ptyp = cs_ptyp_orig;

	return (rc != OK) ? 2 : 0;
}
