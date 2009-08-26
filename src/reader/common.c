#include "globals.h"
#include "reader/common.h"

#include "reader/serial.h"

#include "CAM/common.h"

#include "simples.h"
#include "log.h"

#include <signal.h>
#include <time.h>

extern int io_serial_need_dummy_char;

static void reader_common_clear_memory(struct s_reader *reader)
{
	reader->online = 0;
	reader->card_status = 0;
	reader->card_system = 0;

	memset(reader->hexserial, 0, sizeof (reader->hexserial));
	memset(reader->prid, 0xFF, sizeof (reader->prid));
	memset(reader->caid, 0, sizeof (reader->caid));
	memset(reader->availkeys, 0, sizeof (reader->availkeys));
	reader->acs = 0;
	reader->nprov = 0;

	client[cs_idx].lastemm = 0;
	client[cs_idx].lastecm = 0;
	client[cs_idx].au = -1;

	io_serial_need_dummy_char = 0;
}

static int reader_common_activate_card(struct s_reader *reader, uchar *atr, ushort *atr_size)
{
	if (reader->type & R_IS_SERIAL) {
		if (!reader_serial_reset()) {
			return 0;
		}

		/* Check if card is inserted */
		if (!reader_serial_card_is_inserted()) {
			return 0;
		}

		return reader_serial_get_atr(atr, atr_size);
	}

	return 0;
}

static int reader_common_reset(struct s_reader *reader)
{
	reader_common_clear_memory(reader);

	uchar atr[33];		// Max 33 bytes according to ISO/IEC 7816-3
	ushort atr_size = 0;
	if (!reader_common_activate_card(reader, atr, &atr_size))
		return 0;

	return cam_common_detect_card_system(atr, atr_size);
}

static int reader_common_card_is_inserted(struct s_reader *reader)
{
	/* Check that we don't have "disabled" this reader */
	char filename[255];
	if (strrchr (reader->device, '/')) {
		snprintf(filename, sizeof(filename), "%sdisable-%s", cs_confdir, strrchr(reader->device, '/')+1);
		if (file_exists(filename)) return 0;
	}
	snprintf(filename, sizeof(filename), "%sdisable-%s", cs_confdir, reader->label);
	if (file_exists(filename)) return 0;

	// TODO: detect if this is a serial reader
	return reader_serial_card_is_inserted();
}

int reader_common_init(struct s_reader *reader)
{
	if (reader->type & R_IS_SERIAL)
		return reader_serial_init(reader);

	return 0;
}

void reader_common_card_info(struct s_reader *reader)
{
	reader_common_check_health(reader);

	if (reader->card_status == CARD_INSERTED) {
		client[cs_idx].last = time((time_t) 0);
		if (cam_common_card_info()) {
			/* Mark the reader as online */
			reader[ridx].online = 1;
		}
	}
}

void reader_common_check_health(struct s_reader *reader)
{
	/* Check if there is a card inserted in the reader */
	if (reader_common_card_is_inserted(reader)) {
		/* Check if card was just inserted */
		if ((reader->card_status & CARD_INSERTED) == 0) {
			reader->card_status = CARD_INSERTED;
			cs_log("card detected in %s", reader->label);

			/* Try to initialize the card */
			if (!reader_common_reset(reader)) {
				reader->card_status |= CARD_FAILURE;
				cs_log("card initializing error for %s", reader->label);
			} else {
				client[cs_idx].au = ridx;
				reader_common_card_info(reader);
			}

			/* ? */
			int i;
			for (i = 1; i < CS_MAXPID; i++) {
				if (client[i].pid && client[i].typ == 'c' && client[i].usr[0]) {
					kill(client[i].pid, SIGQUIT);
				}
			}
		}
	} else {
		/* Check if card was just ejected */
		if (reader->card_status & CARD_INSERTED) {
			cs_log("card ejected from %s", reader->label);

			/* Clear all infos from card */
			reader_common_clear_memory(reader);
		}
	}
}

int reader_common_ecm2cam(struct s_reader *reader, ECM_REQUEST * er)
{
	int rc = 0;

	if (reader->online) {
		if ((reader->caid[0] >> 8) == ((er->caid >> 8) & 0xFF)) {		// TODO: move this somewhere else
			client[cs_idx].last_srvid = er->srvid;
			client[cs_idx].last_caid = er->caid;
			client[cs_idx].last = time((time_t) 0);
			rc = cam_common_process_ecm(er);
		}
	}

	return rc;
}

int reader_common_emm2cam(struct s_reader *reader, EMM_PACKET * ep)
{
	int rc = 0;

	if (reader->online) {
		client[cs_idx].last = time((time_t) 0);
		rc = cam_common_process_emm(ep);
	}

	return rc;
}

int reader_common_cmd2card(struct s_reader *reader, uchar *cmd, ushort cmd_size, uchar *result, ushort result_max_size, ushort *result_size)
{
	if (reader->type & R_IS_SERIAL)
		return (reader_serial_cmd2card(cmd, cmd_size, result, result_max_size, result_size) == 0);

	return 0;
}
