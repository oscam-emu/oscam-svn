#include <globals.h>

#include <reader/common.h>
#include <reader/serial.h>

uchar cta_cmd[272], cta_res[260], atr[64];
ushort cta_lr, atr_size = 0;

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
	return reader_serial_doapi(dad, buf, l, dbg);
}

int reader_chkicc(uchar * buf, int l)
{
	return reader_doapi(1, buf, l, D_WATCHDOG);
}

int reader_cmd2api(uchar * buf, int l)
{
	return reader_doapi(1, buf, l, D_DEVICE);
}

int reader_cmd2icc(uchar * buf, int l)
{
//      int rc;
//      if ((rc=reader_doapi(0, buf, l, D_DEVICE)) < 0)
	return (reader_doapi(0, buf, l, D_DEVICE));
//      else
//      return rc;
}

static int reader_activate_card()
{
	reader_serial_activate_card();
}

void reader_card_info()
{
	int rc = -1;

	if (rc = reader_checkhealth()) {
		client[cs_idx].last = time((time_t) 0);
		cs_ri_brk(0);
		cam_common_card_info();
	}
//  return(rc);
}

static int reader_reset(void)
{
	reader_nullcard();
	if (!reader_activate_card())
		return (0);
	int rc = cam_common_get_cardsystem();
	cs_ri_brk(1);
	return rc;
}

static int reader_card_inserted(void)
{
	return reader_serial_card_inserted();
}

int reader_device_init(char *device, int typ)
{
	return reader_serial_device_init(device, typ);
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
			rc = cam_common_ecm(er);
		} else
			rc = 0;
	}

	return rc;
}

int reader_emm(EMM_PACKET * ep)
{
	int rc = -1;

	if (rc = reader_checkhealth()) {
		client[cs_idx].last = time((time_t) 0);
		rc = cam_common_emm(ep);
	}

	return rc;
}
