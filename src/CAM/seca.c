#include "globals.h"
#include "CAM/seca.h"
#include "CAM/common.h"

#include "simples.h"
#include "log.h"

static unsigned short pmap = 0;	// provider-maptable
unsigned long long serial;
char *card;

static int set_provider_info(int i)
{
	static uchar ins12[] = { 0xc1, 0x12, 0x00, 0x00, 0x19 };	// get provider info
	uchar result[260];
	ushort result_size;

	int year, month, day;
	struct tm *lt;
	time_t t;
	int valid = 0;		//0=false, 1=true
	char l_name[16 + 8 + 1] = ", name: ";

	ins12[2] = i;	//select provider
	cam_common_cmd2card(ins12, sizeof(ins12), result, sizeof(result), &result_size);	// show provider properties
	cs_debug("hexdump:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x.", result[0], result[1], result[2], result[3], result[4], result[5], result[6], result[7], result[8], result[9], result[10], result[11], result[12], result[13], result[14],
		 result[15], result[16], result[17], result[18], result[19], result[20], result[21], result[22], result[23], result[24], result[25], result[26]);

	if ((result[25] != 0x90) || (result[26] != 0x00))
		return (0);
	reader[ridx].prid[i][0] = 0;
	reader[ridx].prid[i][1] = 0;	//blanken high byte provider code
	memcpy(&reader[ridx].prid[i][2], result, 2);
//	sprintf(buf+strlen(buf), ",%06X", b2i(3, &reader[ridx].prid[i][1]));

	year = (result[22] >> 1) + 1990;
	month = ((result[22] & 0x1) * 256 + (result[23] & 0xe0)) >> 5;
	day = (result[23] & 0x1f);
	t = time(NULL);
	lt = localtime(&t);
	if (lt->tm_year + 1900 != year) {
		if (lt->tm_year + 1900 < year) {
			valid = 1;
		} else {
			valid = 0;
		}
	} else if (lt->tm_mon + 1 != month) {
		if (lt->tm_mon + 1 < month) {
			valid = 1;
		} else {
			valid = 0;
		}
	} else if (lt->tm_mday != day) {
		if (lt->tm_mday < day) {
			valid = 1;
		} else {
			valid = 0;
		}
	}
	memcpy(l_name + 8, result + 2, 16);
	l_name[sizeof (l_name) - 1] = 0;
	trim(l_name + 8);
	l_name[0] = (l_name[8]) ? ',' : 0;
	reader[ridx].availkeys[i][0] = valid;	//misusing availkeys to register validity of provider
	cs_log("provider: %d, valid: %i%s, expiry date: %4d/%02d/%02d", i + 1, valid, l_name, year, month, day);
	memcpy(&reader[ridx].sa[i][0], result + 18, 4);
	if (valid == 1)	//if not expired
		cs_log("SA: %s", cs_hexdump(0, result + 18, 4));
//	cs_log("SA:%02X%02X%02X%02X.",result[18],result[19],result[20],result[21]);

	return 1;
}

int seca_card_init(uchar *atr, ushort atr_size)
{
	uchar buf[256];
	static uchar ins0e[] = { 0xc1, 0x0e, 0x00, 0x00, 0x08 };	// get serial number (UA)
	static uchar ins16[] = { 0xc1, 0x16, 0x00, 0x00, 0x07 };	// get nr. of prividers
	int i;

// Unlock parental control
// c1 30 00 01 09
// 00 00 00 00 00 00 00 00 ff
	static uchar ins30[] = { 0xc1, 0x30, 0x00, 0x01, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff };
	uchar result[260];
	ushort result_size;

	buf[0] = 0x00;
	if ((atr[10] != 0x0e) || (atr[11] != 0x6c) || (atr[12] != 0xb6) || (atr[13] != 0xd6))
		return (0);
	switch (atr[7] << 8 | atr[8]) {
		case 0x5084:
			card = "Generic";
			break;
		case 0x5384:
			card = "Philips";
			break;
		case 0x5130:
		case 0x5430:
		case 0x5760:
			card = "Thompson";
			break;
		case 0x5284:
		case 0x5842:
		case 0x6060:
			card = "Siemens";
			break;
		case 0x7070:
			card = "Canal+ NL";
			break;
		default:
			card = "Unknown";
			break;
	}
	reader[ridx].caid[0] = 0x0100;
	memset(reader[ridx].prid, 0xff, sizeof (reader[ridx].prid));
	cam_common_cmd2card(ins0e, sizeof(ins0e), result, sizeof(result), &result_size);	// read unique id
	reader[ridx].hexserial[0] = 0;
	reader[ridx].hexserial[1] = 0;
	memcpy(reader[ridx].hexserial + 2, result + 2, 6);
	serial = b2ll(5, result + 3);
	cs_log("type: seca, caid: %04X, serial: %llu, card: %s v%d.%d", reader[ridx].caid[0], serial, card, atr[9] & 0x0F, atr[9] >> 4);
	cam_common_cmd2card(ins16, sizeof(ins0e), result, sizeof(result), &result_size);	// read nr of providers
	pmap = result[2] << 8 | result[3];
	for (reader[ridx].nprov = 0, i = pmap; i; i >>= 1)
		reader[ridx].nprov += i & 1;
//  i=result[2]*256+result[3];
//  do { n+=i&1; i>>=1; } while(i);
//  reader[ridx].nprov=n;

	for (i = 0; i < 16; i++)
		if (pmap & (1 << i)) {
			if (!set_provider_info(i))
				return (0);
			else
				sprintf((char *) buf + strlen((char *) buf), ",%04lX", b2i(2, &reader[ridx].prid[i][2]));
		}

	cs_log("providers: %d (%s)", reader[ridx].nprov, buf + 1);
// Unlock parental control
	if (cfg->ulparent != 0) {
		cam_common_cmd2card(ins30, sizeof(ins30), result, sizeof(result), &result_size);
		cs_log("ins30_answer: %02x%02x", result[0], result[1]);
	} else {
		cs_log("parental locked");
	}
	cs_log("ready for requests");
	return (1);
}

// static int get_prov_index (uchar providhigh, uchar providlow)//returns provider id or -1 if not found
static int get_prov_index(char *provid)	//returns provider id or -1 if not found
{
	int prov;

	for (prov = 0; prov < reader[ridx].nprov; prov++)	//search for provider index
		if (!memcmp(provid, &reader[ridx].prid[prov][2], 2))
			return (prov);
//  for (prov=0; prov<reader[ridx].nprov; prov++) //search for provider index
//    if ( (providhigh == reader[ridx].prid[prov][2]) &&
//         (providlow == reader[ridx].prid[prov][3]) )
//    { 
//      return(prov);
//    }
	return (-1);
}


int seca_do_ecm(ECM_REQUEST * er)
{
	static unsigned char ins3c[] = { 0xc1, 0x3c, 0x00, 0x00, 0x00 };	// coding cw
	static unsigned char ins3a[] = { 0xc1, 0x3a, 0x00, 0x00, 0x10 };	// decoding cw
	uchar result[260];
	ushort result_size;
	uchar ins3cdata[256];
	int i;

//  i=get_prov_index(er->ecm[3],er->ecm[4]);
	i = get_prov_index((char *) er->ecm + 3);
	if ((i == -1) || (reader[ridx].availkeys[i][0] == 0))	//if provider not found or expired
		return (0);
	ins3c[2] = i;
	ins3c[3] = er->ecm[7];	//key nr
	ins3c[4] = (((er->ecm[1] & 0x0f) * 256) + er->ecm[2]) - 0x05;

	memcpy(ins3cdata, er->ecm + 8, 256 - 8);
	cs_debug("do_ecm:ins3c=%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x.", ins3c[0], ins3c[1], ins3c[2], ins3c[3], ins3c[4], ins3cdata[0], ins3cdata[1], ins3cdata[2], ins3cdata[3], ins3cdata[4], ins3cdata[5], ins3cdata[6], ins3cdata[7], ins3cdata[8], ins3cdata[9]);
	uchar ins3c_cmd[272];
	memcpy(ins3c_cmd, ins3c, 5);
	memcpy(ins3c_cmd + 5, ins3cdata, ins3c[4]);
	cam_common_cmd2card(ins3c_cmd, 5 + ins3c[4], result, sizeof(result), &result_size);	//ecm request
	cs_debug("do_ecm_answer:%02x%02x", result[0], result[1]);

	static unsigned char ins30[] = { 0xC1, 0x30, 0x00, 0x02, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF };
	/* We need to use a token */
	if (result[0] == 0x90 && result[1] == 0x1a) {
		cam_common_cmd2card(ins30, sizeof(ins30), result, sizeof(result), &result_size);
		cs_debug("do_ins30_answer:%02x%02x", result[0], result[1]);
		cam_common_cmd2card(ins3c_cmd, 5 + ins3c[4], result, sizeof(result), &result_size);	//ecm request
		cs_debug("do_ecm_answer2:%02x%02x", result[0], result[1]);
	}

	if ((result[0] != 0x90) || (result[1] != 0x00))
		return (0);
	cam_common_cmd2card(ins3a, sizeof(ins3a), result, sizeof(result), &result_size);	//get cw's
	cs_debug("cwdump:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x.", result[0], result[1], result[2], result[3], result[4], result[5], result[6], result[7], result[8], result[9], result[10], result[11], result[12], result[13], result[14], result[15], result[16],
		 result[17]);
	if ((result[16] != 0x90) || (result[17] != 0x00))
		return (0);	//exit if response is not 90 00 //TODO: if response is 9027 ppv mode is possible!
	memcpy(er->cw, result, 16);
	return (1);

}

int seca_do_emm(EMM_PACKET * ep)
{
	static unsigned char ins40[] = { 0xc1, 0x40, 0x00, 0x00, 0x00 };
	uchar result[260];
	ushort result_size;
	uchar ins40data[256];
	uchar ins40_cmd[272];
	int i;

	cs_debug("EMM:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x.", ep->emm[0], ep->emm[1], ep->emm[2], ep->emm[3], ep->emm[4], ep->emm[5], ep->emm[6], ep->emm[7], ep->emm[8], ep->emm[9], ep->emm[10], ep->emm[11], ep->emm[12], ep->emm[13], ep->emm[14],
		 ep->emm[15], ep->emm[16], ep->emm[17], ep->emm[18], ep->emm[19], ep->emm[20], ep->emm[21], ep->emm[22], ep->emm[23], ep->emm[24], ep->emm[25], ep->emm[26]);
	if (ep->emm[0] == 0x84) {	//shared EMM
		//to test if SA matches
		//first find out prov id
//      i=get_prov_index(ep->emm[3],ep->emm[4]);
		i = get_prov_index((char *) ep->emm + 3);
		if (i == -1)
			return (0);
		else	//prov id found, now test for SA (only first 3 bytes, custom byte does not count)
		if ((ep->emm[5] != reader[ridx].sa[i][0]) || (ep->emm[6] != reader[ridx].sa[i][1]) || (ep->emm[7] != reader[ridx].sa[i][2])) {
			cs_log("EMM: Shared update did not match; EMM SA:%02X%02X%02X, Reader SA:%02X,%02X,%02X.", ep->emm[5], ep->emm[6], ep->emm[7], reader[ridx].sa[i][0], reader[ridx].sa[i][1], reader[ridx].sa[i][2]);
			return (0);
		} else {
			cs_log("EMM: Shared update matched for EMM SA %02X%02X%02X.", ep->emm[5], ep->emm[6], ep->emm[7]);
			ins40[3] = ep->emm[9];
			ins40[4] = (ep->emm[1] & 0x0f) * 256 + ep->emm[2] - 0x07;
			memcpy(ins40data, ep->emm + 10, 256 - 10);
		}

	}	//end shared EMM
	else if (ep->emm[0] == 0x82) {	//unique EMM
		//first test if UA matches
		if ((reader[ridx].hexserial[2] != ep->emm[3]) || (reader[ridx].hexserial[3] != ep->emm[4]) || (reader[ridx].hexserial[4] != ep->emm[5]) || (reader[ridx].hexserial[5] != ep->emm[6]) || (reader[ridx].hexserial[6] != ep->emm[7]) || (reader[ridx].hexserial[7] != ep->emm[8])) {
			cs_log("EMM: Unique update did not match; EMM Serial:%02X%02X%02X%02X%02X%02X, Reader Serial:%02X%02X%02X%02X%02X%02X.", ep->emm[3], ep->emm[4], ep->emm[5], ep->emm[6], ep->emm[7], ep->emm[8], reader[ridx].hexserial[2], reader[ridx].hexserial[3], reader[ridx].hexserial[4],
			       reader[ridx].hexserial[5], reader[ridx].hexserial[6], reader[ridx].hexserial[7]);
			return (0);
		} else {
			cs_log("EMM: Unique update matched EMM Serial:%02X%02X%02X%02X%02X.", ep->emm[3], ep->emm[4], ep->emm[5], ep->emm[6], ep->emm[7], ep->emm[8]);
			//first find out prov id
//                      i=get_prov_index(ep->emm[9],ep->emm[10]);
			i = get_prov_index((char *) ep->emm + 9);
			if (i == -1)
				return (0);
			ins40[3] = ep->emm[12];
			ins40[4] = (ep->emm[1] & 0x0f) * 256 + ep->emm[2] - 0x0A;
			memcpy(ins40data, ep->emm + 13, 256 - 13);
		}
	}	//end unique EMM
	else
		return (0);	//geen 0x84 en geen 0x82

	ins40[2] = i;
//  length = ((er->ecm[1]<<8 || er->ecm[2])&0x0fff);
	cs_debug("do_emm:ins40=%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x.", ins40[0], ins40[1], ins40[2], ins40[3], ins40[4], ins40data[0], ins40data[1], ins40data[2], ins40data[3], ins40data[4], ins40data[5], ins40data[6], ins40data[7], ins40data[8], ins40data[9]);
	memcpy(ins40_cmd, ins40, 5);
	memcpy(ins40_cmd + 5, ins40data, ins40[4]);
	cam_common_cmd2card(ins40_cmd, ins40[4], result, sizeof(result), &result_size);	//emm request
	cs_debug("emmdump:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x.", result[0], result[1], result[2], result[3], result[4], result[5], result[6], result[7], result[8], result[9], result[10], result[11], result[12], result[13], result[14], result[15], result[16],
		 result[17]);
//TODO  if ((result[16] != 0x90) || (result[17] != 0x00)) return (0);
//  if ((result[16] != 0x90) || (result[17] != 0x19))
//        seca_card_init(); //if return code = 90 19 then PPUA changed. //untested!!
//  else
	if (result[0] == 0x97) {
		cs_log("EMM: Update not necessary.");
		return (1);	//Update not necessary
	}
	if ((result[0] == 0x90) && ((result[1] == 0x00) || (result[1] == 0x19)))
		if (set_provider_info(i) != 0)	//after successfull EMM, print new provider info
			return (1);
	return (0);

}

#ifndef LALL
int seca_card_info()
{
	int i;

	cs_log("card detected");
	cs_log("type: seca, caid: %04X, serial: %llu, card: %s ", reader[ridx].caid[0], serial, card);
	for (i = 0; i < 16; i++)
		if (pmap & (1 << i)) {
			if (!set_provider_info(i))
				return (0);
		}

	reader[ridx].online = 1;

	return 1;
}
#endif
#ifdef LALL
int seca_card_info()
{
	static uchar ins12[] = { 0xc1, 0x12, 0x00, 0x00, 0x19 };	// get provider info
	uchar result[260];
	ushort result_size;
	int year, month, day;
	struct tm *lt;
	time_t t;
	int valid = 0;		//0=false, 1=true
	char l_name[16 + 8 + 1] = ", name: ";

	ins12[2] = i;	//select provider
	cam_common_cmd2card(ins12, sizeof(ins12), result, sizeof(result), &result_size);	// show provider properties
	cs_debug("hexdump:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x.", result[0], result[1], result[2], result[3], result[4], result[5], result[6], result[7], result[8], result[9], result[10], result[11], result[12], result[13], result[14],
		 result[15], result[16], result[17], result[18], result[19], result[20], result[21], result[22], result[23], result[24], result[25], result[26]);

	if ((result[25] != 0x90) || (result[26] != 0x00))
		return (0);
	reader[ridx].prid[i][0] = 0;
	reader[ridx].prid[i][1] = 0;	//blanken high byte provider code
	memcpy(&reader[ridx].prid[i][2], result, 2);
//  sprintf(buf+strlen(buf), ",%06X", b2i(3, &reader[ridx].prid[i][1]));

	year = (result[22] >> 1) + 1990;
	month = ((result[22] & 0x1) * 256 + (result[23] & 0xe0)) >> 5;
	day = (result[23] & 0x1f);
	t = time(NULL);
	lt = localtime(&t);
	if (lt->tm_year + 1900 != year)
		if (lt->tm_year + 1900 < year)
			valid = 1;
		else
			valid = 0;
	else if (lt->tm_mon + 1 != month)
		if (lt->tm_mon + 1 < month)
			valid = 1;
		else
			valid = 0;
	else if (lt->tm_mday != day)
		if (lt->tm_mday < day)
			valid = 1;
		else
			valid = 0;
	memcpy(l_name + 8, result + 2, 16);
	l_name[sizeof (l_name)] = 0;
	trim(l_name + 8);
	l_name[0] = (l_name[8]) ? ',' : 0;
	reader[ridx].availkeys[i][0] = valid;	//misusing availkeys to register validity of provider
	cs_log("provider: %d, valid: %i, expiry date: %i/%i/%i%s", i + 1, valid, year, month, day, l_name);
	memcpy(&reader[ridx].sa[i][0], result + 18, 4);
	if (valid == 1)	//if not expired
		cs_log("SA:%02X%02X%02X%02X.", result[18], result[19], result[20], result[21]);

	reader[ridx].online = 1;

	return (1);
}
#endif
