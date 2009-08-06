#include "globals.h"
#include "CAM/common.h"

char oscam_device[128];
int oscam_card_detect;
int mhz;
int reader_irdeto_mode;

uchar cta_cmd[272], cta_res[260], atr[64];
ushort cta_lr, atr_size = 0;

ushort len4caid[256];           // table for guessing caid (by len)

static int cs_ptyp_orig;	//reinit=1, 

#define SC_IRDETO 1
#define SC_CRYPTOWORKS 2
#define SC_VIACCESS 3
#define SC_CONAX 4
#define SC_SECA 5
#define SC_VIDEOGUARD2 6

static int reader_device_type(char *device, int typ)
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
	return (rc);
}

static void reader_nullcard(void)
{
	reader[ridx].card_system = 0;
	memset(reader[ridx].hexserial, 0, sizeof (reader[ridx].hexserial));
	memset(reader[ridx].prid, 0xFF, sizeof (reader[ridx].prid));
	memset(reader[ridx].caid, 0, sizeof (reader[ridx].caid));
	memset(reader[ridx].availkeys, 0, sizeof (reader[ridx].availkeys));
	reader[ridx].acs = 0;
	reader[ridx].nprov = 0;
}

int reader_doapi(uchar dad, uchar * buf, int l, int dbg)
{
	int rc;
	uchar sad;

//  oscam_card_inserted=4;
	sad = 2;
	cta_lr = sizeof (cta_res) - 1;
	cs_ptyp_orig = cs_ptyp;
	cs_ptyp = dbg;
	//cs_ddump(buf, l, "send %d bytes to ctapi", l);
	rc = CT_data(1, &dad, &sad, l, buf, &cta_lr, cta_res);
	//cs_ddump(cta_res, cta_lr, "received %d bytes from ctapi with rc=%d", cta_lr, rc);
	cs_ptyp = cs_ptyp_orig;
	return (rc);
}

int reader_chkicc(uchar * buf, int l)
{
	return (reader_doapi(1, buf, l, D_WATCHDOG));
}

int reader_cmd2api(uchar * buf, int l)
{
	return (reader_doapi(1, buf, l, D_DEVICE));
}

int reader_cmd2icc(uchar * buf, int l)
{
//  int rc;
//  if ((rc=reader_doapi(0, buf, l, D_DEVICE))<0)
	return (reader_doapi(0, buf, l, D_DEVICE));
//  else
//    return(rc);
}

static int reader_activate_card()
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

//  ret=reader_cmd2api(cmd, 11); warum 11 ??????
	ret = reader_cmd2api(cta_cmd, 5);
	if (ret != OK) {
		cs_log("Error getting status of terminal: %d", ret);
		return (0);
	}
	if (cta_res[0] != CTBCS_DATA_STATUS_CARD_CONNECT)
		return (0);

	/* Activate card */
//  for (i=0; (i<5) && ((ret!=OK)||(cta_res[cta_lr-2]!=0x90)); i++)
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

void reader_card_info()
{
	int rc = -1;

	if (rc = reader_checkhealth()) {
		client[cs_idx].last = time((time_t) 0);
		cs_ri_brk(0);
		switch (reader[ridx].card_system) {
			case SC_IRDETO:
				rc = irdeto_card_info();
				break;
			case SC_CRYPTOWORKS:
				rc = cryptoworks_card_info();
				break;
			case SC_VIACCESS:
				rc = viaccess_card_info();
				break;
			case SC_CONAX:
				rc = conax_card_info();
				break;
			case SC_VIDEOGUARD2:
				rc = videoguard_card_info();
				break;
			case SC_SECA:
				rc = seca_card_info();
				break;
			default:
				rc = 0;
		}
	}
//  return(rc);
}

static int reader_get_cardsystem(void)
{
	if (irdeto_card_init(atr, atr_size))
		reader[ridx].card_system = SC_IRDETO;
	if (conax_card_init(atr, atr_size))
		reader[ridx].card_system = SC_CONAX;
	if (cryptoworks_card_init(atr, atr_size))
		reader[ridx].card_system = SC_CRYPTOWORKS;
	if (seca_card_init(atr, atr_size))
		reader[ridx].card_system = SC_SECA;
	if (viaccess_card_init(atr, atr_size))
		reader[ridx].card_system = SC_VIACCESS;
	if (videoguard_card_init(atr, atr_size))
		reader[ridx].card_system = SC_VIDEOGUARD2;
	if (!reader[ridx].card_system)
		cs_ri_log("card system not supported");
	cs_ri_brk(1);
	return (reader[ridx].card_system);
}

static int reader_reset(void)
{
	reader_nullcard();
	if (!reader_activate_card())
		return (0);
	return (reader_get_cardsystem());
}

static int reader_card_inserted(void)
{
	cta_cmd[0] = CTBCS_CLA;
	cta_cmd[1] = CTBCS_INS_STATUS;
	cta_cmd[2] = CTBCS_P1_INTERFACE1;
	cta_cmd[3] = CTBCS_P2_STATUS_ICC;
	cta_cmd[4] = 0x00;

	return (reader_chkicc(cta_cmd, 5) ? 0 : cta_res[0]);
}

int reader_device_init(char *device, int typ)
{
	int rc;

	oscam_card_detect = reader[ridx].detect;
	mhz = reader[ridx].mhz;
	cs_ptyp_orig = cs_ptyp;
	cs_ptyp = D_DEVICE;
	snprintf(oscam_device, sizeof (oscam_device), "%s", device);
	if ((rc = CT_init(1, reader_device_type(device, typ), reader[ridx].typ)) != OK)
		cs_log("Cannot open device: %s", device);
	cs_debug("ct_init on %s: %d", device, rc);
	cs_ptyp = cs_ptyp_orig;
	return ((rc != OK) ? 2 : 0);
}

int reader_checkhealth(void)
{
	if (reader_card_inserted()) {
		if (!(reader[ridx].card_status & CARD_INSERTED)) {
			cs_log("card detected");
			reader[ridx].card_status = CARD_NEED_INIT;
			reader[ridx].card_status = CARD_INSERTED | (reader_reset()? 0 : CARD_FAILURE);
			if (reader[ridx].card_status & CARD_FAILURE) {
				cs_log("card initializing error");
			} else {
				client[cs_idx].au = ridx;
				reader_card_info();
			}

			int i;

			for (i = 1; i < CS_MAXPID; i++) {
				if (client[i].pid && client[i].typ == 'c' && client[i].usr[0]) {
					kill(client[i].pid, SIGQUIT);
				}
			}
		}
	} else {
		if (reader[ridx].card_status & CARD_INSERTED) {
			reader_nullcard();
			client[cs_idx].lastemm = 0;
			client[cs_idx].lastecm = 0;
			client[cs_idx].au = -1;
			extern int io_serial_need_dummy_char;

			io_serial_need_dummy_char = 0;
			cs_log("card ejected");
		}
		reader[ridx].card_status = 0;
		reader[ridx].online = 0;
	}
	return reader[ridx].card_status == CARD_INSERTED;
}

int reader_ecm(ECM_REQUEST * er)
{
	int rc = -1;

	if ((rc = reader_checkhealth())) {
		if ((reader[ridx].caid[0] >> 8) == ((er->caid >> 8) & 0xFF)) {
			client[cs_idx].last_srvid = er->srvid;
			client[cs_idx].last_caid = er->caid;
			client[cs_idx].last = time((time_t) 0);
			switch (reader[ridx].card_system) {
				case SC_IRDETO:
					rc = (irdeto_do_ecm(er)) ? 1 : 0;
					break;
				case SC_CRYPTOWORKS:
					rc = (cryptoworks_do_ecm(er)) ? 1 : 0;
					break;
				case SC_VIACCESS:
					rc = (viaccess_do_ecm(er)) ? 1 : 0;
					break;
				case SC_CONAX:
					rc = (conax_do_ecm(er)) ? 1 : 0;
					break;
				case SC_SECA:
					rc = (seca_do_ecm(er)) ? 1 : 0;
					break;
				case SC_VIDEOGUARD2:
					rc = (videoguard_do_ecm(er)) ? 1 : 0;
					break;
				default:
					rc = 0;
			}
		} else
			rc = 0;
	}
	return (rc);
}

int reader_emm(EMM_PACKET * ep)
{
	int rc = -1;

	if (rc = reader_checkhealth()) {
		client[cs_idx].last = time((time_t) 0);
		switch (reader[ridx].card_system) {
			case SC_IRDETO:
				rc = irdeto_do_emm(ep);
				break;
			case SC_CRYPTOWORKS:
				rc = cryptoworks_do_emm(ep);
				break;
			case SC_VIACCESS:
				rc = viaccess_do_emm(ep);
				break;
			case SC_CONAX:
				rc = conax_do_emm(ep);
				break;
			case SC_SECA:
				rc = seca_do_emm(ep);
				break;
			case SC_VIDEOGUARD2:
				rc = videoguard_do_emm(ep);
				break;
			default:
				rc = 0;
		}
	}
	return (rc);
}

ulong chk_provid(uchar * ecm, ushort caid)
{
	int i;
	ulong provid = 0;

	switch (caid) {
		case 0x100:	// seca
			provid = b2i(2, ecm + 3);
			break;
		case 0x500:	// viaccess
			i = (ecm[4] == 0xD2) ? ecm[5] + 2 : 0;	// skip d2 nano
			if ((ecm[5 + i] == 3) && ((ecm[4 + i] == 0x90) || (ecm[4 + i] == 0x40)))
				provid = (b2i(3, ecm + 6 + i) & 0xFFFFF0);
		default:
			// cryptoworks ?
			if (caid & 0x0d00 && ecm[8] == 0x83 && ecm[9] == 1)
				provid = (ulong) ecm[10];
	}
	return (provid);
}

/*
void guess_irdeto(ECM_REQUEST *er)
{
  uchar  b3;
  int    b47;
  //ushort chid;
  struct s_irdeto_quess *ptr;

  b3  = er->ecm[3];
  ptr = cfg->itab[b3];
  if( !ptr ) {
    cs_debug("unknown irdeto byte 3: %02X", b3);
    return;
  }
  b47  = b2i(4, er->ecm+4);
  //chid = b2i(2, er->ecm+6);
  //cs_debug("ecm: b47=%08X, ptr->b47=%08X, ptr->caid=%04X", b47, ptr->b47, ptr->caid);
  while( ptr )
  {
    if( b47==ptr->b47 )
    {
      if( er->srvid && (er->srvid!=ptr->sid) )
      {
        cs_debug("sid mismatched (ecm: %04X, guess: %04X), wrong oscam.ird file?",
                  er->srvid, ptr->sid);
        return;
      }
      er->caid=ptr->caid;
      er->srvid=ptr->sid;
      er->chid=(ushort)ptr->b47;
//      cs_debug("quess_irdeto() found caid=%04X, sid=%04X, chid=%04X",
//               er->caid, er->srvid, er->chid);
      return;
    }
    ptr=ptr->next;
  }
}
*/

void guess_cardsystem(ECM_REQUEST * er)
{
	ushort last_hope = 0;

	// viaccess - check by provid-search
	if ((er->prid = chk_provid(er->ecm, 0x500)))
		er->caid = 0x500;

	// nagra
	// is ecm[1] always 0x30 ?
	// is ecm[3] always 0x07 ?
	if ((er->ecm[6] == 1) && (er->ecm[4] == er->ecm[2] - 2))
		er->caid = 0x1801;

	// seca2 - very poor
	if ((er->ecm[8] == 0x10) && ((er->ecm[9] & 0xF1) == 1))
		last_hope = 0x100;

	// is cryptoworks, but which caid ?
	if ((er->ecm[3] == 0x81) && (er->ecm[4] == 0xFF) && (!er->ecm[5]) && (!er->ecm[6]) && (er->ecm[7] == er->ecm[2] - 5))
		last_hope = 0xd00;

/*
  if (!er->caid && er->ecm[2]==0x31 && er->ecm[0x0b]==0x28)
    guess_irdeto(er);
*/

	if (!er->caid)	// guess by len ..
		er->caid = len4caid[er->ecm[2] + 3];

	if (!er->caid)
		er->caid = last_hope;
}
