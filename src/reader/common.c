#include "globals.h"
#include "reader/common.h"

#include "reader/serial.h"

#include "CAM/common.h"

#include "simples.h"
#include "log.h"

#include <signal.h>
#include <time.h>

static void reader_common_clear_memory(struct s_reader *reader)
{
	reader->online = 0;
	reader->card_status = 0;
	reader->card_system = 0;
	memset(reader->card_atr, 0, sizeof (reader->card_atr));
	reader->card_atr_size = 0;

	memset(reader->hexserial, 0, sizeof (reader->hexserial));
	memset(reader->prid, 0xFF, sizeof (reader->prid));
	memset(reader->caid, 0, sizeof (reader->caid));
	memset(reader->availkeys, 0, sizeof (reader->availkeys));
	reader->acs = 0;
	reader->nprov = 0;

	client[cs_idx].lastemm = 0;
	client[cs_idx].lastecm = 0;
	client[cs_idx].au = -1;
}

static int reader_common_card_is_inserted(struct s_reader *reader)
{
	int rc = 0;

	/* Check that we don't have "disabled" this reader */
	char filename[255];
	if (strrchr (reader->device, '/')) {
		snprintf(filename, sizeof(filename), "%sdisable-%s", cs_confdir, strrchr(reader->device, '/')+1);
		if (file_exists(filename)) return 0;
	}
	snprintf(filename, sizeof(filename), "%sdisable-%s", cs_confdir, reader->label);
	if (file_exists(filename)) return 0;

	if ((reader->type & R_IS_SERIAL) != 0) {
		rc = reader_serial_card_is_inserted();
	}

	return rc;
}

static int reader_common_get_atr(struct s_reader *reader)
{
	int rc = 0;

	if ((reader->type & R_IS_SERIAL) != 0) {
		rc = reader_serial_get_atr(reader->card_atr, &reader->card_atr_size);
	}

	if (rc) {
		log_normal("Reader: ATR = %s", cs_hexdump(1, reader->card_atr, reader->card_atr_size));
	}

	return rc;
}

static int reader_common_get_bitrates(struct s_reader *reader)
{
	int rc = 0;
	unsigned long reader_bitrate_optimal;
	unsigned long reader_bitrate_effective;

	if ((reader->type & R_IS_SERIAL) != 0) {
		rc = reader_serial_get_bitrates(&reader_bitrate_optimal, &reader_bitrate_effective);
	}

	if (rc) {
		if (reader_bitrate_effective == reader_bitrate_optimal) {
			log_normal("Reader: Using optimal bitrate of %lu bit/s", reader_bitrate_optimal);
		} else {
			log_normal("Reader: Using approximate bitrate of %lu bit/s", reader_bitrate_effective);
			log_normal("Reader: Optimal bitrate is %lu bit/s (%+.2f%% off)", reader_bitrate_optimal, (((double) reader_bitrate_effective) - reader_bitrate_optimal) / reader_bitrate_optimal * 100);
		}
	}

	return rc;
}

static int reader_common_init_card(struct s_reader *reader)
{
	/* Get Answer to Reset from card */
	if (!reader_common_get_atr(reader)) {
		return 0;
	}

	/* Show some information about bitrate */
	if (!reader_common_get_bitrates(reader)) {
		return 0;
	}

	/* Detect the card system */
	if (!cam_common_detect(reader->card_atr, reader->card_atr_size)) {
		return 0;
	}

	/* Load information from card */
	reader_common_load_card(reader);

	return 1;
}

int reader_common_init(struct s_reader *reader)
{
	int rc = 0;

	if ((reader->type & R_IS_SERIAL) != 0) {
		rc = reader_serial_init(reader);
		log_normal("Reader: Initialized serial reader %s (%s @ %2.2f Mhz %s%s)", reader->label, reader->device, (float) reader->frequency / 1000000, reader->detect & 0x80 ? "!" : "", RDR_CD_TXT[reader->detect & 0x7f]);
	}

	return rc;
}

void reader_common_load_card(struct s_reader *reader)
{
	reader_common_check_health(reader);

	if (reader->card_status == CARD_INSERTED) {
		/* Disable the reader if it was already online */
		reader->online = 0;

		client[cs_idx].last = time((time_t) 0);

		/* Ask the CAM to load the card information */
		if (cam_common_load_card()) {
			/* Mark the reader as online */
			reader->online = 1;

			log_normal("Reader: Ready for requests (%s)", reader->label);
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
			log_normal("Reader: Card detected in %s", reader->label);

			/* Try to initialize the card */
			if (!reader_common_init_card(reader)) {
				reader->card_status |= CARD_FAILURE;
				log_normal("Reader: Cannot initialize card in %s !", reader->label);
			} else {
				client[cs_idx].au = ridx;
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
		if ((reader->card_status & CARD_INSERTED) != 0) {
			log_normal("Reader: Card ejected from %s", reader->label);

			/* Clear all infos from card */
			reader_common_clear_memory(reader);
		}
	}
}

int reader_common_process_ecm(struct s_reader *reader, ECM_REQUEST *er)
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

int reader_common_process_emm(struct s_reader *reader, EMM_PACKET *ep)
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
	int rc = 0;

	if ((reader->type & R_IS_SERIAL) != 0) {
		rc = (reader_serial_cmd2card(cmd, cmd_size, result, result_max_size, result_size) == 0);
	}

	return rc;
}
