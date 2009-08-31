#include "globals.h"
#include "CAM/common.h"

#include "CAM/conax.h"
#include "CAM/cryptoworks.h"
#include "CAM/irdeto.h"
#include "CAM/seca.h"
#include "CAM/viaccess.h"
#include "CAM/videoguard.h"

#include "reader/common.h"

#include "simples.h"
#include "log.h"

ushort cam_common_len4caid[256];		// table for guessing caid (by len)

typedef enum {
	CAM_UNKNOWN,

	CAM_CONAX,
	CAM_CRYPTOWORKS,
	CAM_IRDETO,
	CAM_SECA,
	CAM_VIACCESS,
	CAM_VIDEOGUARD
} cam_common_card_system;

int cam_common_detect(uchar *atr, ushort atr_size)
{
	cam_common_card_system card_system = CAM_UNKNOWN;

	/* Test each CAM to detect the card_system */
	if (cam_conax_detect(atr, atr_size)) {
		card_system = CAM_CONAX;
		cs_log("CAM: Conax detected");
	} else if (cam_cryptoworks_detect(atr, atr_size)) {
		card_system = CAM_CRYPTOWORKS;
		cs_log("CAM: Cryptoworks detected");
	} else if (cam_irdeto_detect(atr, atr_size)) {
		card_system = CAM_IRDETO;
		cs_log("CAM: Irdeto detected");
	} else if (cam_seca_detect(atr, atr_size)) {
		card_system = CAM_SECA;
		cs_log("CAM: Seca detected");
	} else if (cam_viaccess_detect(atr, atr_size)) {
		card_system = CAM_VIACCESS;
		cs_log("CAM: Viaccess detected");
	} else if (cam_videoguard_detect(atr, atr_size)) {
		card_system = CAM_VIDEOGUARD;
		cs_log("CAM: Videoguard detected");
	} else {
		cs_log("CAM: Card system not supported !");
	}

	/* Save the card_system value for the reader */
	reader[ridx].card_system = card_system;

	return card_system;
}

int cam_common_load_card()
{
	int rc = 0;

	cs_log("CAM: Loading card ...");

	switch (reader[ridx].card_system) {
		case CAM_CONAX:
			rc = cam_conax_load_card();
			break;
		case CAM_CRYPTOWORKS:
			rc = cam_cryptoworks_load_card();
			break;
		case CAM_IRDETO:
			rc = cam_irdeto_load_card();
			break;
		case CAM_SECA:
			rc = cam_seca_load_card();
			break;
		case CAM_VIACCESS:
			rc = cam_viaccess_load_card();
			break;
		case CAM_VIDEOGUARD:
			rc = cam_videoguard_load_card();
			break;
	}

	if (rc) {
		cs_log("CAM: Card information loaded");
	} else {
		cs_log("CAM: Cannot load card information !");
	}

	return rc;
}

int cam_common_process_ecm(ECM_REQUEST * er)
{
	int rc = 0;

	switch (reader[ridx].card_system) {
		case CAM_CONAX:
			rc = cam_conax_process_ecm(er);
			break;
		case CAM_CRYPTOWORKS:
			rc = cam_cryptoworks_process_ecm(er);
			break;
		case CAM_IRDETO:
			rc = cam_irdeto_process_ecm(er);
			break;
		case CAM_SECA:
			rc = cam_seca_process_ecm(er);
			break;
		case CAM_VIACCESS:
			rc = cam_viaccess_process_ecm(er);
			break;
		case CAM_VIDEOGUARD:
			rc = cam_videoguard_process_ecm(er);
			break;
	}

	return rc;
}

int cam_common_process_emm(EMM_PACKET * ep)
{
	int rc = 0;

	switch (reader[ridx].card_system) {
		case CAM_CONAX:
			rc = cam_conax_process_emm(ep);
			break;
		case CAM_CRYPTOWORKS:
			rc = cam_cryptoworks_process_emm(ep);
			break;
		case CAM_IRDETO:
			rc = cam_irdeto_process_emm(ep);
			break;
		case CAM_SECA:
			rc = cam_seca_process_emm(ep);
			break;
		case CAM_VIACCESS:
			rc = cam_viaccess_process_emm(ep);
			break;
		case CAM_VIDEOGUARD:
			rc = cam_videoguard_process_emm(ep);
			break;
	}

	return rc;
}

int cam_common_cmd2card(uchar *cmd, ushort cmd_size, uchar *result, ushort result_max_size, ushort *result_size)
{
	// Forward to the reader
	return reader_common_cmd2card(&reader[ridx], cmd, cmd_size, result, result_max_size, result_size);
}

ulong cam_common_get_provider_id(uchar *ecm, ushort caid)
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

	return provid;
}

void cam_common_guess_card_system(ECM_REQUEST * er)
{
	ushort last_hope = 0;

	// viaccess - check by provid-search
	if ((er->prid = cam_common_get_provider_id(er->ecm, 0x500)))
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
	if (!er->caid && er->ecm[2]==0x31 && er->ecm[0x0b]==0x28) {
		uchar  b3;
		int    b47;
		//ushort chid;
		struct s_irdeto_guess {
			int b47;
			ushort caid;
			ushort sid;
			struct s_irdeto_quess *next;
		};
		struct s_irdeto_guess *ptr;

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
			if (b47 == ptr->b47) {
				if (er->srvid && (er->srvid!=ptr->sid)) {
					cs_debug("sid mismatched (ecm: %04X, guess: %04X), wrong oscam.ird file?", er->srvid, ptr->sid);
					return;
				}
				er->caid = ptr->caid;
				er->srvid = ptr->sid;
				er->chid = (ushort) ptr->b47;
//				cs_debug("guess_irdeto() found caid=%04X, sid=%04X, chid=%04X", er->caid, er->srvid, er->chid);
				return;
			}
			ptr = ptr->next;
		}
	}
*/

	if (!er->caid)	// guess by len
		er->caid = cam_common_len4caid[er->ecm[2] + 3];

	if (!er->caid)
		er->caid = last_hope;
}
