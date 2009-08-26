#include "globals.h"
#include "CAM/conax.h"
#include "CAM/common.h"

#include "simples.h"
#include "log.h"

#define MAX_LEN 256

/*
static unsigned int Conax_ToDate(char data0, char data1)
{	// decimal: yyyymmdd
	int y, m, d;
	unsigned int l;

	y = 1990 + ((data1 >> 4) + ((data0 >> 5) & 0x7) * 10);
	m = data1 & 0xf;
	d = data0 & 0x1f;
	l = (y * 100 + m) * 100 + d;
	return l;
}
*/

static char *chid_date(uchar *ptr, char *buf, int l)
{
	if (buf) {
		snprintf(buf, l, "%04d/%02d/%02d", 1990 + (ptr[1] >> 4) + (((ptr[0] >> 5) & 7) * 10), ptr[1] & 0xf, ptr[0] & 0x1f);
	}
	return (buf);
}

static int read_record(uchar *cmd, ushort cmd_size, uchar *result, ushort result_max_size, ushort *result_size)
{
	uchar insCA[] = { 0xDD, 0xCA, 0x00, 0x00, 0x00 };

	cam_common_cmd2card(cmd, cmd_size, result, result_max_size, result_size);	// select record
	if (result[0] != 0x98)
		return (-1);
	insCA[4] = result[1];	// get len
	cam_common_cmd2card(insCA, sizeof(insCA), result, 260, result_size);	// read record
	if ((result[*result_size - 2] != 0x90) || (result[*result_size - 1]))
		return -1;
	return (*result_size - 2);
}

static int CheckSctLen(const uchar * data, int off)
{
	int l = SCT_LEN(data);

	if (l + off > MAX_LEN) {
		cs_debug("smartcard: section too long %d > %d", l, MAX_LEN - off);
		l = -1;
	}

	return l;
}

static int conax_send_pin()
{
	unsigned char insPIN[] = { 0xDD, 0xC8, 0x00, 0x00, 0x07, 0x1D, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00 };	//letzte vier ist der Pin-Code
	memcpy(insPIN + 8, reader[ridx].pincode, 4);

	uchar result[260];
	ushort result_size;
	cam_common_cmd2card(insPIN, sizeof(insPIN), result, sizeof(result), &result_size);
	cs_log("[conax]-sending pincode to card");

	return 1;
}

int cam_conax_card_init(uchar *atr, ushort atr_size)
{
	int i, j, n;
	uchar atr_0b00[] = { '0', 'B', '0', '0' };
	uchar ins26[] = { 0xDD, 0x26, 0x00, 0x00, 0x03, 0x10, 0x01, 0x40 };
	uchar ins82[] = { 0xDD, 0x82, 0x00, 0x00, 0x11, 0x11, 0x0f, 0x01, 0xb0, 0x0f, 0xff,
		0xff, 0xfb, 0x00, 0x00, 0x09, 0x04, 0x0b, 0x00, 0xe0, 0x30, 0x2b
	};

	uchar cardver = 0;

	if ((memcmp(atr + 3, atr_0b00, sizeof (atr_0b00))) && (memcmp(atr + 4, atr_0b00, sizeof (atr_0b00))))
		return (0);

	reader[ridx].caid[0] = 0xB00;

	uchar result[260];
	ushort result_size;
	if ((n = read_record(ins26, sizeof(ins26), result, sizeof(result), &result_size)) < 0)
		return (0);	// read caid, card-version
	for (i = 0; i < n; i += result[i + 1] + 2)
		switch (result[i]) {
			case 0x20:
				cardver = result[i + 2];
				break;
			case 0x28:
				reader[ridx].caid[0] = (result[i + 2] << 8) | result[i + 3];
		}

	if ((n = read_record(ins82, sizeof(ins82), result, sizeof(result), &result_size)) < 0)
		return (0);	// read serial

	for (j = 0, i = 2; i < n; i += result[i + 1] + 2)
		switch (result[i]) {

			case 0x09:
				reader[ridx].prid[j][0] = 0x00;
				reader[ridx].prid[j][1] = 0x00;
				reader[ridx].prid[j][2] = result[i + 4];
				reader[ridx].prid[j][3] = result[i + 5];

				break;
			case 0x23:
				if (result[i + 5] != 0x00) {
					memcpy(reader[ridx].hexserial, &result[i + 3], 6);
				} else {

					memcpy(reader[ridx].sa[j], &result[i + 5], 4);
					j++;
				}
				break;
		}



	reader[ridx].nprov = j;

	cs_log("type: conax, caid: %04X, serial: %llu, card: v%d", reader[ridx].caid[0], b2ll(6, reader[ridx].hexserial), cardver);
	cs_log("Conax-Provider:%d", reader[ridx].nprov);

	for (j = 0; j < reader[ridx].nprov; j++) {
		cs_log("Provider:%d  Provider-Id:%06X", j + 1, b2ll(4, reader[ridx].prid[j]));
		cs_log("Provider:%d  SharedAddress:%08X", j + 1, b2ll(4, reader[ridx].sa[j]));
	}

	return 1;
}

int cam_conax_load_card_info()
{
	int type, i, j, k, n = 0;
	ushort provid;
	char provname[32], pdate[32];
	uchar insC6[] = { 0xDD, 0xC6, 0x00, 0x00, 0x03, 0x1C, 0x01, 0x00 };
	uchar ins26[] = { 0xDD, 0x26, 0x00, 0x00, 0x03, 0x1C, 0x01, 0x01 };
	uchar insCA[] = { 0xDD, 0xCA, 0x00, 0x00, 0x00 };
	char *txt[] = { "Package", "PPV-Event" };
	uchar *cmd[] = { insC6, ins26 };

	for (type = 0; type < 2; type++) {
		n = 0;
		uchar result[260];
		ushort result_size;
		cam_common_cmd2card(cmd[type], 5 + cmd[type][4], result, sizeof(result), &result_size);
		while (result[result_size - 2] == 0x98) {
			insCA[4] = result[1];	// get len
			cam_common_cmd2card(insCA, sizeof(insCA), result, sizeof(result), &result_size);	// read
			if ((result[result_size - 2] == 0x90) || (result[result_size - 2] == 0x98)) {
				for (j = 0; j < result_size - 2; j += result[j + 1] + 2) {
					provid = (result[j + 2 + type] << 8) | result[j + 3 + type];
					for (k = 0, i = j + 4 + type; (i < j + result[j + 1]) && (k < 2); i += result[i + 1] + 2) {
						int l;

						switch (result[i]) {
							case 0x01:
								l = (result[i + 1] < (sizeof (provname) - 1)) ? result[i + 1] : sizeof (provname) - 1;
								memcpy(provname, result + i + 2, l);
								provname[l] = '\0';
								break;
							case 0x30:
								chid_date(result + i + 2, pdate + (k++ << 4), 15);
								break;
						}
					}
					cs_log("%s: %d, id: %04X, date: %s - %s, name: %s", txt[type], ++n, provid, pdate, pdate + 16, trim(provname));
				}
			}
		}
	}

	return 1;
}

int cam_conax_process_ecm(ECM_REQUEST * er)
{
	int i, j, n, rc = 0;
	unsigned char insA2[] = { 0xDD, 0xA2, 0x00, 0x00, 0x00 };
	unsigned char insCA[] = { 0xDD, 0xCA, 0x00, 0x00, 0x00 };

	unsigned char buf[256];

	if ((n = CheckSctLen(er->ecm, 3)) < 0)
		return 0;

	buf[0] = 0x14;
	buf[1] = n + 1;
	buf[2] = 0;

	memcpy(buf + 3, er->ecm, n);
	insA2[4] = n + 3;

	uchar result[260];
	ushort result_size;
	uchar insA2_cmd[272];
	memcpy(insA2_cmd, insA2, 5);
	memcpy(insA2_cmd + 5, buf, insA2[4]);
	cam_common_cmd2card(insA2_cmd, 5 + insA2[4], result, sizeof(result), &result_size);	// write Header + ECM

	while ((result[result_size - 2] == 0x98) &&	// Antwort 
	       ((insCA[4] = result[result_size - 1]) > 0) && (insCA[4] != 0xFF)) {
		cam_common_cmd2card(insCA, sizeof(insCA), result, sizeof(result), &result_size);	//Codeword auslesen

		if ((result[result_size - 2] == 0x98) || ((result[result_size - 2] == 0x90))) {
			for (i = 0; i < result_size - 2; i += result[i + 1] + 2)

				switch (result[i]) {
					case 0x25:
						if ((result[i + 1] >= 0xD) && !((n = result[i + 4]) & 0xFE)) {
							rc |= (1 << n);
							memcpy(er->cw + (n << 3), result + i + 7, 8);
						}
						break;
					case 0x31:
						if ((result[i + 1] == 0x02 && result[i + 2] == 0x00 && result[i + 3] == 0x00) || (result[i + 1] == 0x02 && result[i + 2] == 0x40 && result[i + 3] == 0x00)) {
							break;
						} else if (strcmp(reader[ridx].pincode, "none")) {
							conax_send_pin();
							memcpy(insA2_cmd + 5, buf, insA2[4]);
							cam_common_cmd2card(insA2_cmd, 5 + insA2[4], result, sizeof(result), &result_size);	// write Header + ECM
							while ((result[result_size - 2] == 0x98) &&	// Antwort
							       ((insCA[4] = result[result_size - 1]) > 0) && (insCA[4] != 0xFF)) {
								cam_common_cmd2card(insCA, sizeof(insCA), result, sizeof(result), &result_size);	//Codeword auslesen
								if ((result[result_size - 2] == 0x98) || ((result[result_size - 2] == 0x90) && (!result[result_size - 1]))) {

									for (j = 0; j < result_size - 2; j += result[j + 1] + 2)
										if ((result[j] == 0x25) &&	// access: is cw
										    (result[j + 1] >= 0xD) &&	// 0xD: 5 header + 8 cw
										    !((n = result[j + 4]) & 0xFE))	// cw idx must be 0 or 1
										{
											rc |= (1 << n);
											memcpy(er->cw + (n << 3), result + j + 7, 8);
										}
								}
							}

						}
						break;
				}

		}
	}

	return (rc == 3);
}

int cam_conax_process_emm(EMM_PACKET * ep)
{
	/* by KrazyIvan
	 *   EMM with lenght 83 and 85, is the same as ECM and PPV.
	 *     EMM with lenght AA and A8 (keyupdate).
	 *       82 70 82 00 00 00 00 2c 86 52 70 79 64 10 16 bc
	 *         */

	unsigned char insEMM[] = { 0xDD, 0x84, 0x00, 0x00, 0x00 };
	unsigned char cmd[260];
	unsigned char *buf = cmd + 5;
	memcpy(cmd, insEMM, 5);
	int rc = 0;

	int l = ep->emm[2];

	ep->type = l + 3;

	insEMM[4] = l + 5;
	buf[0] = 0x12;
	buf[1] = l + 3;
	memcpy(buf + 2, ep->emm, buf[1]);
	uchar result[260];
	ushort result_size;
	cam_common_cmd2card(cmd, 5 + insEMM[4], result, sizeof(result), &result_size);
	rc = ((result[0] == 0x90) && (result[1] == 0x00));

	return rc;
}
