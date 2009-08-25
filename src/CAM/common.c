#include <globals.h>
#include <CAM/common.h>

#include <reader/common.h>

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

int cam_common_detect_card_system(uchar *atr, ushort atr_size)
{
	cam_common_card_system card_system = CAM_UNKNOWN;

	// Test each CAM to detect the card_system
	if (conax_card_init(atr, atr_size)) {
		card_system = CAM_CONAX;
	} else if (cryptoworks_card_init(atr, atr_size)) {
		card_system = CAM_CRYPTOWORKS;
	} else if (irdeto_card_init(atr, atr_size)) {
		card_system = CAM_IRDETO;
	} else if (seca_card_init(atr, atr_size)) {
		card_system = CAM_SECA;
	} else if (viaccess_card_init(atr, atr_size)) {
		card_system = CAM_VIACCESS;
	} else if (videoguard_card_init(atr, atr_size)) {
		card_system = CAM_VIDEOGUARD;
	} else {
		cs_ri_log("card system not supported");
	}

	// Save the card_system value for the reader
	reader[ridx].card_system = card_system;

	return card_system;
}

void cam_common_card_info()
{
	switch (reader[ridx].card_system) {
		case CAM_CONAX:
			conax_card_info();
			break;
		case CAM_CRYPTOWORKS:
			cryptoworks_card_info();
			break;
		case CAM_IRDETO:
			irdeto_card_info();
			break;
		case CAM_SECA:
			seca_card_info();
			break;
		case CAM_VIACCESS:
			viaccess_card_info();
			break;
		case CAM_VIDEOGUARD:
			videoguard_card_info();
			break;
	}
}

int cam_common_process_ecm(ECM_REQUEST * er)
{
	int rc = -1;

	switch (reader[ridx].card_system) {
		case CAM_CONAX:
			rc = (conax_do_ecm(er)) ? 1 : 0;
			break;
		case CAM_CRYPTOWORKS:
			rc = (cryptoworks_do_ecm(er)) ? 1 : 0;
			break;
		case CAM_IRDETO:
			rc = (irdeto_do_ecm(er)) ? 1 : 0;
			break;
		case CAM_SECA:
			rc = (seca_do_ecm(er)) ? 1 : 0;
			break;
		case CAM_VIACCESS:
			rc = (viaccess_do_ecm(er)) ? 1 : 0;
			break;
		case CAM_VIDEOGUARD:
			rc = (videoguard_do_ecm(er)) ? 1 : 0;
			break;
		default:
			rc = 0;
	}

	return rc;
}

int cam_common_process_emm(EMM_PACKET * ep)
{
	int rc = -1;

	switch (reader[ridx].card_system) {
		case CAM_CONAX:
			rc = conax_do_emm(ep);
			break;
		case CAM_CRYPTOWORKS:
			rc = cryptoworks_do_emm(ep);
			break;
		case CAM_IRDETO:
			rc = irdeto_do_emm(ep);
			break;
		case CAM_SECA:
			rc = seca_do_emm(ep);
			break;
		case CAM_VIACCESS:
			rc = viaccess_do_emm(ep);
			break;
		case CAM_VIDEOGUARD:
			rc = videoguard_do_emm(ep);
			break;
		default:
			rc = 0;
	}

	return rc;
}

int cam_common_cmd2card(uchar *cmd, ushort cmd_size, uchar *result, ushort result_max_size, ushort *result_size)
{
	// Forward to the reader
	return reader_common_cmd2card(cmd, cmd_size, result, result_max_size, result_size);
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

	if (!er->caid)	// guess by len ..
		er->caid = cam_common_len4caid[er->ecm[2] + 3];

	if (!er->caid)
		er->caid = last_hope;
}
