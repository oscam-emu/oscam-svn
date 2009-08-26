#include "globals.h"
#include "CAM/cryptoworks.h"
#include "CAM/common.h"

#include "simples.h"
#include "log.h"
#include "config.h"		// UGLY : to be removed !

#include <stdlib.h>

#define MAX_LEN 256

//static uchar ISK[0x40];
static uchar cwexp[] = { 1, 0, 1 };
static BIGNUM exp, ucpk;
static int ucpk_valid = 0;

static void RotateBytes1(unsigned char *out, unsigned char *in, int n)
{
	// loop is executed atleast once, so it's not a good idea to
	// call with n=0 !!
	out += n;
	do {
		*(--out) = *(in++);
	} while (--n);
}

static void RotateBytes2(unsigned char *in, int n)
{
	// loop is executed atleast once, so it's not a good idea to
	// call with n=0 !!
	unsigned char *e = in + n - 1;

	do {
		unsigned char temp = *in;

		*in++ = *e;
		*e-- = temp;
	} while (in < e);
}

static int Input(BIGNUM * d, unsigned char *in, int n, int LE)
{
	if (LE) {
		unsigned char tmp[n];

		RotateBytes1(tmp, in, n);
		return (BN_bin2bn(tmp, n, d) != 0);
	} else
		return (BN_bin2bn(in, n, d) != 0);
}

static int Output(unsigned char *out, int n, BIGNUM * r, int LE)
{
	int s = BN_num_bytes(r);

	if (s > n) {
		unsigned char buff[s];

		cs_debug("rsa: RSA len %d > %d, truncating", s, n);
		BN_bn2bin(r, buff);
		memcpy(out, buff + s - n, n);
	} else if (s < n) {
		int l = n - s;

		cs_debug("rsa: RSA len %d < %d, padding", s, n);
		memset(out, 0, l);
		BN_bn2bin(r, out + l);
	} else
		BN_bn2bin(r, out);
	if (LE)
		RotateBytes2(out, n);
	return (s);
}

static int RSA(unsigned char *out, unsigned char *in, int n, BIGNUM * exp, BIGNUM * mod, int LE)
{
	int rc = 0;
	BN_CTX *ctx;
	BIGNUM *r, *d;

	ctx = BN_CTX_new();
	r = BN_new();
	d = BN_new();
	if (Input(d, in, n, LE)) {
		if (BN_mod_exp(r, d, exp, mod, ctx))
			rc = Output(out, n, r, LE);
		else
			cs_log("rsa: mod-exp failed");
	}
	BN_CTX_free(ctx);
	BN_free(d);
	BN_free(r);
	return (rc);
}

static int CheckSctLen(const uchar * data, int off)
{
	int l = SCT_LEN(data);

	if (l + off > MAX_LEN) {
		cs_debug("smartcard: section too long %d > %d", l, MAX_LEN - off);
		l = -1;
	}
	return (l);
}

static char *chid_date(uchar * ptr, char *buf, int l)
{
	if (buf) {
		snprintf(buf, l, "%04d/%02d/%02d", 1990 + (ptr[0] >> 1), ((ptr[0] & 1) << 3) | (ptr[1] >> 5), ptr[1] & 0x1f);
	}
	return (buf);
}

static int select_file(uchar f1, uchar f2)
{
	uchar insA4[] = { 0xA4, 0xA4, 0x00, 0x00, 0x02, 0x00, 0x00 };
	uchar result[260];
	ushort result_size;

	insA4[5] = f1;
	insA4[6] = f2;
	cam_common_cmd2card(insA4, sizeof(insA4), result, sizeof(result), &result_size);	// select file

	return ((result[0] == 0x9f) && (result[1] == 0x11));
}

static int read_record(uchar rec)
{
	uchar insA2[] = { 0xA4, 0xA2, 0x00, 0x00, 0x01, 0x00 };
	uchar insB2[] = { 0xA4, 0xB2, 0x00, 0x00, 0x00 };
	uchar result[260];
	ushort result_size;

	insA2[5] = rec;
	cam_common_cmd2card(insA2, sizeof(insA2), result, sizeof(result), &result_size);	// select record
	if (result[0] != 0x9f)
		return (-1);
	insB2[4] = result[1];	// get len
	cam_common_cmd2card(insB2, sizeof(insB2), result, sizeof(result), &result_size);	// read record
	if ((result[result_size - 2] != 0x90) || (result[result_size - 1]))
		return (-1);
	return (result_size - 2);
}

/*
static int cryptoworks_send_pin()
{
	unsigned char insPIN[] = { 0xA4, 0x20, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00 };	//Verify PIN  
	uchar result[260];
	ushort result_size;

	if (reader[ridx].pincode[0] && (reader[ridx].pincode[0] & 0xF0) == 0x30) {
		memcpy(insPIN + 5, reader[ridx].pincode, 4);

		cam_common_cmd2card(insPIN, sizeof(insPIN), result, sizeof(result), &result_size);
		cs_log("[cryptoworks]-sending pincode to card");
		if ((result[0] == 0x98) && (result[1] == 0x04))
			cs_log("[cryptoworks]-bad pincode");

		return (1);
	}

	return (0);
}
*/

static int cryptoworks_disbale_pin()
{
	unsigned char insPIN[] = { 0xA4, 0x26, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00 };	//disable PIN  
	uchar result[260];
	ushort result_size;

	if (reader[ridx].pincode[0] && (reader[ridx].pincode[0] & 0xF0) == 0x30) {
		memcpy(insPIN + 5, reader[ridx].pincode, 4);

		cam_common_cmd2card(insPIN, sizeof(insPIN), result, sizeof(result), &result_size);
		cs_log("[cryptoworks]-disable pincode to card");
		if ((result[0] == 0x98) && (result[1] == 0x04))
			cs_log("[cryptoworks]-bad pincode");
		return (1);
	}

	return (0);
}

int cryptoworks_card_init(uchar *atr, ushort atr_size)
{
	int i;
	unsigned int mfid = 0x3F20;
	uchar insA4C[] = { 0xA4, 0xC0, 0x00, 0x00, 0x11 };
	uchar insB8[] = { 0xA4, 0xB8, 0x00, 0x00, 0x0c };
	uchar result[260];
	ushort result_size;

	uchar issuerid = 0;
	char issuer[20] = { 0 };
	char *unknown = "unknown", *pin = unknown, ptxt[CS_MAXPROV << 2] = { 0 };

	if ((atr[6] != 0xC4) || (atr[9] != 0x8F) || (atr[10] != 0xF1))
		return (0);

	reader[ridx].caid[0] = 0xD00;
	reader[ridx].nprov = 0;
	memset(reader[ridx].prid, 0, sizeof (reader[ridx].prid));

	cam_common_cmd2card(insA4C, sizeof(insA4C), result, sizeof(result), &result_size);	// read masterfile-ID
	if ((result[0] == 0xDF) && (result[1] >= 6))
		mfid = (result[6] << 8) | result[7];

	select_file(0x3f, 0x20);
	insB8[2] = insB8[3] = 0;	// first
	for (result[0] = 0xdf; result[0] == 0xdf;) {
		cam_common_cmd2card(insB8, sizeof(insB8), result, sizeof(result), &result_size);	// read provider id's
		if (result[0] != 0xdf)
			break;
		if (((result[4] & 0x1f) == 0x1f) && (reader[ridx].nprov < CS_MAXPROV)) {
			sprintf(ptxt + strlen(ptxt), ",%02X", result[5]);
			reader[ridx].prid[reader[ridx].nprov++][3] = result[5];
		}
		insB8[2] = insB8[3] = 0xff;	// next
	}
	for (i = reader[ridx].nprov; i < CS_MAXPROV; i++)
		memset(&reader[ridx].prid[i][0], 0xff, 4);

	select_file(0x2f, 0x01);	// read caid
	if (read_record(0xD1) >= 4)
		reader[ridx].caid[0] = (result[2] << 8) | result[3];

	if (read_record(0x80) >= 7)	// read serial
		memcpy(reader[ridx].hexserial, result + 2, 5);
	cs_log("type: cryptoworks, caid: %04X, ascii serial: %llu, hex serial: %s", reader[ridx].caid[0], b2ll(5, reader[ridx].hexserial), cs_hexdump(0, reader[ridx].hexserial, 5));

	if (read_record(0x9E) >= 66)	// read ISK
	{
		uchar keybuf[256];
		BIGNUM *ipk;

		if (search_boxkey(reader[ridx].caid[0], 0, (char *) keybuf)) {
			ipk = BN_new();
			BN_bin2bn(cwexp, sizeof (cwexp), &exp);
			BN_bin2bn(keybuf, 64, ipk);
			RSA(result + 2, result + 2, 0x40, &exp, ipk, 0);
			BN_free(ipk);
			if ((ucpk_valid = (result[2] == ((mfid & 0xFF) >> 1)))) {
				result[2] |= 0x80;
				BN_bin2bn(result + 2, 0x40, &ucpk);
				cs_ddump(result + 2, 0x40, "IPK available -> session-key:");
			} else {
				if ((ucpk_valid = (keybuf[0] == (((mfid & 0xFF) >> 1) | 0x80)))) {
					BN_bin2bn(keybuf, 0x40, &ucpk);
					cs_ddump(keybuf, 0x40, "session-key found:");
				} else
					cs_log("invalid IPK or session-key for CAID %04X !", reader[ridx].caid[0]);
			}
		}
	}
	if (read_record(0x9F) >= 3)
		issuerid = result[2];
	if (read_record(0xC0) >= 16) {
		strncpy(issuer, (const char *) result + 2, sizeof (issuer) - 1);
		trim(issuer);
	} else
		strcpy(issuer, unknown);

	select_file(0x3f, 0x20);
	select_file(0x2f, 0x11);	// read pin
	if (read_record(atr[8]) >= 7) {
		result[6] = 0;
		pin = (char *) result + 2;
	}
	cs_log("issuer: %s, id: %02X, bios: v%d, pin: %s, mfid: %04X", issuer, issuerid, atr[7], pin, mfid);
	cs_log("providers: %d (%s)", reader[ridx].nprov, ptxt + 1);
	cs_log("ready for requests");

	cryptoworks_disbale_pin();	//by KrazyIvan

	return (1);
}

#ifdef LALL
bool cSmartCardCryptoworks::Decode(const cEcmInfo * ecm, const unsigned char *data, unsigned char *cw)
{
	static unsigned char ins4c[] = { 0xA4, 0x4C, 0x00, 0x00, 0x00 };

	unsigned char nanoD4[10];
	int l = CheckSctLen(data, -5 + (ucpkValid ? sizeof (nanoD4) : 0));

	if (l > 5) {
		unsigned char buff[MAX_LEN];

		if (ucpkValid) {
			memcpy(buff, data, l);
			nanoD4[0] = 0xD4;
			nanoD4[1] = 0x08;
			for (unsigned int i = 2; i < sizeof (nanoD4); i++)
				nanoD4[i] = rand();
			memcpy(&buff[l], nanoD4, sizeof (nanoD4));
			data = buff;
			l += sizeof (nanoD4);
		}
		ins4c[3] = ucpkValid ? 2 : 0;
		ins4c[4] = l - 5;
		if (IsoWrite(ins4c, &data[5]) && Status() && (l = GetLen()) > 0 && ReadData(buff, l) == l) {
			int r = 0;

			for (int i = 0; i < l && r < 2;) {
				int n = buff[i + 1];

				switch (buff[i]) {
					case 0x80:
						de(printf("smartcardcryptoworks: nano 80 (serial)\n"))
							break;
					case 0xD4:
						de(printf("smartcardcryptoworks: nano D4 (rand)\n"))
							if (n < 8 || memcmp(&buff[i], nanoD4, sizeof (nanoD4)))
							di(printf("smartcardcryptoworks: random data check failed after decrypt\n"))
								break;
					case 0xDB:	// CW
						de(printf("smartcardcryptoworks: nano DB (cw)\n"))
							if (n == 0x10) {
							memcpy(cw, &buff[i + 2], 16);
							r |= 1;
						}
						break;
					case 0xDF:	// signature
						de(printf("smartcardcryptoworks: nano DF %02x (sig)\n", n))
							if (n == 0x08) {
							if ((buff[i + 2] & 0x50) == 0x50 && !(buff[i + 3] & 0x01) && (buff[i + 5] & 0x80))
								r |= 2;
						} else if (n == 0x40) {	// camcrypt
							if (ucpkValid) {
								RSA(&buff[i + 2], &buff[i + 2], n, exp, ucpk, false);
								de(printf("smartcardcryptoworks: after camcrypt "))
									de(HexDump(&buff[i + 2], n))
									r = 0;
								l = n - 4;
								n = 4;
							} else {
								di(printf("smartcardcryptoworks: valid UCPK needed for camcrypt!\n"))
									return false;
							}
						}
						break;
					default:
						de(printf("smartcardcryptoworks: nano %02x (unhandled)\n", buff[i]))
							break;
				}
				i += n + 2;
			}
			return r == 3;
		}
	}
	return false;
}
#endif

int cryptoworks_do_ecm(ECM_REQUEST * er)
{
	int r = 0;
	static unsigned char ins4C[] = { 0xA4, 0x4C, 0x00, 0x00, 0x00 };
	static unsigned char insC0[] = { 0xA4, 0xC0, 0x00, 0x00, 0x1C };
	uchar result[260];
	ushort result_size;

	unsigned char nanoD4[10];
	int secLen = CheckSctLen(er->ecm, -5 + (ucpk_valid ? sizeof (nanoD4) : 0));

	if (secLen > 5) {
		int i;
		uchar *ecm = er->ecm;
		uchar buff[MAX_LEN];

		if (ucpk_valid) {
			memcpy(buff, er->ecm, secLen);
			nanoD4[0] = 0xD4;
			nanoD4[1] = 0x08;
			for (i = 2; i < sizeof (nanoD4); i++)
				nanoD4[i] = rand();
			memcpy(&buff[secLen], nanoD4, sizeof (nanoD4));
			ecm = buff;
			secLen += sizeof (nanoD4);
		}

		ins4C[3] = ucpk_valid ? 2 : 0;
		ins4C[4] = secLen - 5;
		uchar cmd[MAX_LEN];
		memcpy(cmd, ins4C, 5);
		memcpy(cmd + 5, ecm + 5, ins4C[4]);
		cam_common_cmd2card(cmd, 5 + ins4C[4], result, sizeof(result), &result_size);
		if (result[result_size - 2] == 0x9f) {
			insC0[4] = result[result_size - 1];
			cam_common_cmd2card(insC0, sizeof(insC0), result, sizeof(result), &result_size);
			for (i = 0; i < secLen && r < 2;) {
				int n = result[i + 1];

				switch (result[i]) {
					case 0x80:
						cs_debug("cryptoworks: nano 80 (serial)");
						break;
					case 0xD4:
						cs_debug("smartcardcryptoworks: nano D4 (rand)");
						if (n < 8 || memcmp(&result[i], nanoD4, sizeof (nanoD4)))
							cs_debug("cryptoworks: random data check failed after decrypt");
						break;
					case 0xDB:	// CW
						cs_debug("smartcardcryptoworks: nano DB (cw)");
						if (n == 0x10) {
							memcpy(er->cw, &result[i + 2], 16);
							r |= 1;
						}
						break;
					case 0xDF:	// signature
						cs_debug("cryptoworks: nano DF %02x (sig)", n);
						if (n == 0x08) {
							if ((result[i + 2] & 0x50) == 0x50 && !(result[i + 3] & 0x01) && (result[i + 5] & 0x80))
								r |= 2;
						} else if (n == 0x40)	// camcrypt
						{
							if (ucpk_valid) {
								RSA(&result[i + 2], &result[i + 2], n, &exp, &ucpk, 0);
								cs_debug("smartcardcryptoworks: after camcrypt ");
								r = 0;
								secLen = n - 4;
								n = 4;
							} else {
								cs_log("cryptoworks: valid UCPK needed for camcrypt!");
								return (0);
							}
						}
						break;
					default:
						cs_debug("smartcardcryptoworks: nano %02x (unhandled)", result[i]);
						break;
				}
				i += n + 2;
			}
		}
#ifdef LALL
#  #######################################################################
		if ((result[result_size - 2] == 0x9f) && (result[result_size - 1] == 0x1c)) {
			cam_common_cmd2card(insC0, sizeof(insC0), result, sizeof(result), &result_size);
			if ((result_size > 26) && (result[result_size - 2] == 0x90) && (result[result_size - 1] == 0)) {
				if (rc = (((result[20] & 0x50) == 0x50) && (!(result[21] & 0x01)) && (result[23] & 0x80)))
					memcpy(er->cw, result + 2, 16);
			}
		}
#endif
	}
//  return(rc ? 1 : 0);
	return ((r == 3) ? 1 : 0);
}

int cryptoworks_do_emm(EMM_PACKET * ep)
{
	uchar insEMM_GA[] = { 0xA4, 0x44, 0x00, 0x00, 0x00 };
	uchar insEMM_SA[] = { 0xA4, 0x48, 0x00, 0x00, 0x00 };
	uchar insEMM_UA[] = { 0xA4, 0x42, 0x00, 0x00, 0x00 };
	uchar cmd[MAX_LEN];
	uchar result[260];
	ushort result_size;

	int rc = 0;
	uchar *emm = ep->emm;

	/* this original    
	   if ((emm[0]==0x8f) && (emm[3]==0xa4))              // emm via camd3.5x
	   {    
	   ep->type=emm[4];
	   cam_common_cmd2card(emm+3, ep->l - 3, result, &result_size);
	   if ((result_size==2) && (result[0]==0x90) && (result[1]==0))
	   rc=1;
	   }
	 */

	//by KrazyIvan 
	ep->type = emm[0];
	//cs_log("EMM Dump:..: %s",cs_hexdump(1, emm, emm[2])); 
	switch (emm[0]) {
			// emm via camd3.5x
		case 0x8F:
			if (emm[3] == 0xA4) {
				ep->type = emm[4];
				//cs_log("EMM Dump: CMD: %s", cs_hexdump(1, emm+3, 5)); 
				//cs_log("EMM Dump: DATA: %s",cs_hexdump(1, emm+8, emm[7]));
				cam_common_cmd2card(emm + 3, 5 + emm[7], result, sizeof(result), &result_size);
				rc = ((result[0] == 0x90) && (result[1] == 0x00));
			}
			break;

			//GA            
		case 0x88:
		case 0x89:
			if (emm[3] == 0xA9 && emm[4] == 0xFF && emm[8] == 0x83 && emm[9] == 0x01) {
				ep->type = insEMM_GA[1];
				insEMM_GA[4] = ep->emm[2] - 2;
				//cs_log("EMM Dump: CMD: %s", cs_hexdump(1, insEMM_GA, 5)); 
				//cs_log("EMM Dump: DATA: %s",cs_hexdump(1, emm+5, insEMM_GA[4]));                              
				//cs_log("EMM Dump: IF: %02X == %02X",emm[7],(insEMM_GA[4]-3));                                                                 

				if (emm[7] == insEMM_GA[4] - 3) {
					memcpy(cmd, insEMM_GA, 5);
					memcpy(cmd + 5, emm + 5, insEMM_GA[4]);
					cam_common_cmd2card(cmd, 5 + insEMM_GA[4], result, sizeof(result), &result_size);
					rc = ((result[0] == 0x90) && (result[1] == 0x00));
				}
			}
			break;

			//SA
		case 0x84:
			if (emm[3] == 0xA9 && emm[4] == 0xFF && emm[12] == 0x80 && emm[13] == 0x04) {
				ep->type = insEMM_SA[1];
				insEMM_SA[4] = ep->emm[2] - 6;
				//cs_log("EMM Dump: CMD: %s", cs_hexdump(1, insEMM_SA, 5)); 
				//cs_log("EMM Dump: DATA: %s",cs_hexdump(1, emm+9, insEMM_SA[4]));                              
				//cs_log("EMM Dump: IF: %02X == %02X",emm[11],(insEMM_SA[4]-3));                                                                

				if (emm[11] == insEMM_SA[4] - 3) {
					memcpy(cmd, insEMM_SA, 5);
					memcpy(cmd + 5, emm + 9, insEMM_SA[4]);
					cam_common_cmd2card(cmd, 5 + insEMM_SA[4], result, sizeof(result), &result_size);
					rc = ((result[0] == 0x90) && (result[1] == 0x00));
				}
			}
			break;

			//UA            
		case 0x82:
			if (emm[3] == 0xA9 && emm[4] == 0xFF && emm[13] == 0x80 && emm[14] == 0x05) {
				ep->type = insEMM_UA[1];
				insEMM_UA[4] = ep->emm[2] - 7;
				//cs_log("EMM Dump: CMD: %s", cs_hexdump(1, insEMM_UA, 5)); 
				//cs_log("EMM Dump: DATA: %s",cs_hexdump(1, emm+10, insEMM_UA[4]));                             
				//cs_log("EMM Dump: IF: %02X == %02X",emm[12],(insEMM_UA[4]-3));                                                                

				if (emm[12] == insEMM_UA[4] - 3) {
					//cryptoworks_send_pin(); //?? may be 
					memcpy(cmd, insEMM_UA, 5);
					memcpy(cmd + 5, emm + 10, insEMM_UA[4]);
					cam_common_cmd2card(cmd, 5 + insEMM_UA[4], result, sizeof(result), &result_size);
					rc = ((result[0] == 0x90) && (result[1] == 0x00));
				}
			}
			break;
	}

	return (rc);
}

int cryptoworks_card_info()
{
	int i;
	uchar insA21[] = { 0xA4, 0xA2, 0x01, 0x00, 0x05, 0x8C, 0x00, 0x00, 0x00, 0x00 };
	uchar insB2[] = { 0xA4, 0xB2, 0x00, 0x00, 0x00 };
	uchar result[260];
	ushort result_size;
	char l_name[20 + 8] = ", name: ";

	cs_log("card detected");
	cs_log("type: cryptoworks");

	for (i = 0; i < reader[ridx].nprov; i++) {
		l_name[8] = 0;
		select_file(0x1f, reader[ridx].prid[i][3]);	// select provider
		select_file(0x0e, 0x11);	// read provider name
		if (read_record(0xD6) >= 16) {
			strncpy(l_name + 8, (const char *) result + 2, sizeof (l_name) - 9);
			l_name[sizeof (l_name) - 1] = 0;
			trim(l_name + 8);
		}
		l_name[0] = (l_name[8]) ? ',' : 0;
		cs_log("provider: %d, id: %02X%s", i + 1, reader[ridx].prid[i][3], l_name);
		select_file(0x0f, 0x20);	// select provider class
		cam_common_cmd2card(insA21, sizeof(insA21), result, sizeof(result), &result_size);
		if (result[0] == 0x9f) {
			insB2[4] = result[1];
			for (insB2[3] = 0; (result[0] != 0x94) || (result[1] != 0x2); insB2[3] = 1) {
				cam_common_cmd2card(insB2, sizeof(insB2), result, sizeof(result), &result_size);	// read chid
				if (result[0] != 0x94) {
					char ds[16], de[16];

					chid_date(result + 28, ds, sizeof (ds) - 1);
					chid_date(result + 30, de, sizeof (de) - 1);
					cs_log("chid: %02X%02X, date: %s - %s, name: %s", result[6], result[7], ds, de, trim((char *) result + 10));
				}
			}
		}
		//================================================================================
		//by KrazyIvan
		select_file(0x0f, 0x00);	// select provider channel 
		cam_common_cmd2card(insA21, sizeof(insA21), result, sizeof(result), &result_size);
		if (result[0] == 0x9f) {
			insB2[4] = result[1];
			for (insB2[3] = 0; (result[0] != 0x94) || (result[1] != 0x2); insB2[3] = 1) {
				cam_common_cmd2card(insB2, sizeof(insB2), result, sizeof(result), &result_size);	// read chid
				if (result[0] != 0x94) {
					char ds[16], de[16];

					chid_date(result + 28, ds, sizeof (ds) - 1);
					chid_date(result + 30, de, sizeof (de) - 1);
					result[27] = 0;
					cs_log("chid: %02X%02X, date: %s - %s, name: %s", result[6], result[7], ds, de, trim((char *) result + 10));
				}
			}
		}
		//================================================================================

	}

	reader[ridx].online = 1;

	return (1);
}
