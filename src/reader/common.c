#include <globals.h>
#include <reader/common.h>

#include <reader/serial.h>
#include <simples.h>

static void reader_common_nullcard()
{
	reader[ridx].card_system = 0;
	memset(reader[ridx].hexserial, 0, sizeof (reader[ridx].hexserial));
	memset(reader[ridx].prid, 0xFF, sizeof (reader[ridx].prid));
	memset(reader[ridx].caid, 0, sizeof (reader[ridx].caid));
	memset(reader[ridx].availkeys, 0, sizeof (reader[ridx].availkeys));
	reader[ridx].acs = 0;
	reader[ridx].nprov = 0;
#ifdef CS_RDR_INIT_HIST
	reader[ridx].init_history_pos = 0;
	memset(reader[ridx].init_history, 0, sizeof (reader[ridx].init_history));
#endif
}

static int reader_common_activate_card(uchar *atr, ushort *atr_size)
{
	// TODO: detect if this is a serial reader
	return reader_serial_activate_card(atr, atr_size);
}

static int reader_common_reset()
{
	reader_common_nullcard();

	uchar atr[33];		// Max 33 bytes according to ISO/IEC 7816-3
	ushort atr_size = 0;
	if (!reader_common_activate_card(atr, &atr_size))
		return 0;

	int rc = cam_common_detect_card_system(atr, atr_size);
	cs_ri_brk(1);

	return rc;
}

static int reader_common_card_is_inserted()
{
	/* Check that we don't have "disabled" this reader */
	char filename[255];
	if (strrchr (reader[ridx].device, '/')) {
		snprintf(filename, sizeof(filename), "%sdisable-%s", cs_confdir, strrchr(reader[ridx].device, '/')+1);
		if (file_exists(filename)) return 0;
	}
	snprintf(filename, sizeof(filename), "%sdisable-%s", cs_confdir, reader[ridx].label);
	if (file_exists(filename)) return 0;

	// TODO: detect if this is a serial reader
	return reader_serial_card_is_inserted();
}

int reader_common_init(struct s_reader *reader)
{
	// TODO: detect if this is a serial reader
	return reader_serial_init(reader);
}

void reader_common_card_info()
{
	int rc = -1;

	if (rc = reader_common_check_health()) {
		client[cs_idx].last = time((time_t) 0);
		cs_ri_brk(0);
		cam_common_card_info();
	}
}

int reader_common_check_health()
{
	if (reader_common_card_is_inserted()) {
		if (!(reader[ridx].card_status & CARD_INSERTED)) {
			cs_log("card detected");
			reader[ridx].card_status = CARD_NEED_INIT;
			reader[ridx].card_status = CARD_INSERTED | (reader_common_reset() ? 0 : CARD_FAILURE);
			if (reader[ridx].card_status & CARD_FAILURE) {
				cs_log("card initializing error");
			} else {
				client[cs_idx].au = ridx;
				reader_common_card_info();
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
			reader_common_nullcard();
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

int reader_common_ecm2cam(ECM_REQUEST * er)
{
	int rc = -1;

	if (rc = reader_common_check_health()) {
		if ((reader[ridx].caid[0] >> 8) == ((er->caid >> 8) & 0xFF)) {
			client[cs_idx].last_srvid = er->srvid;
			client[cs_idx].last_caid = er->caid;
			client[cs_idx].last = time((time_t) 0);
			rc = cam_common_process_ecm(er);
		} else
			rc = 0;
	}

	return rc;
}

int reader_common_emm2cam(EMM_PACKET * ep)
{
	int rc = -1;

	if (rc = reader_common_check_health()) {
		client[cs_idx].last = time((time_t) 0);
		rc = cam_common_process_emm(ep);
	}

	return rc;
}

int reader_common_cmd2card(uchar *cmd, ushort cmd_size, uchar *result, ushort result_max_size, ushort *result_size)
{
	if (reader[ridx].type & R_IS_SERIAL)
		return (reader_serial_cmd2card(cmd, cmd_size, result, result_max_size, result_size) == 0);

	return 0;
}
